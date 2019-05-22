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

#import "WXBridgeManager.h"
#import "WXBridgeContext.h"
#import "WXLog.h"
#import "WXAssert.h"
#import "WXBridgeMethod.h"
#import "WXCallJSMethod.h"
#import "WXSDKManager.h"
#import "WXSDKInstance_private.h"
#import "WXServiceFactory.h"
#import "WXResourceRequest.h"
#import "WXResourceLoader.h"
#import "WXDebugTool.h"
#import "WXMonitor.h"
#import "WXSDKInstance_performance.h"
#import "WXThreadSafeMutableArray.h"
#import "WXComponentManager.h"
#import "WXCoreBridge.h"
#import "WXDataRenderHandler.h"
#import "WXHandlerFactory.h"

@interface WXBridgeManager ()

@property (nonatomic, assign) BOOL stopRunning;
@property (nonatomic, strong) WXBridgeContext *bridgeCtx;
@property (nonatomic, strong) WXThreadSafeMutableArray *instanceIdStack;

@end

static NSThread *WXBridgeThread;

@implementation WXBridgeManager

+ (instancetype)sharedManager
{
    static id _sharedInstance = nil;
    static dispatch_once_t oncePredicate;
    dispatch_once(&oncePredicate, ^{
        _sharedInstance = [[self alloc] init];
    });
    return _sharedInstance;
}

- (instancetype)init
{
    self = [super init];
    if (self) {
        _bridgeCtx = [[WXBridgeContext alloc] init];
    }
    return self;
}

- (WXSDKInstance *)topInstance
{
    return _bridgeCtx.topInstance;
}

- (void)unload
{
    _bridgeCtx = nil;
}

#pragma mark Thread Management

- (void)_runLoopThread
{
    [[NSRunLoop currentRunLoop] addPort:[NSMachPort port] forMode:NSDefaultRunLoopMode];
    
    while (!_stopRunning) {
        @autoreleasepool {
            [[NSRunLoop currentRunLoop] runMode:NSDefaultRunLoopMode beforeDate:[NSDate distantFuture]];
        }
    }
}

+ (NSThread *)jsThread
{
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        
        WXBridgeThread = [[NSThread alloc] initWithTarget:[[self class]sharedManager] selector:@selector(_runLoopThread) object:nil];
        [WXBridgeThread setName:WX_BRIDGE_THREAD_NAME];
        if(WX_SYS_VERSION_GREATER_THAN_OR_EQUAL_TO(@"8.0")) {
            [WXBridgeThread setQualityOfService:[[NSThread mainThread] qualityOfService]];
        } else {
            [WXBridgeThread setThreadPriority:[[NSThread mainThread] threadPriority]];
        }
        
        [WXBridgeThread start];
    });
    
    return WXBridgeThread;
}


void WXPerformBlockOnBridgeThread(void (^block)(void))
{
    [WXBridgeManager _performBlockOnBridgeThread:block];
}

+ (void)_performBlockOnBridgeThread:(void (^)(void))block
{
    if ([NSThread currentThread] == [self jsThread]) {
        block();
    } else {
        [self performSelector:@selector(_performBlockOnBridgeThread:)
                     onThread:[self jsThread]
                   withObject:[block copy]
                waitUntilDone:NO];
    }
}

void WXPerformBlockSyncOnBridgeThread(void (^block) (void))
{
    [WXBridgeManager _performBlockSyncOnBridgeThread:block];
}

+ (void)_performBlockSyncOnBridgeThread:(void (^)(void))block
{
    if ([NSThread currentThread] == [self jsThread]) {
        block();
    } else {
        [self performSelector:@selector(_performBlockSyncOnBridgeThread:)
                     onThread:[self jsThread]
                   withObject:[block copy]
                waitUntilDone:YES];
    }
}

#pragma mark JSBridge Management
- (void)createInstanceForJS:(NSString *)instance
              template:(NSString *)temp
               options:(NSDictionary *)options
                  data:(id)data {
    if (!instance || !temp) return;
    __weak typeof(self) weakSelf = self;
    NSMutableDictionary *newOptions = [options mutableCopy] ?: [NSMutableDictionary new];
    newOptions[@"EXEC_JS"] = @(YES);
    WXPerformBlockOnBridgeThread(^(){
        [weakSelf.bridgeCtx createInstance:instance
                                  template:temp
                                   options:newOptions
                                      data:data];
    });
}

- (void)createInstance:(NSString *)instance
              template:(NSString *)temp
               options:(NSDictionary *)options
                  data:(id)data
{
    if (!instance || !temp) return;
    if (![self.instanceIdStack containsObject:instance]) {
        if ([options[@"RENDER_IN_ORDER"] boolValue]) {
            [self.instanceIdStack addObject:instance];
        } else {
            [self.instanceIdStack insertObject:instance atIndex:0];
        }
    }
    //third team impl...
    WXSDKInstance* sdkInstance = [WXSDKManager instanceForID:instance];
    if (sdkInstance) {
        sdkInstance.apmInstance.isStartRender = YES;
    }
    __weak typeof(self) weakSelf = self;
    WXPerformBlockOnBridgeThread(^(){
        [weakSelf.bridgeCtx createInstance:instance
                                  template:temp
                                   options:options
                                      data:data];
    });
}

- (void)createInstance:(NSString *)instance
              contents:(NSData *)contents
               options:(NSDictionary *)options
                  data:(id)data
{
    if (!instance || !contents) return;
    if (![self.instanceIdStack containsObject:instance]) {
        if ([options[@"RENDER_IN_ORDER"] boolValue]) {
            [self.instanceIdStack addObject:instance];
        } else {
            [self.instanceIdStack insertObject:instance atIndex:0];
        }
    }
    //third team impl...
    WXSDKInstance* sdkInstance = [WXSDKManager instanceForID:instance];
    if (sdkInstance) {
        sdkInstance.apmInstance.isStartRender = YES;
    }
    __weak typeof(self) weakSelf = self;
    WXPerformBlockOnBridgeThread(^(){
        [weakSelf.bridgeCtx createInstance:instance
                                  contents:contents
                                   options:options
                                      data:data];
    });
}

- (WXThreadSafeMutableArray *)instanceIdStack
{
    if (_instanceIdStack) return _instanceIdStack;
    
    _instanceIdStack = [[WXThreadSafeMutableArray alloc] init];
    
    return _instanceIdStack;
}

- (NSArray *)getInstanceIdStack;
{
    return [self.instanceIdStack copy];
}

- (void)destroyInstance:(NSString *)instance
{
    if (!instance) return;
    [self.instanceIdStack removeObject:instance];
    
    __weak typeof(self) weakSelf = self;
    WXPerformBlockOnBridgeThread(^(){
        [weakSelf.bridgeCtx destroyInstance:instance];
    });
}

- (void)forceGarbageCollection
{
    __weak typeof(self) weakSelf = self;
    WXPerformBlockOnBridgeThread(^(){
        [weakSelf.bridgeCtx forceGarbageCollection];
    });
}

- (void)refreshInstance:(NSString *)instance
                   data:(NSDictionary *)data
{
    if (!instance) return;
    
    __weak typeof(self) weakSelf = self;
    WXPerformBlockOnBridgeThread(^(){
        [weakSelf.bridgeCtx refreshInstance:instance data:data];
    });
}

- (void)updateState:(NSString *)instance data:(id)data
{
    if (!instance) return;
    
    __weak typeof(self) weakSelf = self;
    WXPerformBlockOnBridgeThread(^(){
        [weakSelf.bridgeCtx updateState:instance data:data];
    });
}

- (void)executeJsFramework:(NSString *)script
{
    if (!script) return;
    
    __weak typeof(self) weakSelf = self;
    WXPerformBlockOnBridgeThread(^(){
        [weakSelf.bridgeCtx executeJsFramework:script];
    });
}

- (void)callJsMethod:(WXCallJSMethod *)method
{
    if (!method) return;
    
    __weak typeof(self) weakSelf = self;
    WXPerformBlockOnBridgeThread(^(){
        [weakSelf.bridgeCtx executeJsMethod:method];
    });
}

- (JSValue *)callJSMethodWithResult:(WXCallJSMethod *)method
{
    if (!method) return nil;
    __weak typeof(self) weakSelf = self;
    __block JSValue *value;
    WXPerformBlockSyncOnBridgeThread(^(){
        value = [weakSelf.bridgeCtx excuteJSMethodWithResult:method];
    });
    return value;
}

- (void)DownloadJS:(NSString*)instance url:(NSURL *)scriptUrl completion:(void (^)(NSString *script))complection;
{
    if (!scriptUrl || ![scriptUrl.absoluteString length]) {
        if (complection) {
            complection(nil);
        }
        return;
    }
    WXResourceRequest* request = [WXResourceRequest requestWithURL:scriptUrl];
    WXResourceLoader* jsLoader = [[WXResourceLoader alloc] initWithRequest:request];
    jsLoader.onFinished = ^(WXResourceResponse *response, NSData *data) {
        NSString* jsString = [[NSString alloc] initWithData:data encoding:NSUTF8StringEncoding];
        if (complection) {
            complection(jsString);
        }
    };
    jsLoader.onFailed = ^(NSError *loadError) {
        if (complection) {
            complection(nil);
        }

        WXSDKInstance *sdkInstance = [WXSDKManager instanceForID:instance];
        WXComponentManager *manager = sdkInstance.componentManager;
        if (manager.isValid) {
            NSString *errorMessage = [NSString stringWithFormat:@"Request to %@ occurs an error:%@, info:%@", request.URL, loadError.localizedDescription, loadError.userInfo];
            WXSDKErrCode errorCode = WX_KEY_EXCEPTION_JS_DOWNLOAD;
            NSError *error = [NSError errorWithDomain:WX_ERROR_DOMAIN code:errorCode userInfo:@{NSLocalizedDescriptionKey:(errorMessage?:@"No message")}];
            WXPerformBlockOnComponentThread(^{
                [manager renderFailed:error];
            });
        }
    };

   [jsLoader start];
}

- (void)registerService:(NSString *)name withServiceUrl:(NSURL *)serviceScriptUrl withOptions:(NSDictionary *)options completion:(void(^)(BOOL result))completion
{
    if (!name || !serviceScriptUrl || !options) {
        if (completion) {
            completion(NO);
        }
        return;
    }
    __weak typeof(self) weakSelf = self;
    WXResourceRequest *request = [WXResourceRequest requestWithURL:serviceScriptUrl resourceType:WXResourceTypeServiceBundle referrer:@"" cachePolicy:NSURLRequestUseProtocolCachePolicy];
    WXResourceLoader *serviceBundleLoader = [[WXResourceLoader alloc] initWithRequest:request];;
    serviceBundleLoader.onFinished = ^(WXResourceResponse *response, NSData *data) {
        __strong typeof(weakSelf) strongSelf = weakSelf;
        NSString *jsServiceString = [[NSString alloc] initWithData:data encoding:NSUTF8StringEncoding];
        [strongSelf registerService:name withService:jsServiceString withOptions:options completion:completion];
    };
    
    serviceBundleLoader.onFailed = ^(NSError *loadError) {
        WXLogError(@"No script URL found");
        if (completion) {
            completion(NO);
        }
    };
    
    [serviceBundleLoader start];
}

- (void)registerService:(NSString *)name withService:(NSString *)serviceScript withOptions:(NSDictionary *)options completion:(void(^)(BOOL result))completion
{
    if (!name || !serviceScript || !options) {
        if (completion) {
            completion(NO);
        }
        return;
    }
    
    NSString *script = [WXServiceFactory registerServiceScript:name withRawScript:serviceScript withOptions:options];
    
    __weak typeof(self) weakSelf = self;
    WXPerformBlockOnBridgeThread(^(){
        // save it when execute
        [WXDebugTool cacheJsService:name withScript:serviceScript withOptions:options];
        [weakSelf.bridgeCtx executeJsService:script withName:name];
        if (completion) {
            completion(YES);
        }
    });
}

- (void)unregisterService:(NSString *)name
{
    if (!name) return;
    
    NSString *script = [WXServiceFactory unregisterServiceScript:name];
    
    __weak typeof(self) weakSelf = self;
    WXPerformBlockOnBridgeThread(^(){
        // save it when execute
        [WXDebugTool removeCacheJsService:name];
        [weakSelf.bridgeCtx executeJsService:script withName:name];
    });
}

- (void)registerModules:(NSDictionary *)modules
{
    if (!modules) return;
    
    __weak typeof(self) weakSelf = self;
    WXPerformBlockOnBridgeThread(^(){
        [weakSelf.bridgeCtx registerModules:modules];
    });
}

- (void)registerComponents:(NSArray *)components
{
    if (!components) return;
    
    __weak typeof(self) weakSelf = self;
    WXPerformBlockOnBridgeThread(^(){
        [weakSelf.bridgeCtx registerComponents:components];
    });
}

- (void)fireEvent:(NSString *)instanceId ref:(NSString *)ref type:(NSString *)type params:(NSDictionary *)params
{
    [self fireEvent:instanceId ref:ref type:type params:params domChanges:nil];
}

- (void)fireEvent:(NSString *)instanceId ref:(NSString *)ref type:(NSString *)type params:(NSDictionary *)params domChanges:(NSDictionary *)domChanges
{
    [self fireEvent:instanceId ref:ref type:type params:params domChanges:domChanges handlerArguments:nil];
}

- (void)fireEvent:(NSString *)instanceId ref:(NSString *)ref type:(NSString *)type params:(NSDictionary *)params domChanges:(NSDictionary *)domChanges handlerArguments:(NSArray *)handlerArguments
{
    WXSDKInstance *instance = [WXSDKManager instanceForID:instanceId];
    if (instance.dataRender) {
        id<WXDataRenderHandler> dataRenderHandler = [WXHandlerFactory handlerForProtocol:@protocol(WXDataRenderHandler)];
        if (dataRenderHandler) {
            WXPerformBlockOnComponentThread(^{
                [dataRenderHandler fireEvent:instanceId ref:ref event:type args:params?:@{} domChanges:domChanges?:@{}];
            });
        }
        else {
            WXComponentManager *manager = instance.componentManager;
            if (manager.isValid) {
                WXSDKErrCode errorCode = WX_KEY_EXCEPTION_DEGRADE_EAGLE_RENDER_ERROR;
                NSError *error = [NSError errorWithDomain:WX_ERROR_DOMAIN code:errorCode userInfo:@{@"message":@"No data render handler found!"}];
                WXPerformBlockOnComponentThread(^{
                    [manager renderFailed:error];
                });
            }
        }
        return;
    }

    if (!type || !ref) {
        WXLogError(@"Event type and component ref should not be nil");
        return;
    }
    
    NSArray *args = @[ref, type, params?:@{}, domChanges?:@{}];
    if (handlerArguments) {
        NSMutableArray *newArgs = [args mutableCopy];
        [newArgs addObject:@{@"params":handlerArguments}];
        args = newArgs;
    }
    
    if(instance && !instance.isJSCreateFinish)
    {
        instance.performance.fsCallEventNum++;
    }
    if (instance && !instance.apmInstance.isFSEnd) {
        [instance.apmInstance updateFSDiffStats:KEY_PAGE_STATS_FS_CALL_EVENT_NUM withDiffValue:1];
    }
    
    WXCallJSMethod *method = [[WXCallJSMethod alloc] initWithModuleName:nil methodName:@"fireEvent" arguments:args instance:instance];
    [self callJsMethod:method];
}

- (void)callComponentHook:(NSString*)instanceId componentId:(NSString*)componentId type:(NSString*)type hook:(NSString*)hookPhase args:(NSArray*)args competion:(void (^)(JSValue * value))completion
{
    WXPerformBlockOnBridgeThread(^{
        if (!type || !instanceId || !hookPhase) {
            WXLogError(@"type and instance id and hookPhase should not be nil");
            return;
        }
        NSArray *newArgs = @[componentId, type, hookPhase, args?:@[]];
        
        WXCallJSMethod * method = [[WXCallJSMethod alloc] initWithModuleName:nil methodName:@"componentHook" arguments:newArgs instance:[WXSDKManager instanceForID:instanceId]];
        [self.bridgeCtx callJSMethod:@"callJS" args:@[instanceId, @[method.callJSTask]] onContext:nil completion:completion];
    });
}

- (JSValue *)fireEventWithResult:(NSString *)instanceId ref:(NSString *)ref type:(NSString *)type params:(NSDictionary *)params domChanges:(NSDictionary *)domChanges
{
    if (!type || !ref) {
        WXLogError(@"Event type and component ref should not be nil");
        return nil;
    }
    NSArray *args = @[ref, type, params?:@{}, domChanges?:@{}];
    WXSDKInstance *instance = [WXSDKManager instanceForID:instanceId];
    WXCallJSMethod *method = [[WXCallJSMethod alloc] initWithModuleName:nil methodName:@"fireEvent" arguments:args instance:instance];
    return [self callJSMethodWithResult:method];
}

- (void)callBack:(NSString *)instanceId funcId:(NSString *)funcId params:(id)params keepAlive:(BOOL)keepAlive
{
    NSArray *args = nil;
    if (keepAlive) {
        args = @[[funcId copy], params? [params copy]:@"\"{}\"", @true];
    } else {
        args = @[[funcId copy], params? [params copy]:@"\"{}\""];
    }
    WXSDKInstance *instance = [WXSDKManager instanceForID:instanceId];
    if (instance.wlasmRender) {
        id<WXDataRenderHandler> dataRenderHandler = [WXHandlerFactory handlerForProtocol:@protocol(WXDataRenderHandler)];
        if (dataRenderHandler) {
            id strongArgs = params ? [params copy]:@"\"{}\"";
            WXPerformBlockOnComponentThread(^{
                [dataRenderHandler invokeCallBack:instanceId function:funcId args:strongArgs keepAlive:keepAlive];
            });
        }
        else {
            WXComponentManager *manager = instance.componentManager;
            if (manager.isValid) {
                WXSDKErrCode errorCode = WX_KEY_EXCEPTION_DEGRADE_EAGLE_RENDER_ERROR;
                NSError *error = [NSError errorWithDomain:WX_ERROR_DOMAIN code:errorCode userInfo:@{@"message":@"No data render handler found!"}];
                WXPerformBlockOnComponentThread(^{
                    [manager renderFailed:error];
                });
            }
        }
    }
    else {
        WXCallJSMethod *method = [[WXCallJSMethod alloc] initWithModuleName:@"jsBridge" methodName:@"callback" arguments:args instance:instance];
        [self callJsMethod:method];
    }}

- (void)callBack:(NSString *)instanceId funcId:(NSString *)funcId params:(id)params
{
    [self callBack:instanceId funcId:funcId params:params keepAlive:NO];
}

- (void)connectToDevToolWithUrl:(NSURL *)url {
    [self.bridgeCtx connectToDevToolWithUrl:url];
}

- (void)connectToWebSocket:(NSURL *)url
{
    __weak typeof(self) weakSelf = self;
    WXPerformBlockOnBridgeThread(^(){
        [weakSelf.bridgeCtx connectToWebSocket:url];
    });
}

- (void)logToWebSocket:(NSString *)flag message:(NSString *)message
{
    if (!message) return;
    
    __weak typeof(self) weakSelf = self;
    WXPerformBlockOnBridgeThread(^(){
        [weakSelf.bridgeCtx logToWebSocket:flag message:message];
    });
}

- (void)resetEnvironment
{
    __weak typeof(self) weakSelf = self;
    WXPerformBlockOnBridgeThread(^(){
        [weakSelf.bridgeCtx resetEnvironment];
    });
}

- (void)callJSMethod:(NSString *)method args:(NSArray *)args
{
    if (!method) return;

    __weak typeof(self) weakSelf = self;
    WXPerformBlockOnBridgeThread(^(){
        [weakSelf.bridgeCtx callJSMethod:method args:args onContext:nil completion:nil];
    });
}

#pragma mark - Deprecated

- (void)executeJsMethod:(WXCallJSMethod *)method
{
    if (!method) return;
    
    __weak typeof(self) weakSelf = self;
    WXPerformBlockOnBridgeThread(^(){
        [weakSelf.bridgeCtx executeJsMethod:method];
    });
}

@end
