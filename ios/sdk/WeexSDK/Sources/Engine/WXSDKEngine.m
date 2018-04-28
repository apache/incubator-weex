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

#import "WXSDKEngine.h"
#import "WXDebugTool.h"
#import "WXModuleFactory.h"
#import "WXHandlerFactory.h"
#import "WXComponentFactory.h"
#import "WXBridgeManager.h"

#import "WXAppConfiguration.h"
#import "WXResourceRequestHandlerDefaultImpl.h"
#import "WXNavigationDefaultImpl.h"
#import "WXURLRewriteDefaultImpl.h"

#import "WXSDKManager.h"
#import "WXSDKError.h"
#import "WXMonitor.h"
#import "WXSimulatorShortcutManager.h"
#import "WXAssert.h"
#import "WXLog.h"
#import "WXUtility.h"
#import "WXExtendCallNativeManager.h"
#import "WXExceptionUtils.h"
#import "WXConfigCenterProtocol.h"
#import "WXComponent+Layout.h"

@implementation WXSDKEngine

# pragma mark Module Register

// register some default modules when the engine initializes.
+ (void)_registerDefaultModules
{
    [self registerModule:@"dom" withClass:NSClassFromString(@"WXDomModule")];
    [self registerModule:@"locale" withClass:NSClassFromString(@"WXLocaleModule")];
    [self registerModule:@"navigator" withClass:NSClassFromString(@"WXNavigatorModule")];
    [self registerModule:@"stream" withClass:NSClassFromString(@"WXStreamModule")];
    [self registerModule:@"animation" withClass:NSClassFromString(@"WXAnimationModule")];
    [self registerModule:@"modal" withClass:NSClassFromString(@"WXModalUIModule")];
    [self registerModule:@"webview" withClass:NSClassFromString(@"WXWebViewModule")];
    [self registerModule:@"instanceWrap" withClass:NSClassFromString(@"WXInstanceWrap")];
    [self registerModule:@"timer" withClass:NSClassFromString(@"WXTimerModule")];
    [self registerModule:@"storage" withClass:NSClassFromString(@"WXStorageModule")];
    [self registerModule:@"clipboard" withClass:NSClassFromString(@"WXClipboardModule")];
    [self registerModule:@"globalEvent" withClass:NSClassFromString(@"WXGlobalEventModule")];
    [self registerModule:@"canvas" withClass:NSClassFromString(@"WXCanvasModule")];
    [self registerModule:@"picker" withClass:NSClassFromString(@"WXPickerModule")];
    [self registerModule:@"meta" withClass:NSClassFromString(@"WXMetaModule")];
    [self registerModule:@"webSocket" withClass:NSClassFromString(@"WXWebSocketModule")];
    [self registerModule:@"voice-over" withClass:NSClassFromString(@"WXVoiceOverModule")];
}

+ (void)registerModule:(NSString *)name withClass:(Class)clazz
{
    WXAssert(name && clazz, @"Fail to register the module, please check if the parameters are correct ！");
    if (!clazz || !name) {
        return;
    }
    NSString *moduleName = [WXModuleFactory registerModule:name withClass:clazz];
    NSDictionary *dict = [WXModuleFactory moduleMethodMapsWithName:moduleName];
    
    [[WXSDKManager bridgeMgr] registerModules:dict];
}

# pragma mark Component Register

// register some default components when the engine initializes.
+ (void)_registerDefaultComponents
{
    [self registerComponent:@"container" withClass:NSClassFromString(@"WXDivComponent") withProperties:nil];
    [self registerComponent:@"div" withClass:NSClassFromString(@"WXComponent") withProperties:nil];
    [self registerComponent:@"text" withClass:NSClassFromString(@"WXTextComponent") withProperties:nil];
    [self registerComponent:@"image" withClass:NSClassFromString(@"WXImageComponent") withProperties:nil];
    [self registerComponent:@"scroller" withClass:NSClassFromString(@"WXScrollerComponent") withProperties:nil];
    [self registerComponent:@"list" withClass:NSClassFromString(@"WXListComponent") withProperties:nil];
    [self registerComponent:@"recycler" withClass:NSClassFromString(@"WXRecyclerComponent") withProperties:nil];
    [self registerComponent:@"waterfall" withClass:NSClassFromString(@"WXRecyclerComponent") withProperties:nil];
    
    [self registerComponent:@"header" withClass:NSClassFromString(@"WXHeaderComponent")];
    [self registerComponent:@"cell" withClass:NSClassFromString(@"WXCellComponent")];
    [self registerComponent:@"embed" withClass:NSClassFromString(@"WXEmbedComponent")];
    [self registerComponent:@"a" withClass:NSClassFromString(@"WXAComponent")];
    
    [self registerComponent:@"select" withClass:NSClassFromString(@"WXSelectComponent")];
    [self registerComponent:@"switch" withClass:NSClassFromString(@"WXSwitchComponent")];
    [self registerComponent:@"input" withClass:NSClassFromString(@"WXTextInputComponent")];
    [self registerComponent:@"video" withClass:NSClassFromString(@"WXVideoComponent")];
    [self registerComponent:@"indicator" withClass:NSClassFromString(@"WXIndicatorComponent")];
    [self registerComponent:@"slider" withClass:NSClassFromString(@"WXCycleSliderComponent")];
    [self registerComponent:@"cycleslider" withClass:NSClassFromString(@"WXCycleSliderComponent")];
    [self registerComponent:@"web" withClass:NSClassFromString(@"WXWebComponent")];
    [self registerComponent:@"loading" withClass:NSClassFromString(@"WXLoadingComponent")];
    [self registerComponent:@"loading-indicator" withClass:NSClassFromString(@"WXLoadingIndicator")];
    [self registerComponent:@"refresh" withClass:NSClassFromString(@"WXRefreshComponent")];
    [self registerComponent:@"textarea" withClass:NSClassFromString(@"WXTextAreaComponent")];
	[self registerComponent:@"canvas" withClass:NSClassFromString(@"WXCanvasComponent")];
    [self registerComponent:@"slider-neighbor" withClass:NSClassFromString(@"WXSliderNeighborComponent")];
    
    [self registerComponent:@"recycle-list" withClass:NSClassFromString(@"WXRecycleListComponent")];
    [self registerComponent:@"cell-slot" withClass:NSClassFromString(@"WXCellSlotComponent") withProperties: @{@"append":@"tree", @"isTemplate":@YES}];
    
}

+ (void)registerComponent:(NSString *)name withClass:(Class)clazz
{
    [self registerComponent:name withClass:clazz withProperties: @{@"append":@"tree"}];
}

+ (void)registerExtendCallNative:(NSString *)name withClass:(Class)clazz
{
    [WXExtendCallNativeManager registerExtendCallNative:name withClass:clazz];
}

+ (void)registerComponent:(NSString *)name withClass:(Class)clazz withProperties:(NSDictionary *)properties
{
    if (!name || !clazz) {
        return;
    }

    WXAssert(name && clazz, @"Fail to register the component, please check if the parameters are correct ！");
    
    [WXComponentFactory registerComponent:name withClass:clazz withPros:properties];
    NSMutableDictionary *dict = [WXComponentFactory componentMethodMapsWithName:name];
    dict[@"type"] = name;
    if (properties) {
        NSMutableDictionary *props = [properties mutableCopy];
        if ([dict[@"methods"] count]) {
            [props addEntriesFromDictionary:dict];
        }
        [[WXSDKManager bridgeMgr] registerComponents:@[props]];
    } else {
        [[WXSDKManager bridgeMgr] registerComponents:@[dict]];
    }
}


# pragma mark Service Register
+ (void)registerService:(NSString *)name withScript:(NSString *)serviceScript withOptions:(NSDictionary *)options
{
    [[WXSDKManager bridgeMgr] registerService:name withService:serviceScript withOptions:options];
}

+ (void)registerService:(NSString *)name withScriptUrl:(NSURL *)serviceScriptUrl WithOptions:(NSDictionary *)options
{
    [[WXSDKManager bridgeMgr] registerService:name withServiceUrl:serviceScriptUrl withOptions:options];
}

+ (void)unregisterService:(NSString *)name
{
    [[WXSDKManager bridgeMgr] unregisterService:name];
}

# pragma mark Handler Register

// register some default handlers when the engine initializes.
+ (void)_registerDefaultHandlers
{
    [self registerHandler:[WXResourceRequestHandlerDefaultImpl new] withProtocol:@protocol(WXResourceRequestHandler)];
    [self registerHandler:[WXNavigationDefaultImpl new] withProtocol:@protocol(WXNavigationProtocol)];
    [self registerHandler:[WXURLRewriteDefaultImpl new] withProtocol:@protocol(WXURLRewriteProtocol)];
    
}

+ (void)registerHandler:(id)handler withProtocol:(Protocol *)protocol
{
    WXAssert(handler && protocol, @"Fail to register the handler, please check if the parameters are correct ！");
    
    [WXHandlerFactory registerHandler:handler withProtocol:protocol];
}

+ (id)handlerForProtocol:(Protocol *)protocol
{
    WXAssert(protocol, @"Fail to get the handler, please check if the parameters are correct ！");
    
    return  [WXHandlerFactory handlerForProtocol:protocol];
}

# pragma mark SDK Initialize

+ (void)initSDKEnvironment
{
    NSString *fileName = @"weex-main-jsfm";
    [WXSDKManager sharedInstance].multiContext = YES;
    
    if ([[NSUserDefaults standardUserDefaults] objectForKey:@"createInstanceUsingMutliContext"]) {
        BOOL createInstanceUsingMutliContext = [[[NSUserDefaults standardUserDefaults] objectForKey:@"createInstanceUsingMutliContext"] boolValue];
        if (!createInstanceUsingMutliContext) {
            fileName = @"native-bundle-main";
            [WXSDKManager sharedInstance].multiContext = NO;
        }
    }
    NSString *filePath = [[NSBundle bundleForClass:self] pathForResource:fileName ofType:@"js"];
    NSString *script = [NSString stringWithContentsOfFile:filePath encoding:NSUTF8StringEncoding error:nil];
    [WXSDKEngine initSDKEnvironment:script];
    
#if TARGET_OS_SIMULATOR
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        [WXSimulatorShortcutManager registerSimulatorShortcutWithKey:@"i" modifierFlags:UIKeyModifierCommand | UIKeyModifierAlternate action:^{
            NSURL *URL = [NSURL URLWithString:@"http://localhost:8687/launchDebugger"];
            NSURLRequest *request = [NSURLRequest requestWithURL:URL];
            
            NSURLSession *session = [NSURLSession sharedSession];
            NSURLSessionDataTask *task = [session dataTaskWithRequest:request
                                                    completionHandler:
                                          ^(NSData *data, NSURLResponse *response, NSError *error) {
                                              // ...
                                          }];
            
            [task resume];
            WXLogInfo(@"Launching browser...");
            
            dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(1 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
                [self connectDebugServer:@"ws://localhost:8687/debugger/0/renderer"];
            });
            
        }];
    });
#endif
}

+ (void)initSDKEnvironment:(NSString *)script
{
    WX_MONITOR_PERF_START(WXPTInitalize)
    WX_MONITOR_PERF_START(WXPTInitalizeSync)
    
    if (!script || script.length <= 0) {
        NSMutableString *errMsg = [NSMutableString stringWithFormat:@"[WX_KEY_EXCEPTION_SDK_INIT_JSFM_INIT_FAILED] script don't exist:%@",script];
        [WXExceptionUtils commitCriticalExceptionRT:@"WX_KEY_EXCEPTION_SDK_INIT" errCode:[NSString stringWithFormat:@"%d", WX_KEY_EXCEPTION_SDK_INIT] function:@"initSDKEnvironment" exception:errMsg extParams:nil];
        WX_MONITOR_FAIL(WXMTJSFramework, WX_ERR_JSFRAMEWORK_LOAD, errMsg);
        return;
    }
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        [self registerDefaults];
        [[WXSDKManager bridgeMgr] executeJsFramework:script];
    });
    
    WX_MONITOR_PERF_END(WXPTInitalizeSync)
    
}

+ (void)registerDefaults
{
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        [self _registerDefaultComponents];
        [self _registerDefaultModules];
        [self _registerDefaultHandlers];
    });
}

+ (NSString*)SDKEngineVersion
{
    return WX_SDK_VERSION;
}

+ (WXSDKInstance *)topInstance
{
    return [WXSDKManager bridgeMgr].topInstance;
}


static NSDictionary *_customEnvironment;
+ (void)setCustomEnvironment:(NSDictionary *)environment
{
    _customEnvironment = environment;
}

+ (NSDictionary *)customEnvironment
{
    return _customEnvironment;
}

# pragma mark Debug

+ (void)unload
{
    [WXSDKManager unload];
    [WXComponentFactory unregisterAllComponents];
}

+ (void)restart
{
    NSString *fileName = @"weex-main-jsfm";
    [WXSDKManager sharedInstance].multiContext = YES;
    if ([[NSUserDefaults standardUserDefaults] objectForKey:@"createInstanceUsingMutliContext"]) {
        BOOL createInstanceUsingMutliContext = [[[NSUserDefaults standardUserDefaults] objectForKey:@"createInstanceUsingMutliContext"] boolValue];
        if (!createInstanceUsingMutliContext) {
            fileName = @"native-bundle-main";
            [WXSDKManager sharedInstance].multiContext = NO;
        }
    }
    NSString *filePath = [[NSBundle bundleForClass:self] pathForResource:fileName ofType:@"js"];
    NSString *script = [NSString stringWithContentsOfFile:filePath encoding:NSUTF8StringEncoding error:nil];
    [self restartWithScript:script];
}

+ (void)restartWithScript:(NSString*)script
{
    NSDictionary *components = [WXComponentFactory componentConfigs];
    NSDictionary *modules = [WXModuleFactory moduleConfigs];
    NSDictionary *handlers = [WXHandlerFactory handlerConfigs];
    [WXSDKManager unload];
    [WXComponentFactory unregisterAllComponents];
    
    [self _originalRegisterComponents:components];
    [self _originalRegisterModules:modules];
    [self _originalRegisterHandlers:handlers];
    
    [[WXSDKManager bridgeMgr] executeJsFramework:script];
    
    NSDictionary *jsSerices = [WXDebugTool jsServiceCache];
    for(NSString *serviceName in jsSerices) {
        NSDictionary *service = [jsSerices objectForKey:serviceName];
        NSString *serviceName = [service objectForKey:@"name"];
        NSString *serviceScript = [service objectForKey:@"script"];
        NSDictionary *serviceOptions = [service objectForKey:@"options"];
        [WXSDKEngine registerService:serviceName withScript:serviceScript withOptions:serviceOptions];
    }
}

+ (void)connectDebugServer:(NSString*)URL
{
    [[WXSDKManager bridgeMgr] connectToWebSocket:[NSURL URLWithString:URL]];
}

+ (void)connectDevToolServer:(NSString *)URL
{
    [[WXSDKManager bridgeMgr] connectToDevToolWithUrl:[NSURL URLWithString:URL]];
}

+ (void)_originalRegisterComponents:(NSDictionary *)components {
    NSMutableDictionary * mutableComponents = [components mutableCopy];
    void (^componentBlock)(id, id, BOOL *) = ^(id mKey, id mObj, BOOL * mStop) {
        
        NSString *name = mObj[@"name"];
        NSString *componentClass = mObj[@"clazz"];
        NSDictionary *pros = nil;
        if (mObj[@"pros"]) {
            pros = mObj[@""];
        }
        [self registerComponent:name withClass:NSClassFromString(componentClass) withProperties:pros];
    };
    [mutableComponents enumerateKeysAndObjectsUsingBlock:componentBlock];
    
}

+ (void)_originalRegisterModules:(NSDictionary *)modules {
    NSMutableDictionary * mutableModules = [modules mutableCopy];
    void (^moduleBlock)(id, id, BOOL *) = ^(id mKey, id mObj, BOOL * mStop) {
        
        [self registerModule:mKey withClass:NSClassFromString(mObj)];
    };
    [mutableModules enumerateKeysAndObjectsUsingBlock:moduleBlock];
}

+ (void)_originalRegisterHandlers:(NSDictionary *)handlers {
    NSMutableDictionary * mutableHandlers = [handlers mutableCopy];
    void (^handlerBlock)(id, id, BOOL *) = ^(id mKey, id mObj, BOOL * mStop) {
        [self registerHandler:mObj withProtocol:NSProtocolFromString(mKey)];
    };
    [mutableHandlers enumerateKeysAndObjectsUsingBlock:handlerBlock];
}

@end

@implementation WXSDKEngine (Deprecated)

+ (void)initSDKEnviroment
{
    [self initSDKEnvironment];
}

+ (void)initSDKEnviroment:(NSString *)script
{
    [self initSDKEnvironment:script];
}

@end
