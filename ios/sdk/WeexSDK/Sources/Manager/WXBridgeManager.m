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
#import "WXServiceFactory.h"
#import "WXResourceRequest.h"
#import "WXResourceLoader.h"
#import "WXDebugTool.h"

@interface WXBridgeManager ()

@property (nonatomic, strong) WXBridgeContext   *bridgeCtx;
@property (nonatomic, assign) BOOL  stopRunning;
@property (nonatomic, strong) NSMutableArray *instanceIdStack;

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

- (void)dealloc
{
   
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


void WXPerformBlockOnBridgeThread(void (^block)())
{
    [WXBridgeManager _performBlockOnBridgeThread:block];
}

+ (void)_performBlockOnBridgeThread:(void (^)())block
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

#pragma mark JSBridge Management

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
    __weak typeof(self) weakSelf = self;
    WXPerformBlockOnBridgeThread(^(){
        [weakSelf.bridgeCtx createInstance:instance
                                  template:temp
                                   options:options
                                      data:data];
    });
}

- (NSMutableArray *)instanceIdStack
{
    if (_instanceIdStack) return _instanceIdStack;
    
    _instanceIdStack = [NSMutableArray array];
    
    return _instanceIdStack;
}

- (NSArray *)getInstanceIdStack;
{
    return self.instanceIdStack;
}

- (void)destroyInstance:(NSString *)instance
{
    if (!instance) return;
    if([self.instanceIdStack containsObject:instance]){
        [self.instanceIdStack removeObject:instance];
    }
    
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

-(void)registerService:(NSString *)name withServiceUrl:(NSURL *)serviceScriptUrl withOptions:(NSDictionary *)options
{
    if (!name || !serviceScriptUrl || !options) return;
    __weak typeof(self) weakSelf = self;
    WXResourceRequest *request = [WXResourceRequest requestWithURL:serviceScriptUrl resourceType:WXResourceTypeServiceBundle referrer:@"" cachePolicy:NSURLRequestUseProtocolCachePolicy];
    WXResourceLoader *serviceBundleLoader = [[WXResourceLoader alloc] initWithRequest:request];;
    serviceBundleLoader.onFinished = ^(WXResourceResponse *response, NSData *data) {
        __strong typeof(weakSelf) strongSelf = weakSelf;
        NSString *jsServiceString = [[NSString alloc] initWithData:data encoding:NSUTF8StringEncoding];
        [strongSelf registerService:name withService:jsServiceString withOptions:options];
    };
    
    serviceBundleLoader.onFailed = ^(NSError *loadError) {
        WXLogError(@"No script URL found");
    };
    
    [serviceBundleLoader start];
}

- (void)registerService:(NSString *)name withService:(NSString *)serviceScript withOptions:(NSDictionary *)options
{
    if (!name || !serviceScript || !options) return;
    
    NSString *script = [WXServiceFactory registerServiceScript:name withRawScript:serviceScript withOptions:options];
    
    __weak typeof(self) weakSelf = self;
    WXPerformBlockOnBridgeThread(^(){
        // save it when execute
        [WXDebugTool cacheJsService:name withScript:serviceScript withOptions:options];
        [weakSelf.bridgeCtx executeJsService:script withName:name];
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
    if (!type || !ref) {
        WXLogError(@"Event type and component ref should not be nil");
        return;
    }
    
    NSArray *args = @[ref, type, params?:@{}, domChanges?:@{}];
    WXSDKInstance *instance = [WXSDKManager instanceForID:instanceId];
    
    WXCallJSMethod *method = [[WXCallJSMethod alloc] initWithModuleName:nil methodName:@"fireEvent" arguments:args instance:instance];
    [self callJsMethod:method];
}

- (void)callBack:(NSString *)instanceId funcId:(NSString *)funcId params:(id)params keepAlive:(BOOL)keepAlive
{
    NSArray *args = nil;
    if (keepAlive) {
        args = @[[funcId copy], params? [params copy]:@"\"{}\"", @true];
    }else {
        args = @[[funcId copy], params? [params copy]:@"\"{}\""];
    }
    WXSDKInstance *instance = [WXSDKManager instanceForID:instanceId];

    WXCallJSMethod *method = [[WXCallJSMethod alloc] initWithModuleName:@"jsBridge" methodName:@"callback" arguments:args instance:instance];
    [self callJsMethod:method];
}

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
