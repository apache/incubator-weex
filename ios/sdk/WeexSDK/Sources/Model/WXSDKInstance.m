/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 * 
 *   http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#import "WXSDKInstance.h"
#import "WXSDKInstance_private.h"
#import "WXSDKManager.h"
#import "WXSDKError.h"
#import "WXMonitor.h"
#import "WXAppMonitorProtocol.h"
#import "WXNetworkProtocol.h"
#import "WXModuleFactory.h"
#import "WXHandlerFactory.h"
#import "WXDebugTool.h"
#import "WXUtility.h"
#import "WXAssert.h"
#import "WXLog.h"
#import "WXView.h"
#import "WXRootView.h"
#import "WXThreadSafeMutableDictionary.h"
#import "WXResourceRequest.h"
#import "WXResourceResponse.h"
#import "WXResourceLoader.h"
#import "WXSDKEngine.h"
#import "WXValidateProtocol.h"
#import "WXConfigCenterProtocol.h"
#import "WXTextComponent.h"
#import "WXConvert.h"
#import "WXPrerenderManager.h"
#import "WXJSExceptionProtocol.h"
#import "WXExceptionUtils.h"
#import "WXMonitor.h"
#import "WXBridgeContext.h"
#import "WXJSCoreBridge.h"
#import "WXSDKInstance_performance.h"
#import "WXPageEventNotifyEvent.h"
#import "WXConvertUtility.h"
#import "WXCoreBridge.h"
#import <WeexSDK/WXDataRenderHandler.h>

#define WEEX_LITE_URL_SUFFIX           @"wlasm"
#define WEEX_RENDER_TYPE_PLATFORM       @"platform"

NSString *const bundleUrlOptionKey = @"bundleUrl";
NSString *const bundleResponseUrlOptionKey = @"bundleResponseUrl";

NSTimeInterval JSLibInitTime = 0;

static NSString* lastPageInfoLock = @"";
static NSDictionary* lastPageInfo = nil;

typedef enum : NSUInteger {
    WXLoadTypeNormal,
    WXLoadTypeBack,
    WXLoadTypeForward,
    WXLoadTypeReload,
    WXLoadTypeReplace
} WXLoadType;

@implementation WXSDKInstance
{
    NSDictionary *_options;
    id _jsData;
    
    WXResourceLoader *_mainBundleLoader;
    WXComponentManager *_componentManager;
    UIView *_rootView;
    WXThreadSafeMutableDictionary *_moduleEventObservers;
    BOOL _performanceCommit;
    BOOL _debugJS;
    id<WXBridgeProtocol> _instanceJavaScriptContext; // sandbox javaScript context
    BOOL _defaultDataRender;
}

- (void)dealloc
{
    [_moduleEventObservers removeAllObjects];
    [self removeObservers];
}

- (instancetype)init
{
    return [self initWithRenderType:WEEX_RENDER_TYPE_PLATFORM];
}

- (instancetype)initWithRenderType:(NSString*)renderType
{
    self = [super init];
    if (self) {
        _renderType = renderType;
        _appearState = YES;
        
        NSInteger instanceId = 0;
        @synchronized(bundleUrlOptionKey) {
            static NSInteger __instance = 0;
            instanceId = __instance % (1024*1024);
            __instance += 2; // always add by 2 as even number
        }
        
        _instanceId = [NSString stringWithFormat:@"%ld", (long)instanceId];
        
        if (self.isCustomRenderType) {
            // check render type is available
            NSSet* availableRenderTypes = [WXCustomPageBridge getAvailableCustomRenderTypes];
            if ([availableRenderTypes containsObject:_renderType]) {
                // custom render page, we use odd instanceId, and (instanceId + 1) is sure not used by other pages.
                _instanceId = [NSString stringWithFormat:@"%ld", (long)(instanceId + 1)];
                [WXCoreBridge setPageArgument:_instanceId key:@"renderType" value:_renderType];
            }
            else {
                WXLogError(@"Unsupported render type '%@'. Regress to platform target.", _renderType);
                _renderType = WEEX_RENDER_TYPE_PLATFORM;
            }
        }
        
        WXLogInfo(@"Create instance: %@, render type: %@", _instanceId, _renderType);
        
        // TODO self is retained here.
        [WXSDKManager storeInstance:self forID:_instanceId];
        
        _bizType = @"";
        _pageName = @"";
        
        _performanceDict = [WXThreadSafeMutableDictionary new];
        _moduleInstances = [WXThreadSafeMutableDictionary new];
        _styleConfigs = [NSMutableDictionary new];
        _attrConfigs = [NSMutableDictionary new];
        _moduleEventObservers = [WXThreadSafeMutableDictionary new];
        _trackComponent = NO;
        _performanceCommit = NO;
        
        _performance = [[WXPerformance alloc] init];
        _apmInstance = [[WXApmForInstance alloc] init];
        
        _defaultDataRender = NO;
        
        _useBackupJsThread = NO;

        [self addObservers];
    }
    return self;
}

- (BOOL)isCustomRenderType
{
    return ![_renderType isEqualToString:WEEX_RENDER_TYPE_PLATFORM];
}

- (id<WXBridgeProtocol>)instanceJavaScriptContext
{
    _debugJS = [WXDebugTool isDevToolDebug];
    
    Class bridgeClass = _debugJS ? NSClassFromString(@"WXDebugger") : [WXJSCoreBridge class];
    
    if (_instanceJavaScriptContext && [_instanceJavaScriptContext isKindOfClass:bridgeClass]) {
        return _instanceJavaScriptContext;
    }
    
    WXAssertBridgeThread();
    if (_instanceJavaScriptContext) {
        _instanceJavaScriptContext = nil;
    }
    
    // WXDebugger is a singleton actually and should not call its init twice.
    _instanceJavaScriptContext = _debugJS ? [NSClassFromString(@"WXDebugger") alloc] : [[WXJSCoreBridge alloc] initWithoutDefaultContext];
    if (!_debugJS) {
        id<WXBridgeProtocol> jsBridge = [[WXSDKManager bridgeMgr] valueForKeyPath:@"bridgeCtx.jsBridge"];
        if (_useBackupJsThread) {
              jsBridge = [[WXSDKManager bridgeMgr] valueForKeyPath:@"backupBridgeCtx.jsBridge"];
        }
        JSContext* globalContex = jsBridge.javaScriptContext;
        JSContextGroupRef contextGroup = JSContextGetGroup([globalContex JSGlobalContextRef]);
        JSClassDefinition classDefinition = kJSClassDefinitionEmpty;
        classDefinition.attributes = kJSClassAttributeNoAutomaticPrototype;
        JSClassRef globalObjectClass = JSClassCreate(&classDefinition);
        JSGlobalContextRef sandboxGlobalContextRef = JSGlobalContextCreateInGroup(contextGroup, globalObjectClass);
        JSClassRelease(globalObjectClass);
        JSContext * instanceContext = [JSContext contextWithJSGlobalContextRef:sandboxGlobalContextRef];
        JSGlobalContextRelease(sandboxGlobalContextRef);
        [WXBridgeContext mountContextEnvironment:instanceContext];
        [_instanceJavaScriptContext setJSContext:instanceContext];
    }
    
    if ([_instanceJavaScriptContext respondsToSelector:@selector(setWeexInstanceId:)]) {
        [_instanceJavaScriptContext setWeexInstanceId:_instanceId];
    }
    if (!_debugJS) {
        [[NSNotificationCenter defaultCenter] postNotificationName:WX_INSTANCE_JSCONTEXT_CREATE_NOTIFICATION object:_instanceJavaScriptContext.javaScriptContext];
    }
    
    return _instanceJavaScriptContext;
}

- (NSString *)description
{
    // get _rootView.frame in JS thread may cause deaklock.
    return [NSString stringWithFormat:@"<%@: %p; id = %@; rootView = %p; url= %@>", NSStringFromClass([self class]), self, _instanceId, (__bridge void*)_rootView, _scriptURL];
}

- (void)setParentInstance:(WXSDKInstance *)parentInstance
{
    WXLogInfo(@"Embed instance %@ into parent instance %@", _instanceId, parentInstance.instanceId);
    _parentInstance = parentInstance;
}

#pragma mark Public Mehtods

- (UIView *)rootView
{
    return _rootView;
}

- (void)setFrame:(CGRect)frame
{
#ifdef DEBUG
    WXLogDebug(@"flexLayout -> setFrame :%@,instance :%@",NSStringFromCGRect(frame),self);
#endif
    if (!CGRectEqualToRect(frame, _frame)) {
        _frame = frame;
        CGFloat screenHeight =  [[UIScreen mainScreen] bounds].size.height;
        if (screenHeight>0) {
            CGFloat pageRatio = frame.size.height/screenHeight *100;
            self.apmInstance.pageRatio = pageRatio>100?100:pageRatio;
        }
        WXPerformBlockOnMainThread(^{
            if (_rootView) {
                _rootView.frame = frame;
                WXPerformBlockOnComponentThread(^{
                    [self.componentManager rootViewFrameDidChange:frame];
                });
            }
        });
    }
}

- (void)setViewportWidth:(CGFloat)viewportWidth
{
    _viewportWidth = viewportWidth;
    
    // notify weex core
    NSString* pageId = _instanceId;
    WXPerformBlockOnComponentThread(^{
        [WXCoreBridge setViewportWidth:pageId width:viewportWidth];
    });
}

- (void)setPageKeepRawCssStyles
{
    [self setPageArgument:@"reserveCssStyles" value:@"true"];
}

- (void)isKeepingRawCssStyles:(void(^)(BOOL))callback {
    NSString* pageId = _instanceId;
    WXPerformBlockOnComponentThread(^{
        if (callback) {
            callback([WXCoreBridge isKeepingRawCssStyles:pageId]);
        }
    });
}

- (void)setPageArgument:(NSString*)key value:(NSString*)value
{
    NSString* pageId = _instanceId;
    WXPerformBlockOnComponentThread(^{
        [WXCoreBridge setPageArgument:pageId key:key value:value];
    });
}

- (void)setScriptURL:(NSURL *)scriptURL
{
    _scriptURL = scriptURL;
    [WXCoreBridge setPageArgument:_instanceId key:@"url" value:[_scriptURL absoluteString]];
}

- (void)setPageRequiredWidth:(CGFloat)width height:(CGFloat)height
{
    _screenSize = CGSizeMake(width, height);
    
    // notify weex core
    NSString* pageId = _instanceId;
    WXPerformBlockOnComponentThread(^{
        [WXCoreBridge setPageRequired:pageId width:width height:height];
    });
}

- (void)renderWithURL:(NSURL *)url
{
    [self renderWithURL:url options:nil data:nil];
}

- (void)_checkPageName
{
    if (nil == self.pageName || [self.pageName isEqualToString:@""]) {
        self.pageName = [self.scriptURL isFileURL] ? self.scriptURL.path.lastPathComponent: self.scriptURL.absoluteString;
    }
    if (nil == self.pageName || [self.pageName isEqualToString:@""]) {
        self.pageName = NSStringFromClass(self.viewController.class)?:@"unkonwPageCauseUnsetNameAndUrlAndVc";
    }
}

- (void)renderWithURL:(NSURL *)url options:(NSDictionary *)options data:(id)data
{
    if (!url) {
        WXLogError(@"Url must be passed if you use renderWithURL");
        return;
    }
    WXLogInfo(@"pageid: %@ renderWithURL: %@", _instanceId, url.absoluteString);
    
    @synchronized (lastPageInfoLock) {
        lastPageInfo = @{@"pageId": [_instanceId copy], @"url": [url absoluteString] ?: @""};
    }
    
    [WXCoreBridge install];
    if (_useBackupJsThread) {
        [[WXSDKManager bridgeMgr] executeJSTaskQueue];
    }

    self.scriptURL = url;
    [self _checkPageName];
    [self.apmInstance startRecord:self.instanceId];
    self.apmInstance.isStartRender = YES;
    
    self.needValidate = [[WXHandlerFactory handlerForProtocol:@protocol(WXValidateProtocol)] needValidate:url];
    WXResourceRequest *request = [WXResourceRequest requestWithURL:url resourceType:WXResourceTypeMainBundle referrer:@"" cachePolicy:NSURLRequestUseProtocolCachePolicy];
    [self _renderWithRequest:request options:options data:data];

    NSURL* nsURL = [NSURL URLWithString:options[@"DATA_RENDER_JS"]];
    [self _downloadAndExecScript:nsURL];
}

- (void)renderView:(id)source options:(NSDictionary *)options data:(id)data
{
    _options = [options isKindOfClass:[NSDictionary class]] ? options : nil;
    _jsData = data;
    WXLogInfo(@"pageid: %@ renderView pageNmae: %@  options: %@", _instanceId, _pageName, options);
    
    @synchronized (lastPageInfoLock) {
        lastPageInfo = @{@"pageId": [_instanceId copy], @"options": options ? [options description] : @""};
    }

    [WXCoreBridge install];
    if (_useBackupJsThread) {
        [[WXSDKManager bridgeMgr] executeJSTaskQueue];
    }

    self.needValidate = [[WXHandlerFactory handlerForProtocol:@protocol(WXValidateProtocol)] needValidate:self.scriptURL];

    if ([source isKindOfClass:[NSString class]]) {
        WXLogDebug(@"Render source: %@, data:%@", self, [WXUtility JSONString:data]);
        [self _renderWithMainBundleString:source];
    } else if ([source isKindOfClass:[NSData class]]) {
        [self _renderWithData:source];
    }
    NSURL* nsURL = [NSURL URLWithString:options[@"DATA_RENDER_JS"]];
    [self _downloadAndExecScript:nsURL];
}

- (void)_downloadAndExecScript:(NSURL *)url {
    [[WXSDKManager bridgeMgr] DownloadJS:_instanceId url:url completion:^(NSString *script) {
        if (!script) {
            return;
        }
        if (self.dataRender) {
            id<WXDataRenderHandler> dataRenderHandler = [WXHandlerFactory handlerForProtocol:@protocol(WXDataRenderHandler)];
            if (dataRenderHandler) {
                [[WXSDKManager bridgeMgr] createInstanceForJS:_instanceId template:script options:_options data:_jsData];

                NSString* instanceId = self.instanceId;
                WXPerformBlockOnComponentThread(^{
                    [dataRenderHandler DispatchPageLifecycle:instanceId];
                });
            }
            else {
                if (self.componentManager.isValid) {
                    WXSDKErrCode errorCode = WX_KEY_EXCEPTION_DEGRADE_EAGLE_RENDER_ERROR;
                    NSError *error = [NSError errorWithDomain:WX_ERROR_DOMAIN code:errorCode userInfo:@{@"message":@"No data render handler found!"}];
                    WXPerformBlockOnComponentThread(^{
                        [self.componentManager renderFailed:error];
                    });
                }
            }
            return;
        }
    }];
}

- (NSString *) bundleTemplate
{
    return self.mainBundleString;
}

- (void)_renderWithData:(NSData *)contents
{
    if (!self.instanceId) {
        WXLogError(@"Fail to find instance！");
        return;
    }
    
    if (_isRendered) {
        [WXExceptionUtils commitCriticalExceptionRT:self.instanceId errCode:[NSString stringWithFormat:@"%d", WX_ERR_RENDER_TWICE] function:@"_renderWithData:" exception:[NSString stringWithFormat:@"instance is rendered twice"] extParams:nil];
        return;
    }

    //some case , with out render (url)
    [self.apmInstance startRecord:self.instanceId];
    self.apmInstance.isStartRender = YES;
    
    [_apmInstance setProperty:KEY_PAGE_PROPERTIES_UIKIT_TYPE withValue:_renderType?: WEEX_RENDER_TYPE_PLATFORM];
    if (self.dataRender) {
        [self.apmInstance setProperty:KEY_PAGE_PROPERTIES_RENDER_TYPE withValue:@"eagle"];
    }

    self.performance.renderTimeOrigin = CACurrentMediaTime()*1000;
    self.performance.renderUnixTimeOrigin = [WXUtility getUnixFixTimeMillis];
    long renderOriginTimePlatform = [self.apmInstance onStage:KEY_PAGE_STAGES_RENDER_ORGIGIN];
    
    // pass render origin time to page
    [WXCoreBridge setPageArgument:_instanceId key:@"renderTimeOrigin" value:[NSString stringWithFormat:@"%lld", (long long)([[NSDate date] timeIntervalSince1970] * 1000)]];
    [WXCoreBridge setPageArgument:_instanceId key:@"renderTimeOriginPlatform" value:[NSString stringWithFormat:@"%lld", (long long)renderOriginTimePlatform]];

    if (![WXUtility isBlankString:self.pageName]) {
        WXLog(@"Start rendering page:%@", self.pageName);
    } else {
        WXLogWarning(@"WXSDKInstance's pageName should be specified.");
        id<WXJSExceptionProtocol> jsExceptionHandler = [WXHandlerFactory handlerForProtocol:@protocol(WXJSExceptionProtocol)];
        if ([jsExceptionHandler respondsToSelector:@selector(onRuntimeCheckException:)]) {
            WXRuntimeCheckException * runtimeCheckException = [WXRuntimeCheckException new];
            runtimeCheckException.exception = @"We highly recommend you to set pageName.\n Using WXSDKInstance * instance = [WXSDKInstance new]; instance.pageName = @\"your page name\" to fix it";
            [jsExceptionHandler onRuntimeCheckException:runtimeCheckException];
        }
    }

    id<WXPageEventNotifyEventProtocol> pageEvent = [WXSDKEngine handlerForProtocol:@protocol(WXPageEventNotifyEventProtocol)];
    if ([pageEvent respondsToSelector:@selector(pageStart:)]) {
        [pageEvent pageStart:self.instanceId];
    }

    WX_MONITOR_INSTANCE_PERF_START(WXPTFirstScreenRender, self);
    WX_MONITOR_INSTANCE_PERF_START(WXPTAllRender, self);

    NSMutableDictionary *dictionary = [_options mutableCopy];
    if ([WXLog logLevel] >= WXLogLevelLog) {
        dictionary[@"debug"] = @(YES);
    }

    WXPerformBlockOnMainThread(^{
        if (self.isCustomRenderType) {
            self->_rootView = [WXCustomPageBridge createPageRootView:self.instanceId pageType:self.renderType frame:self.frame];
        }
        else {
            self->_rootView = [[WXRootView alloc] initWithFrame:self.frame];
            ((WXRootView*)(self->_rootView)).instance = self;
        }
        
        if (self.onCreate) {
            self.onCreate(self->_rootView);
        }
    });
    // ensure default modules/components/handlers are ready before create instance
    [WXSDKEngine registerDefaults];
    [[NSNotificationCenter defaultCenter] postNotificationName:WX_SDKINSTANCE_WILL_RENDER object:self];

    if ([self _handleConfigCenter]) {
        int wxErrorCode = 9999;
        NSError * error = [NSError errorWithDomain:WX_ERROR_DOMAIN code:wxErrorCode userInfo:nil];
        if (self.onFailed) {
            self.onFailed(error);
        }
        [self.apmInstance setProperty:KEY_PROPERTIES_ERROR_CODE withValue:[@(wxErrorCode) stringValue]];
        return;
    }

    [[WXSDKManager bridgeMgr] createInstance:self.instanceId contents:contents options:dictionary data:_jsData];

    // WX_MONITOR_PERF_SET(WXPTBundleSize, [data length], self);
    _isRendered = YES;
}

- (void)_renderWithMainBundleString:(NSString *)mainBundleString
{
    if (!self.instanceId) {
        WXLogError(@"Fail to find instance！");
        return;
    }
    
    if (_isRendered) {
        [WXExceptionUtils commitCriticalExceptionRT:self.instanceId errCode:[NSString stringWithFormat:@"%d", WX_ERR_RENDER_TWICE] function:@"_renderWithMainBundleString:" exception:[NSString stringWithFormat:@"instance is rendered twice"] extParams:nil];
        return;
    }

    //some case , with out render (url)
    [self _checkPageName];
    [self.apmInstance startRecord:self.instanceId];
    self.apmInstance.isStartRender = YES;
    
    [_apmInstance setProperty:KEY_PAGE_PROPERTIES_UIKIT_TYPE withValue:_renderType?: WEEX_RENDER_TYPE_PLATFORM];
    if (self.dataRender) {
        [self.apmInstance setProperty:KEY_PAGE_PROPERTIES_RENDER_TYPE withValue:@"eagle"];
    }
    
    self.performance.renderTimeOrigin = CACurrentMediaTime()*1000;
    self.performance.renderUnixTimeOrigin = [WXUtility getUnixFixTimeMillis];
    long renderOriginTimePlatform = [self.apmInstance onStage:KEY_PAGE_STAGES_RENDER_ORGIGIN];
    
    // pass render origin time to page
    [WXCoreBridge setPageArgument:_instanceId key:@"renderTimeOrigin" value:[NSString stringWithFormat:@"%lld", (long long)([[NSDate date] timeIntervalSince1970] * 1000)]];
    [WXCoreBridge setPageArgument:_instanceId key:@"renderTimeOriginPlatform" value:[NSString stringWithFormat:@"%lld", (long long)renderOriginTimePlatform]];
    
    if (![WXUtility isBlankString:self.pageName]) {
        WXLog(@"Start rendering page:%@", self.pageName);
    } else {
        WXLogWarning(@"WXSDKInstance's pageName should be specified.");
        id<WXJSExceptionProtocol> jsExceptionHandler = [WXHandlerFactory handlerForProtocol:@protocol(WXJSExceptionProtocol)];
        if ([jsExceptionHandler respondsToSelector:@selector(onRuntimeCheckException:)]) {
            WXRuntimeCheckException * runtimeCheckException = [WXRuntimeCheckException new];
            runtimeCheckException.exception = @"We highly recommend you to set pageName.\n Using WXSDKInstance * instance = [WXSDKInstance new]; instance.pageName = @\"your page name\" to fix it";
            [jsExceptionHandler onRuntimeCheckException:runtimeCheckException];
        }
    }
    if (!self.userInfo) {
        self.userInfo = [NSMutableDictionary new];
    }
    if (!self.userInfo[@"jsMainBundleStringContentLength"]) {
        self.userInfo[@"jsMainBundleStringContentLength"] = @([mainBundleString length]);
    }
    if (!self.userInfo[@"jsMainBundleStringContentLength"]) {
        self.userInfo[@"jsMainBundleStringContentMd5"] = [WXUtility md5:mainBundleString];
    }
    
    id<WXPageEventNotifyEventProtocol> pageEvent = [WXSDKEngine handlerForProtocol:@protocol(WXPageEventNotifyEventProtocol)];
    if ([pageEvent respondsToSelector:@selector(pageStart:)]) {
        [pageEvent pageStart:self.instanceId];
    }

    WX_MONITOR_INSTANCE_PERF_START(WXPTFirstScreenRender, self);
    WX_MONITOR_INSTANCE_PERF_START(WXPTAllRender, self);
    
    NSMutableDictionary *dictionary = [_options mutableCopy];
    if ([WXLog logLevel] >= WXLogLevelLog) {
        dictionary[@"debug"] = @(YES);
    }
    
    if ([WXDebugTool getReplacedBundleJS]) {
        mainBundleString = [WXDebugTool getReplacedBundleJS];
    }
    
    WXPerformBlockOnMainThread(^{
        if (self.isCustomRenderType) {
            self->_rootView = [WXCustomPageBridge createPageRootView:self.instanceId pageType:self.renderType frame:self.frame];
        }
        else {
            self->_rootView = [[WXRootView alloc] initWithFrame:self.frame];
            ((WXRootView*)(self->_rootView)).instance = self;
        }
        
        if (self.onCreate) {
            self.onCreate(self->_rootView);
        }
    });
    // ensure default modules/components/handlers are ready before create instance
    [WXSDKEngine registerDefaults];
    [[NSNotificationCenter defaultCenter] postNotificationName:WX_SDKINSTANCE_WILL_RENDER object:self];
    
    _mainBundleString = mainBundleString;
    if ([self _handleConfigCenter]) {
        int wxErrorCode = 9999;
        NSError * error = [NSError errorWithDomain:WX_ERROR_DOMAIN code:wxErrorCode userInfo:nil];
        if (self.onFailed) {
            self.onFailed(error);
        }
        [self.apmInstance setProperty:KEY_PROPERTIES_ERROR_CODE withValue:[@(wxErrorCode) stringValue]];
        return;
    }
    
    [[WXSDKManager bridgeMgr] createInstance:self.instanceId template:mainBundleString options:dictionary data:_jsData];
    
    WX_MONITOR_PERF_SET(WXPTBundleSize, [mainBundleString lengthOfBytesUsingEncoding:NSUTF8StringEncoding], self);
    
    _isRendered = YES;
}

- (BOOL)_handleConfigCenter
{
    id configCenter = [WXSDKEngine handlerForProtocol:@protocol(WXConfigCenterProtocol)];
    if ([configCenter respondsToSelector:@selector(configForKey:defaultValue:isDefault:)]) {		
        BOOL enableRTLLayoutDirection = [[configCenter configForKey:@"iOS_weex_ext_config.enableRTLLayoutDirection" defaultValue:@(YES) isDefault:NULL] boolValue];
        [WXUtility setEnableRTLLayoutDirection:enableRTLLayoutDirection];
        
        BOOL isIOS13 = [[[UIDevice currentDevice] systemVersion] integerValue] == 13;
        BOOL useMRCForInvalidJSONObject = [[configCenter configForKey:@"iOS_weex_ext_config.useMRCForInvalidJSONObject" defaultValue:@(YES) isDefault:NULL] boolValue];
        BOOL alwaysUseMRCForObjectToWeexCore = [[configCenter configForKey:@"iOS_weex_ext_config.alwaysUseMRC" defaultValue:@(NO) isDefault:NULL] boolValue];
        ConvertSwitches(isIOS13, useMRCForInvalidJSONObject, alwaysUseMRCForObjectToWeexCore);
    }
    else {
        BOOL isIOS13 = [[[UIDevice currentDevice] systemVersion] integerValue] == 13;
        ConvertSwitches(isIOS13, YES, NO);
    }
    return NO;
}

- (void)renderWithMainBundleString:(NSNotification*)notification
{
    [self _renderWithMainBundleString:_mainBundleString];
}

- (void)_renderWithRequest:(WXResourceRequest *)request options:(NSDictionary *)options data:(id)data;
{
    NSURL *url = request.URL;
    self.scriptURL = url;
    _jsData = data;
    if (![options isKindOfClass:[NSDictionary class]]) {
        options = @{};
    }
    NSMutableDictionary *newOptions = [options mutableCopy] ?: [NSMutableDictionary new];
    
    if (!newOptions[bundleUrlOptionKey]) {
        newOptions[bundleUrlOptionKey] = url.absoluteString;
    }

    if ( [url.absoluteString containsString:@"__data_render=true"]) {
        newOptions[@"DATA_RENDER"] = @(YES);
    }

    if ([url.absoluteString hasSuffix:WEEX_LITE_URL_SUFFIX] || [url.absoluteString containsString:@"__eagle=true"]) {
        newOptions[@"WLASM_RENDER"] = @(YES);
    }

    // compatible with some wrong type, remove this hopefully in the future.
    if ([newOptions[bundleUrlOptionKey] isKindOfClass:[NSURL class]]) {
        WXLogWarning(@"Error type in options with key:bundleUrl, should be of type NSString, not NSURL!");
        newOptions[bundleUrlOptionKey] = ((NSURL*)newOptions[bundleUrlOptionKey]).absoluteString;
    }
    _options = [newOptions copy];
    request.userAgent = [WXUtility userAgent];
    
    WX_MONITOR_INSTANCE_PERF_START(WXPTJSDownload, self);
    __weak typeof(self) weakSelf = self;
    _mainBundleLoader = [[WXResourceLoader alloc] initWithRequest:request];;
     [self.apmInstance onStage:KEY_PAGE_STAGES_DOWN_BUNDLE_START];
    _mainBundleLoader.onFinished = ^(WXResourceResponse *response, NSData *data) {
        __strong typeof(weakSelf) strongSelf = weakSelf;
        if (strongSelf == nil) {
            return;
        }

        NSMutableDictionary* optionsCopy = [strongSelf->_options mutableCopy];
        optionsCopy[bundleResponseUrlOptionKey] = [response.URL absoluteString];
        strongSelf->_options = [optionsCopy copy];
        
        NSError *error = nil;
        if ([response isKindOfClass:[NSHTTPURLResponse class]] && ((NSHTTPURLResponse *)response).statusCode != 200) {
            error = [NSError errorWithDomain:WX_ERROR_DOMAIN
                                        code:((NSHTTPURLResponse *)response).statusCode
                                    userInfo:@{@"message":@"status code error."}];
            if (strongSelf.onFailed) {
                strongSelf.onFailed(error);
            }
        }
        
        if (strongSelf.onJSDownloadedFinish) {
            strongSelf.onJSDownloadedFinish(response, request, data, error);
        }
        
        if (error) {
            [WXExceptionUtils commitCriticalExceptionRT:strongSelf.instanceId
                                                errCode:[NSString stringWithFormat:@"%d", WX_KEY_EXCEPTION_JS_DOWNLOAD]
                                               function:@"_renderWithRequest:options:data:"
                                              exception:[NSString stringWithFormat:@"download bundle error :%@",[error localizedDescription]]
                                              extParams:nil];
        
            strongSelf.apmInstance.isDownLoadFailed = YES;
            [strongSelf.apmInstance setProperty:KEY_PROPERTIES_ERROR_CODE withValue:[@(WX_KEY_EXCEPTION_JS_DOWNLOAD) stringValue]];
            return;
        }

        if (!data) {
            NSString *errorMessage = [NSString stringWithFormat:@"Request to %@ With no data return", request.URL];
            WX_MONITOR_FAIL_ON_PAGE(WXMTJSDownload, WX_ERR_JSBUNDLE_DOWNLOAD, errorMessage, strongSelf.pageName);
            [WXExceptionUtils commitCriticalExceptionRT:strongSelf.instanceId
                                                errCode:[NSString stringWithFormat:@"%d", WX_KEY_EXCEPTION_JS_DOWNLOAD]
                                               function:@"_renderWithRequest:options:data:"
                                              exception:errorMessage
                                              extParams:nil];
            
            if (strongSelf.onFailed) {
                strongSelf.onFailed(error);
            }
            strongSelf.apmInstance.isDownLoadFailed = YES;
            [strongSelf.apmInstance setProperty:KEY_PROPERTIES_ERROR_CODE withValue:[@(WX_KEY_EXCEPTION_JS_DOWNLOAD) stringValue]];
            return;
        }
        
        if (([newOptions[@"DATA_RENDER"] boolValue] && [newOptions[@"RENDER_WITH_BINARY"] boolValue]) || [newOptions[@"WLASM_RENDER"] boolValue]) {
            [strongSelf.apmInstance onStage:KEY_PAGE_STAGES_DOWN_BUNDLE_END];
            [strongSelf _renderWithData:data];
            return;
        }

        NSString *jsBundleString = [[NSString alloc] initWithData:data encoding:NSUTF8StringEncoding];
        if (!jsBundleString) {
            WX_MONITOR_FAIL_ON_PAGE(WXMTJSDownload, WX_ERR_JSBUNDLE_STRING_CONVERT, @"data converting to string failed.", strongSelf.pageName)
            [strongSelf.apmInstance setProperty:KEY_PROPERTIES_ERROR_CODE withValue:[@(WX_ERR_JSBUNDLE_STRING_CONVERT) stringValue]];
            return;
        }
        if (!strongSelf.userInfo) {
            strongSelf.userInfo = [NSMutableDictionary new];
        }
        
        NSUInteger bundleSize = [jsBundleString length];
        [strongSelf.apmInstance updateDiffStats:KEY_PAGE_STATS_BUNDLE_SIZE withDiffValue:bundleSize];
        
        strongSelf.userInfo[@"jsMainBundleStringContentLength"] = @(bundleSize);
        strongSelf.userInfo[@"jsMainBundleStringContentMd5"] = [WXUtility md5:jsBundleString];

        WX_MONITOR_SUCCESS_ON_PAGE(WXMTJSDownload, strongSelf.pageName);
        WX_MONITOR_INSTANCE_PERF_END(WXPTJSDownload, strongSelf);
        
        if (strongSelf.onRenderTerminateWhenJSDownloadedFinish) {
            if (strongSelf.onRenderTerminateWhenJSDownloadedFinish(response, request, data, error)) {
                return;
            }
        }
        
        [strongSelf.apmInstance onStage:KEY_PAGE_STAGES_DOWN_BUNDLE_END];
        [strongSelf.apmInstance updateExtInfoFromResponseHeader:response.allHeaderFields];
        [strongSelf _renderWithMainBundleString:jsBundleString];
        [WXMonitor performanceFinishWithState:DebugAfterRequest instance:strongSelf];
    };
    
    _mainBundleLoader.onFailed = ^(NSError *loadError) {
        WXLogError(@"Request failed with error: %@", loadError);
        
        NSString *errorMessage = [NSString stringWithFormat:@"Request to %@ occurs an error:%@, info:%@", request.URL, loadError.localizedDescription, loadError.userInfo];
        long wxErrorCode = [loadError.domain isEqualToString:NSURLErrorDomain] && loadError.code == NSURLErrorNotConnectedToInternet ? WX_ERR_NOT_CONNECTED_TO_INTERNET : WX_ERR_JSBUNDLE_DOWNLOAD;

        WX_MONITOR_FAIL_ON_PAGE(WXMTJSDownload, wxErrorCode, errorMessage, weakSelf.pageName);
        
        NSMutableDictionary *allUserInfo = [[NSMutableDictionary alloc] initWithDictionary:error.userInfo];
        [allUserInfo addEntriesFromDictionary:loadError.userInfo];
        NSError *errorWithReportMsg = [NSError errorWithDomain:error.domain
                                             code:error.code
                                         userInfo:allUserInfo];
      
        if (weakSelf.onFailed) {
            weakSelf.onFailed(errorWithReportMsg);
        }
        [weakSelf.apmInstance setProperty:KEY_PROPERTIES_ERROR_CODE withValue:[@(wxErrorCode) stringValue]];
    };
    
    [_mainBundleLoader start];
}

- (void)reload:(BOOL)forcedReload
{
    // TODO: [self unload]
    if (!_scriptURL) {
        WXLogError(@"No script URL found while reloading!");
        return;
    }
    
    NSURLRequestCachePolicy cachePolicy = forcedReload ? NSURLRequestReloadIgnoringCacheData : NSURLRequestUseProtocolCachePolicy;
    WXResourceRequest *request = [WXResourceRequest requestWithURL:_scriptURL resourceType:WXResourceTypeMainBundle referrer:_scriptURL.absoluteString cachePolicy:cachePolicy];
    [self _renderWithRequest:request options:_options data:_jsData];
}

- (void)reloadLayout
{
    NSString* pageId = _instanceId;
    WXPerformBlockOnComponentThread(^{
        [WXCoreBridge reloadPageLayout:pageId];
    });
}

- (void)refreshInstance:(id)data
{
    WXLogDebug(@"refresh instance: %@, data:%@", self, [WXUtility JSONString:data]);
    
    if (!self.instanceId) {
        WXLogError(@"Fail to find instance！");
        return;
    }
    
    [[WXSDKManager bridgeMgr] refreshInstance:self.instanceId data:data];
}

- (void)destroyInstance
{
    WXLogInfo(@"Destroy instance: %@", _instanceId);
    
    [self.apmInstance endRecord];
    NSString *url = @"";
    if ([WXPrerenderManager isTaskExist:[self.scriptURL absoluteString]]) {
        url = [self.scriptURL absoluteString];
    }
    if (!self.instanceId) {
        WXLogError(@"Fail to find instance！");
        return;
    }
    
    id<WXPageEventNotifyEventProtocol> pageEvent = [WXSDKEngine handlerForProtocol:@protocol(WXPageEventNotifyEventProtocol)];
    if ([pageEvent respondsToSelector:@selector(pageDestroy:)]) {
        [pageEvent pageDestroy:self.instanceId];
    }
    [[NSNotificationCenter defaultCenter] postNotificationName:WX_INSTANCE_WILL_DESTROY_NOTIFICATION object:nil userInfo:@{@"instanceId":self.instanceId}];

    [WXPrerenderManager removePrerenderTaskforUrl:[self.scriptURL absoluteString]];
    [WXPrerenderManager destroyTask:self.instanceId];
    BOOL dataRender = self.dataRender;
    BOOL wlasmRender = self.wlasmRender;
    if (!dataRender) {
        [[WXSDKManager bridgeMgr] destroyInstance:self.instanceId];
    }
    
    WXComponentManager* componentManager = self.componentManager;
    NSString* instanceId = self.instanceId;
    
    /* Custom render target, currently we manage the pages by ourselves not by WeexCore.
     We remove the WeexCore page immediately so that any later render commands will be ignored. */
    if ([WXCustomPageBridge isCustomPage:instanceId]) {
        [[WXCustomPageBridge sharedInstance] invalidatePage:instanceId];
    }
    
    WXPerformBlockOnComponentThread(^{
        // Destroy components and views in main thread. Unbind with underneath RenderObjects.
        [componentManager unload];
        
        // Destroy weexcore c++ page and objects.
        [WXCoreBridge closePage:instanceId];
        
        if (dataRender && !wlasmRender) {
            [[WXSDKManager bridgeMgr] destroyInstance:instanceId];
        }

        // Destroy heron render target page
        if ([WXCustomPageBridge isCustomPage:instanceId]) {
            [[WXCustomPageBridge sharedInstance] removePage:instanceId];
        }
        
        // Reading config from orange for Release instance in Main Thread or not, for Bug #15172691 +{
        dispatch_async(dispatch_get_main_queue(), ^{
            [WXSDKManager removeInstanceforID:instanceId];
            WXLogInfo(@"Finally remove instance: %@", instanceId);
        });
        //+}
    });
    
    if (url.length > 0) {
        [WXPrerenderManager addGlobalTask:url callback:nil];
    }
}

- (void)forceGarbageCollection
{
    [[WXSDKManager bridgeMgr] forceGarbageCollection];
}

- (void)updateState:(WXState)state
{
    if (!self.instanceId) {
        WXLogError(@"Fail to find instance！");
        return;
    }
    
    if (!_performanceCommit && state == WeexInstanceDisappear) {
        [self updatePerDicBeforExit];
        WX_MONITOR_INSTANCE_PERF_COMMIT(self);
        _performanceCommit = YES;
    }
    
    NSMutableDictionary *data = [NSMutableDictionary dictionary];
    [data setObject:[NSString stringWithFormat:@"%ld",(long)state] forKey:@"state"];
    //[[WXSDKManager bridgeMgr] updateState:self.instanceId data:data];
    
    // First post internal notification
    [[NSNotificationCenter defaultCenter] postNotificationName:WX_INSTANCE_NOTIFICATION_UPDATE_STATE_INTERNAL object:self userInfo:data];
    [[NSNotificationCenter defaultCenter] postNotificationName:WX_INSTANCE_NOTIFICATION_UPDATE_STATE object:self userInfo:data];
}

- (id)moduleForClass:(Class)moduleClass
{
    if (!moduleClass)
        return nil;
    
    id<WXModuleProtocol> moduleInstance = self.moduleInstances[NSStringFromClass(moduleClass)];
    if (!moduleInstance) {
        moduleInstance = [[moduleClass alloc] init];
        if ([moduleInstance respondsToSelector:@selector(setWeexInstance:)])
            [moduleInstance setWeexInstance:self];
        self.moduleInstances[NSStringFromClass(moduleClass)] = moduleInstance;
    }
    
    return moduleInstance;
}

- (WXComponent *)componentForRef:(NSString *)ref
{
    WXAssertComponentThread();
    
    return [_componentManager componentForRef:ref];
}

- (NSUInteger)numberOfComponents
{
    WXAssertComponentThread();
    
    return [_componentManager numberOfComponents];
}

- (void)enumerateComponentsUsingBlock:(void (^)(WXComponent *, BOOL *stop))block
{
    WXAssertComponentThread();
    
    [_componentManager enumerateComponentsUsingBlock:block];
}

- (void)fireGlobalEvent:(NSString *)eventName params:(NSDictionary *)params
{
    if (!params){
        params = [NSDictionary dictionary];
    }
    NSDictionary * userInfo = @{
            @"weexInstance":self.instanceId,
            @"param":params
    };
    [[NSNotificationCenter defaultCenter] postNotificationName:eventName object:self userInfo:userInfo];
}

- (void)fireModuleEvent:(Class)module eventName:(NSString *)eventName params:(NSDictionary*)params
{
    NSDictionary * userInfo = @{
                                @"moduleId":NSStringFromClass(module)?:@"",
                                @"param":params?:@{},
                                @"eventName":eventName
                                };
    
    [[NSNotificationCenter defaultCenter] postNotificationName:WX_MODULE_EVENT_FIRE_NOTIFICATION object:self userInfo:userInfo];
}

- (CGFloat)pixelScaleFactor
{
    CGFloat usingScreenWidth = _screenSize.width > 0 ? _screenSize.width : [WXCoreBridge getDeviceSize].width;
    CGFloat usingViewPort = _viewportWidth > 0 ? _viewportWidth : WXDefaultScreenWidth;
    return usingScreenWidth / usingViewPort;
}
    
- (BOOL)wlasmRender {
    if ([_options[@"WLASM_RENDER"] boolValue]) {
        return YES;
    }
    return NO;
}

- (BOOL)dataRender
{
    if ([_options[@"DATA_RENDER"] boolValue] || [_options[@"WLASM_RENDER"] boolValue]) {
        return YES;
    }
    return _defaultDataRender;
}

- (NSURL *)completeURL:(NSString *)url
{
    if (!_scriptURL) {
        return [NSURL URLWithString:url];
    }
    if ([url hasPrefix:@"//"] && [_scriptURL isFileURL]) {
        return [NSURL URLWithString:url];
    }
    if (!url) {
        return nil;
    }
    NSURL *result = [NSURL URLWithString:url relativeToURL:_scriptURL];
    if (result) {
        return result;
    }
    // if result is nil, try url-encode the 'url' string.
    return [NSURL URLWithString:[url stringByAddingPercentEncodingWithAllowedCharacters:[NSCharacterSet URLQueryAllowedCharacterSet]] relativeToURL:_scriptURL];
}

- (BOOL)checkModuleEventRegistered:(NSString*)event moduleClassName:(NSString*)moduleClassName
{
    NSDictionary * observer = [_moduleEventObservers objectForKey:moduleClassName];
    return observer && observer[event]? YES:NO;
}

#pragma mark Private Methods

- (void)_addModuleEventObserversWithModuleMethod:(WXModuleMethod *)method
{
    if ([method.arguments count] < 2) {
        WXLogError(@"please check your method parameter!!");
        return;
    }
    if(![method.arguments[0] isKindOfClass:[NSString class]]) {
        // arguments[0] will be event name, so it must be a string type value here.
        return;
    }
    NSMutableArray * methodArguments = [method.arguments mutableCopy];
    if ([methodArguments count] == 2) {
        [methodArguments addObject:@{@"once": @false}];
    }
    if (![methodArguments[2] isKindOfClass:[NSDictionary class]]) {
        //arguments[2] is the option value, so it must be a dictionary.
        return;
    }
    Class moduleClass =  [WXModuleFactory classWithModuleName:method.moduleName];
    NSMutableDictionary * option = [methodArguments[2] mutableCopy];
    [option setObject:method.moduleName forKey:@"moduleName"];
    // the value for moduleName in option is for the need of callback
    [self addModuleEventObservers:methodArguments[0] callback:methodArguments[1] option:option moduleClassName:NSStringFromClass(moduleClass)];
}

- (void)addModuleEventObservers:(NSString*)event callback:(NSString*)callbackId option:(NSDictionary *)option moduleClassName:(NSString*)moduleClassName
{
    BOOL once = [[option objectForKey:@"once"] boolValue];
    NSString * moduleName = [option objectForKey:@"moduleName"];
    NSMutableDictionary * observer = nil;
    NSDictionary * callbackInfo = @{@"callbackId":callbackId,@"once":@(once),@"moduleName":moduleName};
    if(![self checkModuleEventRegistered:event moduleClassName:moduleClassName]) {
        //had not registered yet
        observer = [NSMutableDictionary new];
        [observer setObject:[@{event:[@[callbackInfo] mutableCopy]} mutableCopy] forKey:moduleClassName];
        if (_moduleEventObservers[moduleClassName]) { //support multi event
            [_moduleEventObservers[moduleClassName] addEntriesFromDictionary:observer[moduleClassName]];
        }else {
            [_moduleEventObservers addEntriesFromDictionary:observer];
        }
    } else {
        observer = _moduleEventObservers[moduleClassName];
        [[observer objectForKey:event] addObject:callbackInfo];
    }
}

- (void)_removeModuleEventObserverWithModuleMethod:(WXModuleMethod *)method
{
    if (![method.arguments count] && [method.arguments[0] isKindOfClass:[NSString class]]) {
        return;
    }
    Class moduleClass =  [WXModuleFactory classWithModuleName:method.moduleName];
    [self removeModuleEventObserver:method.arguments[0] moduleClassName:NSStringFromClass(moduleClass)];
}

- (void)removeModuleEventObserver:(NSString*)event moduleClassName:(NSString*)moduleClassName
{
    if (![self checkModuleEventRegistered:event moduleClassName:moduleClassName]) {
        return;
    }
    [_moduleEventObservers[moduleClassName] removeObjectForKey:event];
}

- (void)moduleEventNotification:(NSNotification *)notification
{
    NSMutableDictionary *moduleEventObserversCpy = (NSMutableDictionary *)CFBridgingRelease(CFPropertyListCreateDeepCopy(kCFAllocatorDefault, (CFDictionaryRef)_moduleEventObservers, kCFPropertyListMutableContainers));// deep
    NSDictionary * userInfo = notification.userInfo;
    NSMutableArray * listeners = [moduleEventObserversCpy[userInfo[@"moduleId"]] objectForKey:userInfo[@"eventName"]];
    if (![listeners isKindOfClass:[NSArray class]]) {
        return;
        // something wrong
    }
    for (int i = 0;i < [listeners count]; i ++) {
        NSDictionary * callbackInfo = listeners[i];
        NSString *callbackId = callbackInfo[@"callbackId"];
        BOOL once = [callbackInfo[@"once"] boolValue];
        NSDictionary * retData = @{@"type":userInfo[@"eventName"],
                                   @"module":callbackInfo[@"moduleName"],
                                   @"data":userInfo[@"param"]};
        [[WXSDKManager bridgeMgr] callBack:self.instanceId funcId:callbackId params:retData keepAlive:!once];
        // if callback function is not once, then it is keepalive
        if (once) {
            NSMutableArray * moduleEventListener = [_moduleEventObservers[userInfo[@"moduleId"]] objectForKey:userInfo[@"eventName"]];
            [moduleEventListener removeObject:callbackInfo];
            if ([moduleEventListener count] == 0) {
                [self removeModuleEventObserver:userInfo[@"eventName"] moduleClassName:userInfo[@"moduleId"]];
            }
            // if callback function is once. clear it after fire it.
        }
    }
}

- (void)addObservers
{
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(moduleEventNotification:) name:WX_MODULE_EVENT_FIRE_NOTIFICATION object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(applicationWillResignActive:) name:UIApplicationWillResignActiveNotification object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(applicationDidBecomeActive:) name:UIApplicationDidBecomeActiveNotification object:nil];
    [self addObserver:self forKeyPath:@"state" options:NSKeyValueObservingOptionNew context:NULL];
}

- (void)removeObservers
{
    @try {
        [self removeObserver:self forKeyPath:@"state" context:NULL];
        [[NSNotificationCenter defaultCenter] removeObserver:self];
    }
    @catch (NSException *exception) {//!OCLint
    }
}

- (void)applicationWillResignActive:(NSNotification*)notification
{
    [self fireGlobalEvent:WX_APPLICATION_WILL_RESIGN_ACTIVE params:nil];
}

- (void)applicationDidBecomeActive:(NSNotification*)notification
{
    [self fireGlobalEvent:WX_APPLICATION_DID_BECOME_ACTIVE params:nil];
}

- (WXComponentManager *)componentManager
{
    if (!_componentManager) {
        _componentManager = [[WXComponentManager alloc] initWithWeexInstance:self];
    }
    
    return _componentManager;
}

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
    if ([keyPath isEqualToString:@"state"]) {
        WXState state = [change[@"new"] longValue];
        [self updateState:state];
        
        if (state == WeexInstanceDestroy) {
            [self destroyInstance];
        }
    }
}

- (void)willAppear
{
    if (self.isCustomRenderType) {
        if (!self.appearState) {
            // do create window,
            [[NSNotificationCenter defaultCenter] postNotificationName:WX_INSTANCE_NOTIFICATION_CHANGE_VISIBILITY_INTERNAL object:self userInfo:@{@"visible": @(YES)}];
            self.appearState = YES;
        }
    }
}

- (void)didDisappear
{
    if (self.isCustomRenderType) {
        if (self.appearState) {
            // do destroy window
            [[NSNotificationCenter defaultCenter] postNotificationName:WX_INSTANCE_NOTIFICATION_CHANGE_VISIBILITY_INTERNAL object:self userInfo:@{@"visible": @(NO)}];
            self.appearState = NO;
        }
    }
}

+ (NSDictionary*)lastPageInfo
{
    NSDictionary* result;
    @synchronized (lastPageInfoLock) {
        result = [lastPageInfo copy];
    }
    return result;
}

@end

@implementation WXSDKInstance (Deprecated)

# pragma mark - Deprecated

- (void)reloadData:(id)data
{
    [self refreshInstance:data];
}

- (void)finishPerformance
{
    //deprecated
}

- (void)creatFinish
{
    
}

@end
