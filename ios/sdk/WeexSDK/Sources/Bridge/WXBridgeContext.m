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

#import "WXBridgeContext.h"
#import "WXJSCoreBridge.h"
#import "WXLog.h"
#import "WXUtility.h"
#import "WXModuleFactory.h"
#import "WXModuleProtocol.h"
#import "WXUtility.h"
#import "WXSDKError.h"
#import "WXMonitor.h"
#import "WXAssert.h"
#import "WXSDKManager.h"
#import "WXDebugTool.h"
#import "WXSDKInstance_private.h"
#import "WXAppConfiguration.h"
#import "WXInvocationConfig.h"
#import "WXComponentMethod.h"
#import "WXModuleMethod.h"
#import "WXCallJSMethod.h"
#import "WXSDKInstance_private.h"
#import "WXPrerenderManager.h"
#import "WXExceptionUtils.h"
#import "WXSDKEngine.h"
#import "WXPolyfillSet.h"
#import "WXJSExceptionProtocol.h"
#import "WXMonitor.h"
#import "WXAppMonitorProtocol.h"
#import "WXConfigCenterProtocol.h"
#import "WXSDKInstance_performance.h"
#import "JSContext+Weex.h"
#import "WXCoreBridge.h"
#import "WXJSFrameworkLoadProtocol.h"
#import "WXJSFrameworkLoadDefaultImpl.h"
#import "WXHandlerFactory.h"
#import "WXExtendCallNativeManager.h"
#import "WXDataRenderHandler.h"

#define SuppressPerformSelectorLeakWarning(Stuff) \
do { \
_Pragma("clang diagnostic push") \
_Pragma("clang diagnostic ignored \"-Warc-performSelector-leaks\"") \
Stuff; \
_Pragma("clang diagnostic pop") \
} while (0)

@interface WXBridgeContext ()

@property (nonatomic, strong) id<WXBridgeProtocol>  jsBridge;
@property (nonatomic, strong) id<WXBridgeProtocol> devToolSocketBridge;
@property (nonatomic, assign) BOOL  debugJS;
//store the methods which will be executed from native to js
@property (nonatomic, strong) NSMutableDictionary   *sendQueue;
//the instance stack
@property (nonatomic, strong) NSMutableArray    *insStack;
//identify if the JSFramework has been loaded
@property (nonatomic) BOOL frameworkLoadFinished;
//store some methods temporarily before JSFramework is loaded
@property (nonatomic, strong) NSMutableArray *methodQueue;
// store service
@property (nonatomic, strong) NSMutableArray *jsServiceQueue;

@property (nonatomic, readonly) id<WXDataRenderHandler> dataRenderHandler;

@end

@implementation WXBridgeContext
    
@synthesize dataRenderHandler = _dataRenderHandler;

- (instancetype) init
{
    self = [super init];
    if (self) {
        _methodQueue = [NSMutableArray new];
        _frameworkLoadFinished = NO;
        _jsServiceQueue = [NSMutableArray new];
        _dataRenderHandler = [WXHandlerFactory handlerForProtocol:@protocol(WXDataRenderHandler)];
        if (!_dataRenderHandler) {
            Class handlerClass = NSClassFromString(@"WXEagleHandler");
            if (handlerClass) {
                _dataRenderHandler = [[handlerClass alloc] init];
                [WXSDKEngine registerHandler:_dataRenderHandler withProtocol:@protocol(WXDataRenderHandler)];
            }
        }
    }
    return self;
}

- (id<WXBridgeProtocol>)jsBridge
{
    WXAssertBridgeThread();
    _debugJS = [WXDebugTool isDevToolDebug];
    
    Class bridgeClass = _debugJS ? NSClassFromString(@"WXDebugger") : [WXJSCoreBridge class];
    
    if (_jsBridge && [_jsBridge isKindOfClass:bridgeClass]) {
        return _jsBridge;
    }
    
    if (_jsBridge) {
        [_methodQueue removeAllObjects];
        _frameworkLoadFinished = NO;
    }
    
    // WXDebugger is a singleton actually and should not call its init twice.
    _jsBridge = _debugJS ? [NSClassFromString(@"WXDebugger") alloc] : [[WXJSCoreBridge alloc] init];
    
    [self registerGlobalFunctions];
    
    return _jsBridge;
}

- (NSInteger)checkInstance:(WXSDKInstance *)instance
{
    if (!instance) {
        WXLogInfo(@"instance not found, maybe already destroyed");
        return FALSE;
    }
    return TRUE;
}

- (void)registerGlobalFunctions
{
    __weak typeof(self) weakSelf = self;
    [_jsBridge registerCallNative:^NSInteger(NSString *instance, NSArray *tasks, NSString *callback) {
        return [weakSelf invokeNative:instance tasks:tasks callback:callback];
    }];
    
    [_jsBridge registerCallUpdateComponentData:^NSInteger(NSString *instanceId, NSString *componentId, NSString *jsonData) {
        if (_dataRenderHandler) {
            WXPerformBlockOnComponentThread(^{
                [_dataRenderHandler callUpdateComponentData:instanceId componentId:componentId jsonData:jsonData];
            });
        }
        else {
            WXSDKInstance *instance = [WXSDKManager instanceForID:instanceId];
            WXComponentManager *manager = instance.componentManager;
            if (manager.isValid) {
                WXSDKErrCode errorCode = WX_KEY_EXCEPTION_DEGRADE_EAGLE_RENDER_ERROR;
                NSError *error = [NSError errorWithDomain:WX_ERROR_DOMAIN code:errorCode userInfo:@{@"message":@"No data render handler found!"}];
                WXPerformBlockOnComponentThread(^{
                    [manager renderFailed:error];
                });
            }
        }
        return 0;
    }];

    [_jsBridge registerCallAddElement:^NSInteger(NSString *instanceId, NSString *parentRef, NSDictionary *elementData, NSInteger index) {
        
        WXPerformBlockOnComponentThread(^{
            [WXCoreBridge callAddElement:instanceId parentRef:parentRef data:elementData index:(int)index];
        });
        
        return 0;
    }];
    
    [_jsBridge registerCallCreateBody:^NSInteger(NSString *instanceId, NSDictionary *bodyData) {
        
        WXPerformBlockOnComponentThread(^{
            [WXCoreBridge callCreateBody:instanceId data:bodyData];
        });
        
        return 0;
    }];
    
    [_jsBridge registerCallRemoveElement:^NSInteger(NSString *instanceId, NSString *ref) {
        
        WXPerformBlockOnComponentThread(^{
            [WXCoreBridge callRemoveElement:instanceId ref:ref];
        });
        
        return 0;
    }];
    
    [_jsBridge registerCallMoveElement:^NSInteger(NSString *instanceId, NSString *ref, NSString *parentRef, NSInteger index) {
        
        WXPerformBlockOnComponentThread(^{
            [WXCoreBridge callMoveElement:instanceId ref:ref parentRef:parentRef index:(int)index];
        });
        
        return 0;
    }];
    
    [_jsBridge registerCallUpdateAttrs:^NSInteger(NSString *instanceId, NSString *ref, NSDictionary *attrsData) {
        
        WXPerformBlockOnComponentThread(^{
            [WXCoreBridge callUpdateAttrs:instanceId ref:ref data:attrsData];
        });
        
        return 0;
    }];
    
    [_jsBridge registerCallUpdateStyle:^NSInteger(NSString *instanceId, NSString *ref, NSDictionary *stylesData) {
        
        WXPerformBlockOnComponentThread(^{
            [WXCoreBridge callUpdateStyle:instanceId ref:ref data:stylesData];
        });

        return 0;
    }];
    
    [_jsBridge registerCallAddEvent:^NSInteger(NSString *instanceId, NSString *ref, NSString *event) {
        
        WXPerformBlockOnComponentThread(^{
            [WXCoreBridge callAddEvent:instanceId ref:ref event:event];
        });
        
        return 0;
    }];
    
    [_jsBridge registerCallRemoveEvent:^NSInteger(NSString *instanceId, NSString *ref, NSString *event) {
        
        WXPerformBlockOnComponentThread(^{
            [WXCoreBridge callRemoveEvent:instanceId ref:ref event:event];
        });
        
        return 0;
    }];
    
    [_jsBridge registerCallCreateFinish:^NSInteger(NSString *instanceId) {
        
        WXSDKInstance *instance = [WXSDKManager instanceForID:instanceId];
        [instance.apmInstance onStage:KEY_PAGE_STAGES_CREATE_FINISH];
        
        WXPerformBlockOnComponentThread(^{
            [WXCoreBridge callCreateFinish:instanceId];
        });
        
        return 0;
    }];
    
    if ([_jsBridge respondsToSelector:@selector(registerCallRefreshFinish:)]) {
        [_jsBridge registerCallRefreshFinish:^NSInteger(NSString *instanceId) {
            
            WXPerformBlockOnComponentThread(^{
                [WXCoreBridge callRefreshFinish:instanceId];
            });
            
            return 0;
        }];
    }
    
    if ([_jsBridge respondsToSelector:@selector(registerCallUpdateFinish:)]) {
        [_jsBridge registerCallUpdateFinish:^NSInteger(NSString *instanceId) {
            
            WXPerformBlockOnComponentThread(^{
                [WXCoreBridge callUpdateFinish:instanceId];
            });
            
            return 0;
        }];
    }
    
    [_jsBridge registerCallNativeModule:^NSInvocation*(NSString *instanceId, NSString *moduleName, NSString *methodName, NSArray *arguments, NSDictionary *options) {
        
        WXSDKInstance *instance = [WXSDKManager instanceForID:instanceId];
        
        if (!instance) {
            WXLogInfo(@"instance not found for callNativeModule:%@.%@, maybe already destroyed", moduleName, methodName);
            return nil;
        }
#ifdef DEBUG
        WXLogDebug(@"flexLayout -> action: callNativeModule : %@ . %@",moduleName,methodName);
#endif
        NSMutableDictionary * newOptions = options ? [options mutableCopy] : [NSMutableDictionary new];
        NSMutableArray * newArguments = [arguments mutableCopy];
        
        if ([instance.bundleType.lowercaseString isEqualToString:@"rax"]) {
            // we need to adjust __weex_options__ params in arguments to options compatible with rax javaScript framework.
            NSDictionary * weexOptions = nil;
            for(int i = 0;i < [arguments count]; i ++) {
                if ([arguments[i] isKindOfClass:[NSDictionary class]]) {
                    NSDictionary * dict = (NSDictionary*)arguments[i];
                    if (dict[@"__weex_options__"] && [dict[@"__weex_options__"] isKindOfClass:[NSDictionary class]]) {
                        weexOptions = dict;
                        [newOptions addEntriesFromDictionary:(NSDictionary*)(weexOptions[@"__weex_options__"])];
                    }
                }
            }
            if (weexOptions) {
                [newArguments removeObject:weexOptions];
            }
        }
        WXModuleMethod *method = [[WXModuleMethod alloc] initWithModuleName:moduleName methodName:methodName arguments:[newArguments copy] options:[newOptions copy] instance:instance];
        if(![moduleName isEqualToString:@"dom"] && instance.needPrerender){
            [WXPrerenderManager storePrerenderModuleTasks:method forUrl:instance.scriptURL.absoluteString];
            return nil;
        }
        return [method invoke];
    }];
    
    [_jsBridge registerCallNativeComponent:^void(NSString *instanceId, NSString *componentRef, NSString *methodName, NSArray *args, NSDictionary *options) {
       
#ifdef DEBUG
        WXLogDebug(@"flexLayout -> action: callNativeComponent ref:%@",componentRef);
#endif
        
        WXSDKInstance *instance = [WXSDKManager instanceForID:instanceId];
        WXComponentMethod *method = [[WXComponentMethod alloc] initWithComponentRef:componentRef methodName:methodName arguments:args instance:instance];
        [method invoke];
    }];
}

- (NSMutableArray *)insStack
{
    if (_insStack) return _insStack;

    _insStack = [NSMutableArray array];
    
    return _insStack;
}

- (WXSDKInstance *)topInstance
{
	WXSDKInstance *topInstance = nil;
	@synchronized(self) {
		if (self.insStack.count > 0) {
			topInstance = [WXSDKManager instanceForID:[self.insStack firstObject]];
		}
	}
    return topInstance;
}

- (NSMutableDictionary *)sendQueue
{
    WXAssertBridgeThread();
    
    if (_sendQueue) return _sendQueue;
    
    _sendQueue = [NSMutableDictionary dictionary];
    
    return _sendQueue;
}

#pragma mark JS Bridge Management

- (NSInteger)invokeNative:(NSString *)instanceId tasks:(NSArray *)tasks callback:(NSString __unused*)callback
{
    WXAssertBridgeThread();
    
    if (!instanceId || !tasks) {
        WX_MONITOR_FAIL(WXMTNativeRender, WX_ERR_JSFUNC_PARAM, @"JS call Native params error!");
        return 0;
    }

    WXSDKInstance *instance = [WXSDKManager instanceForID:instanceId];
    if (!instance) {
        WXLogInfo(@"instance already destroyed, task ignored");
        return -1;
    }
    
    NSTimeInterval startTime = CACurrentMediaTime()*1000;
    
    for (NSDictionary *task in tasks) {
        NSString *methodName = task[@"method"];
        NSArray *arguments = task[@"args"];
        if (task[@"component"]) {
            NSString *ref = task[@"ref"];
            WXComponentMethod *method = [[WXComponentMethod alloc] initWithComponentRef:ref methodName:methodName arguments:arguments instance:instance];
            [method invoke];
        } else {
            NSString *moduleName = task[@"module"];
            NSDictionary *options = task[@"options"];
            WXModuleMethod *method = [[WXModuleMethod alloc] initWithModuleName:moduleName methodName:methodName arguments:arguments options:options instance:instance];
            [method invoke];
        }
    }
    
    [self performSelector:@selector(_sendQueueLoop) withObject:nil];
    if (!instance.isJSCreateFinish) {
        NSTimeInterval diff = CACurrentMediaTime()*1000-startTime;
        instance.performance.fsCallNativeNum++;
        instance.performance.fsCallNativeTime =  instance.performance.fsCallNativeTime + diff;
    }
    if (instance && !instance.apmInstance.isFSEnd) {
        NSTimeInterval diff = CACurrentMediaTime()*1000 - startTime;
        [instance.apmInstance updateFSDiffStats:KEY_PAGE_STATS_FS_CALL_NATIVE_NUM withDiffValue:1];
        [instance.apmInstance updateFSDiffStats:KEY_PAGE_STATS_FS_CALL_NATIVE_TIME withDiffValue:diff];
    }
    return 1;
}

- (void)createInstance:(NSString *)instanceIdString
              template:(NSString *)jsBundleString
               options:(NSDictionary *)options
                  data:(id)data
{
    WXAssertBridgeThread();
    WXAssertParam(instanceIdString);
	
	@synchronized(self) {
		if (![self.insStack containsObject:instanceIdString]) {
			if ([options[@"RENDER_IN_ORDER"] boolValue]) {
				[self.insStack addObject:instanceIdString];
			} else {
				[self.insStack insertObject:instanceIdString atIndex:0];
			}
		}
	}
    WXSDKInstance *sdkInstance = [WXSDKManager instanceForID:instanceIdString];
    if (!sdkInstance) {
        return;
    }
    [sdkInstance.apmInstance onStage:KEY_PAGE_STAGES_LOAD_BUNDLE_START];
    
    //create a sendQueue bind to the current instance
    NSMutableArray *sendQueue = [NSMutableArray array];
    [self.sendQueue setValue:sendQueue forKey:instanceIdString];

    if (sdkInstance.dataRender && ![options[@"EXEC_JS"] boolValue]) {
        WX_MONITOR_INSTANCE_PERF_START(WXFirstScreenJSFExecuteTime, [WXSDKManager instanceForID:instanceIdString]);
        WX_MONITOR_INSTANCE_PERF_START(WXPTJSCreateInstance, [WXSDKManager instanceForID:instanceIdString]);
        if (_dataRenderHandler) {
            WXPerformBlockOnComponentThread(^{
                [_dataRenderHandler createPage:instanceIdString template:jsBundleString options:options data:data];
            });
        }
        else {
            WXComponentManager *manager = sdkInstance.componentManager;
            if (manager.isValid) {
                WXSDKErrCode errorCode = WX_KEY_EXCEPTION_DEGRADE_EAGLE_RENDER_ERROR;
                NSError *error = [NSError errorWithDomain:WX_ERROR_DOMAIN code:errorCode userInfo:@{@"message":@"No data render handler found!"}];
                WXPerformBlockOnComponentThread(^{
                    [manager renderFailed:error];
                });
            }
        }
        WX_MONITOR_INSTANCE_PERF_END(WXPTJSCreateInstance, [WXSDKManager instanceForID:instanceIdString]);
        [sdkInstance.apmInstance onStage:KEY_PAGE_STAGES_LOAD_BUNDLE_END];
        return;
    }

    NSArray *args = nil;
    WX_MONITOR_INSTANCE_PERF_START(WXFirstScreenJSFExecuteTime, [WXSDKManager instanceForID:instanceIdString]);
    WX_MONITOR_INSTANCE_PERF_START(WXPTJSCreateInstance, [WXSDKManager instanceForID:instanceIdString]);

    NSString * bundleType = [self _pareJSBundleType:instanceIdString jsBundleString:jsBundleString]; // bundleType can be Vue, Rax and the new framework.
    if (bundleType) {
        [sdkInstance.apmInstance setProperty:KEY_PAGE_PROPERTIES_BUNDLE_TYPE withValue:bundleType];
        NSMutableDictionary *newOptions = [options mutableCopy];
        if (!options) {
            newOptions = [NSMutableDictionary new];
        }
        [newOptions addEntriesFromDictionary:@{@"env":[WXUtility getEnvironment]}];
        newOptions[@"bundleType"] = bundleType;
        __block NSString *raxAPIScript = nil;
        __block NSString *raxAPIScriptPath = nil;
        id<WXJSFrameworkLoadProtocol> handler = [WXHandlerFactory handlerForProtocol:@protocol(WXJSFrameworkLoadProtocol)];
        sdkInstance.bundleType = bundleType;
        if ([bundleType.lowercaseString isEqualToString:@"rax"]) {
            if (handler && [handler respondsToSelector:@selector(loadRaxFramework:)]) {
                [handler loadRaxFramework:^(NSString *path, NSString *script) {
                    raxAPIScriptPath = path;
                    raxAPIScript = script;
                }];
            }
        }
        if ([WXDebugTool isDevToolDebug]) {
            [self callJSMethod:@"createInstanceContext" args:@[instanceIdString, newOptions, data?:@[],raxAPIScript?:@""]];
            
            if ([NSURL URLWithString:sdkInstance.pageName]) {
                [sdkInstance.instanceJavaScriptContext executeJavascript:jsBundleString withSourceURL:[NSURL URLWithString:sdkInstance.pageName]];
            } else {
                [sdkInstance.instanceJavaScriptContext executeJavascript:jsBundleString];
            }
            WX_MONITOR_INSTANCE_PERF_END(WXPTJSCreateInstance, [WXSDKManager instanceForID:instanceIdString]);
            [sdkInstance.apmInstance onStage:KEY_PAGE_STAGES_LOAD_BUNDLE_END];
        } else {
            sdkInstance.callCreateInstanceContext = [NSString stringWithFormat:@"instanceId:%@\noptions:%@\ndata:%@", instanceIdString, newOptions, data];
            //add instanceId to weexContext ,if fucn createInstanceContext failure ，then we will know which instance has problem (exceptionhandler)
            self.jsBridge.javaScriptContext[@"wxExtFuncInfo"]= @{
                                                                 @"func":@"createInstanceContext",
                                                                 @"arg":@"start",
                                                                 @"instanceId":sdkInstance.instanceId?:@"unknownId"
                                                                };
            __weak typeof(self) weakSelf = self;
            [self callJSMethod:@"createInstanceContext" args:@[instanceIdString, newOptions, data?:@[]] onContext:nil completion:^(JSValue *instanceContextEnvironment) {
                if (sdkInstance.pageName) {
                    if (@available(iOS 8.0, *)) {
                        [sdkInstance.instanceJavaScriptContext.javaScriptContext setName:sdkInstance.pageName];
                    } else {
                        // Fallback
                    }
                }
                weakSelf.jsBridge.javaScriptContext[@"wxExtFuncInfo"]= nil;
                
                NSArray* allKeys = [WXUtility extractPropertyNamesOfJSValueObject:instanceContextEnvironment];
                sdkInstance.createInstanceContextResult = [NSString stringWithFormat:@"%@", allKeys];
                JSGlobalContextRef instanceContextRef = sdkInstance.instanceJavaScriptContext.javaScriptContext.JSGlobalContextRef;
                JSObjectRef instanceGlobalObject = JSContextGetGlobalObject(instanceContextRef);
                
                for (NSString * key in allKeys) {
                    JSStringRef propertyName = JSStringCreateWithUTF8CString([key cStringUsingEncoding:NSUTF8StringEncoding]);
                    if ([key isEqualToString:@"Vue"]) {
                        JSObjectSetPrototype(instanceContextRef, JSValueToObject(instanceContextRef, [instanceContextEnvironment valueForProperty:key].JSValueRef, NULL), JSObjectGetPrototype(instanceContextRef, instanceGlobalObject));
                    }
                    JSObjectSetProperty(instanceContextRef, instanceGlobalObject, propertyName, [instanceContextEnvironment valueForProperty:key].JSValueRef, 0, NULL);
                }
                
                if (WX_SYS_VERSION_LESS_THAN(@"10.2")) {
                    if (handler && [handler respondsToSelector:@selector(loadPolyfillFramework:)]) {
                        [handler loadPolyfillFramework:^(NSString *path, NSString *script) {
                            if (script) {
                                [sdkInstance.instanceJavaScriptContext executeJavascript:script withSourceURL:[NSURL URLWithString:path]];
                            } else {
                                WXLogError(@"weex-pollyfill can not found");
                            }
                        }];
                    }
                }
                
                if (raxAPIScript) {
                    [sdkInstance.instanceJavaScriptContext executeJavascript:raxAPIScript withSourceURL:[NSURL URLWithString:raxAPIScriptPath]];
                    NSArray* allKeys = [WXUtility extractPropertyNamesOfJSValueObject:sdkInstance.instanceJavaScriptContext.javaScriptContext.globalObject];
                    sdkInstance.executeRaxApiResult = [NSString stringWithFormat:@"%@", allKeys];
                }
                
                [sdkInstance.apmInstance onStage:KEY_PAGE_STAGES_LOAD_BUNDLE_END];
                NSDictionary* funcInfo = @{
                                           @"func":@"createInstance",
                                           @"arg":@"start",
                                           @"instanceId":sdkInstance.instanceId?:@"unknownId"
                                        };
                sdkInstance.instanceJavaScriptContext.javaScriptContext[@"wxExtFuncInfo"]= funcInfo;
                if ([NSURL URLWithString:sdkInstance.pageName] || sdkInstance.scriptURL) {
                    [sdkInstance.instanceJavaScriptContext executeJavascript:jsBundleString withSourceURL:[NSURL URLWithString:sdkInstance.pageName]?:sdkInstance.scriptURL];
                } else {
                    [sdkInstance.instanceJavaScriptContext executeJavascript:jsBundleString];
                }
                sdkInstance.instanceJavaScriptContext.javaScriptContext[@"wxExtFuncInfo"] = nil;
                WX_MONITOR_INSTANCE_PERF_END(WXPTJSCreateInstance, [WXSDKManager instanceForID:instanceIdString]);
            }];
        }
        
    } else {
        [sdkInstance.apmInstance setProperty:KEY_PAGE_PROPERTIES_BUNDLE_TYPE withValue:@"other"];
        [sdkInstance.apmInstance onStage:KEY_PAGE_STAGES_LOAD_BUNDLE_END];
        if (data){
            args = @[instanceIdString, jsBundleString, options ?: @{}, data];
        } else {
            args = @[instanceIdString, jsBundleString, options ?: @{}];
        }
        NSDictionary* funcInfo = @{
                                   @"func":@"createInstance",
                                   @"arg":@"start",
                                   @"instanceId":sdkInstance.instanceId?:@"unknownId"
                                   };
        sdkInstance.instanceJavaScriptContext.javaScriptContext[@"wxExtFuncInfo"] = funcInfo;
        [self callJSMethod:@"createInstance" args:args];
        sdkInstance.instanceJavaScriptContext.javaScriptContext[@"wxExtFuncInfo"] = nil;
        
        [WXExceptionUtils commitCriticalExceptionRT:instanceIdString errCode:[NSString stringWithFormat:@"%d", WX_KEY_EXCEPTION_NO_BUNDLE_TYPE] function:@"createInstance" exception:@"Fatal Error : No bundle type in js bundle head, cause white screen or memory leak!!" extParams:nil];

        WX_MONITOR_INSTANCE_PERF_END(WXPTJSCreateInstance, [WXSDKManager instanceForID:instanceIdString]);
    }
}

- (BOOL) _shouldMountExtInfoToInstanceContxt
{
    id configCenter = [WXSDKEngine handlerForProtocol:@protocol(WXConfigCenterProtocol)];
    BOOL shouldMountInstanceContextExtInfo = YES;
    if ([configCenter respondsToSelector:@selector(configForKey:defaultValue:isDefault:)]) {
        shouldMountInstanceContextExtInfo = [[configCenter configForKey:@"iOS_weex_ext_config.shouldMountInstanceContextExtInfo" defaultValue:@YES isDefault:NULL] boolValue];
    }
    return shouldMountInstanceContextExtInfo;
}

- (void)createInstance:(NSString *)instanceIdString
              contents:(NSData *)contents
               options:(NSDictionary *)options
                  data:(id)data
{
    WXAssertBridgeThread();
    WXAssertParam(instanceIdString);

    @synchronized(self) {
        if (![self.insStack containsObject:instanceIdString]) {
            if ([options[@"RENDER_IN_ORDER"] boolValue]) {
                [self.insStack addObject:instanceIdString];
            } else {
                [self.insStack insertObject:instanceIdString atIndex:0];
            }
        }
    }
    WXSDKInstance *sdkInstance = [WXSDKManager instanceForID:instanceIdString];
    [sdkInstance.apmInstance onStage:KEY_PAGE_STAGES_LOAD_BUNDLE_START];

    //create a sendQueue bind to the current instance
    NSMutableArray *sendQueue = [NSMutableArray array];
    [self.sendQueue setValue:sendQueue forKey:instanceIdString];

    if (sdkInstance.dataRender) {
        WX_MONITOR_INSTANCE_PERF_START(WXFirstScreenJSFExecuteTime, [WXSDKManager instanceForID:instanceIdString]);
        WX_MONITOR_INSTANCE_PERF_START(WXPTJSCreateInstance, [WXSDKManager instanceForID:instanceIdString]);
        if (_dataRenderHandler) {
            WXPerformBlockOnComponentThread(^{
                [_dataRenderHandler createPage:instanceIdString contents:contents options:options data:data];
            });
        }
        else {
            WXComponentManager *manager = sdkInstance.componentManager;
            if (manager.isValid) {
                WXSDKErrCode errorCode = WX_KEY_EXCEPTION_DEGRADE_EAGLE_RENDER_ERROR;
                NSError *error = [NSError errorWithDomain:WX_ERROR_DOMAIN code:errorCode userInfo:@{@"message":@"No data render handler found!"}];
                WXPerformBlockOnComponentThread(^{
                    [manager renderFailed:error];
                });
            }
        }
        WX_MONITOR_INSTANCE_PERF_END(WXPTJSCreateInstance, [WXSDKManager instanceForID:instanceIdString]);
        [sdkInstance.apmInstance onStage:KEY_PAGE_STAGES_LOAD_BUNDLE_END];
        return;
    }
}

- (NSString *)_pareJSBundleType:(NSString*)instanceIdString jsBundleString:(NSString*)jsBundleString
{
    NSString * bundleType = nil;
    WXSDKInstance * instance = [WXSDKManager instanceForID:instanceIdString];
    NSURLComponents * urlComponent = [NSURLComponents componentsWithString:instance.pageName?:@""];
    if (@available(iOS 8.0, *)) {
        for (NSURLQueryItem * queryItem in urlComponent.queryItems) {
            if ([queryItem.name isEqualToString:@"bundleType"] && [@[@"vue", @"rax"] containsObject:queryItem.value]) {
                bundleType = queryItem.value;
                return bundleType;
            }
        }
    } else {
        // Fallback on earlier versions
        return bundleType;
    }
    
    // find first character that is not space or new line character
    NSCharacterSet* voidCharSet = [NSCharacterSet whitespaceAndNewlineCharacterSet];
    NSUInteger length = [jsBundleString length];
    NSUInteger validCharacter = 0;
    while (validCharacter < length && [voidCharSet characterIsMember:[jsBundleString characterAtIndex:validCharacter]]) {
        validCharacter ++;
    }
    if (validCharacter >= length) {
        return bundleType;
    }
    @try {
        jsBundleString = [jsBundleString substringWithRange:NSMakeRange(validCharacter, MIN(100, length - validCharacter))];
    }
    @catch (NSException* e) {
    }
    if ([jsBundleString length] == 0) {
        return bundleType;
    }
    
    static NSRegularExpression* headerExp = nil;
    static NSRegularExpression* vueExp = nil;
    static NSRegularExpression* raxExp = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        headerExp = [NSRegularExpression regularExpressionWithPattern:@"^\\s*\\/\\/ *(\\{[^}]*\\}) *\\r?\\n" options:NSRegularExpressionCaseInsensitive error:NULL];
        vueExp = [NSRegularExpression regularExpressionWithPattern:@"(use)(\\s+)(weex:vue)" options:NSRegularExpressionCaseInsensitive error:NULL];
        raxExp = [NSRegularExpression regularExpressionWithPattern:@"(use)(\\s+)(weex:rax)" options:NSRegularExpressionCaseInsensitive error:NULL];
    });
    
    if ( [self _isParserByRegEx]) {
        NSTextCheckingResult *match = [headerExp firstMatchInString:jsBundleString options:0 range:NSMakeRange(0, jsBundleString.length)];
        if (match) {
            NSString* bundleTypeStr = [jsBundleString substringWithRange:match.range];
            bundleTypeStr = [bundleTypeStr stringByReplacingOccurrencesOfString:@"//" withString:@""];
            id vale = [WXUtility objectFromJSON:bundleTypeStr];
            bundleType = [vale objectForKey:@"framework"];
        }else{
            NSTextCheckingResult *match = [vueExp firstMatchInString:jsBundleString options:0 range:NSMakeRange(0, jsBundleString.length)];
            if (match) {
                bundleType = [jsBundleString substringWithRange:match.range];
                return bundleType;
            }
            match = [raxExp firstMatchInString:jsBundleString options:0 range:NSMakeRange(0, jsBundleString.length)];
            if (match) {
                bundleType = [jsBundleString substringWithRange:match.range];
            }
        }
    }else{
        if ([jsBundleString hasPrefix:@"// { \"framework\": \"Vue\""] || [jsBundleString hasPrefix:@"// { \"framework\": \"vue\""]) {
            bundleType = @"Vue";
        } else if ([jsBundleString hasPrefix:@"// { \"framework\": \"Rax\""] || [jsBundleString hasPrefix:@"// { \"framework\": \"rax\""] || [jsBundleString hasPrefix:@"// {\"framework\" : \"Rax\"}"] || [jsBundleString hasPrefix:@"// {\"framework\" : \"rax\"}"]) {
            bundleType = @"Rax";
        }else {
            NSTextCheckingResult *match = [vueExp firstMatchInString:jsBundleString options:0 range:NSMakeRange(0, jsBundleString.length)];
            if (match) {
                bundleType = [jsBundleString substringWithRange:match.range];
                return bundleType;
            }
            match = [raxExp firstMatchInString:jsBundleString options:0 range:NSMakeRange(0, jsBundleString.length)];
            if (match) {
                bundleType = [jsBundleString substringWithRange:match.range];
            }
        }
    }
    return bundleType;
}

- (bool)_isParserByRegEx
{
    return false;
//    bool useRegEx = true;
//    id<WXConfigCenterProtocol> configCenter = [WXSDKEngine handlerForProtocol:@protocol(WXConfigCenterProtocol)];
//
//    if ([configCenter respondsToSelector:@selector(configForKey:defaultValue:isDefault:)]) {
//        useRegEx = [[configCenter configForKey:@"iOS_weex_ext_config.parserTypeByRegEx" defaultValue:@(YES) isDefault:NULL] boolValue];
//    }
//    return useRegEx;
}

- (void)destroyInstance:(NSString *)instance
{
    WXAssertBridgeThread();
    WXAssertParam(instance);
    
    //remove this instance exception history
    [WXExceptionUtils removeExceptionHistory:instance];

    //remove instance from stack
	@synchronized(self) {
		[self.insStack removeObject:instance];
	}
    
    if(_jsBridge && [_jsBridge respondsToSelector:@selector(removeTimers:)]){
        [_jsBridge removeTimers:instance];
    }

    if(self.sendQueue[instance]){
        [self.sendQueue removeObjectForKey:instance];
    }
    
    WXSDKInstance *sdkInstance = [WXSDKManager instanceForID:instance];
    if (sdkInstance.dataRender) {
        if (_dataRenderHandler) {
            WXPerformBlockOnComponentThread(^{
                [_dataRenderHandler destroyDataRenderInstance:instance];
                WXPerformBlockOnBridgeThread(^{
                    [self callJSMethod:@"destroyInstance" args:@[instance]];
                });
            });
        }
        else {
            WXComponentManager *manager = sdkInstance.componentManager;
            if (manager.isValid) {
                WXSDKErrCode errorCode = WX_KEY_EXCEPTION_DEGRADE_EAGLE_RENDER_ERROR;
                NSError *error = [NSError errorWithDomain:WX_ERROR_DOMAIN code:errorCode userInfo:@{@"message":@"No data render handler found!"}];
                WXPerformBlockOnComponentThread(^{
                    [manager renderFailed:error];
                });
            }
        }
    } else {
        [self callJSMethod:@"destroyInstance" args:@[instance]];
    }
}

- (void)forceGarbageCollection
{
    if ([self.jsBridge respondsToSelector:@selector(garbageCollect)]) {
        [self.jsBridge garbageCollect];
    }
}

- (void)refreshInstance:(NSString *)instance
                   data:(id)data
{
    WXAssertBridgeThread();
    WXAssertParam(instance);
    
    if (!data) return;
    
    WXSDKInstance *sdkInstance = [WXSDKManager instanceForID:instance];
    if (sdkInstance.dataRender) {
        if (!_dataRenderHandler) {
            WXComponentManager *manager = sdkInstance.componentManager;
            if (manager.isValid) {
                WXSDKErrCode errorCode = WX_KEY_EXCEPTION_DEGRADE_EAGLE_RENDER_ERROR;
                NSError *error = [NSError errorWithDomain:WX_ERROR_DOMAIN code:errorCode userInfo:@{@"message":@"No data render handler found!"}];
                WXPerformBlockOnComponentThread(^{
                    [manager renderFailed:error];
                });
            }
            return;
        }
        WXPerformBlockOnComponentThread(^{
            if ([data isKindOfClass:[NSDictionary class]]) {
                [_dataRenderHandler refreshDataRenderInstance:instance data:[WXUtility JSONString:data]];
            } else if ([data isKindOfClass:[NSString class]]) {
                [_dataRenderHandler refreshDataRenderInstance:instance data:data];
            }
        });
        [[WXSDKManager bridgeMgr] callJSMethod:@"callJS" args:@[instance, @[@{@"method":@"fireEvent", @"args":@[@"", @"refresh", [WXUtility objectFromJSON:@"[]"], @"", @{@"params":@[@{@"data":data}]}]}]]];
    } else {
        [self callJSMethod:@"refreshInstance" args:@[instance, data]];
    }
}

- (void)updateState:(NSString *)instance data:(id)data
{
    WXAssertBridgeThread();
    WXAssertParam(instance);
    
    //[self.jsBridge callJSMethod:@"updateState" args:@[instance, data]];
}

- (void)executeJsFramework:(NSString *)script
{
    WXAssertBridgeThread();
    WXAssertParam(script);
    
    WX_MONITOR_PERF_START(WXPTFrameworkExecute);

    [self.jsBridge executeJSFramework:script];
    
    WX_MONITOR_PERF_END(WXPTFrameworkExecute);
    
    if ([self.jsBridge exception]) {
        NSString *exception = [[self.jsBridge exception] toString];
        NSMutableString *errMsg = [NSMutableString stringWithFormat:@"[WX_KEY_EXCEPTION_SDK_INIT_JSFM_INIT_FAILED] %@",exception];
        [WXExceptionUtils commitCriticalExceptionRT:@"WX_KEY_EXCEPTION_SDK_INIT" errCode:[NSString stringWithFormat:@"%d", WX_KEY_EXCEPTION_SDK_INIT] function:@"" exception:errMsg extParams:nil];
    } else {
        WX_MONITOR_SUCCESS(WXMTJSFramework);
        //the JSFramework has been load successfully.
        self.frameworkLoadFinished = YES;
        
        [self executeAllJsService];
        
        JSValue *frameworkVersion = [self.jsBridge callJSMethod:@"getJSFMVersion" args:nil];
        if (frameworkVersion && [frameworkVersion isString]) {
            [WXAppConfiguration setJSFrameworkVersion:[frameworkVersion toString]];
        }
        
        if (script) {
             [WXAppConfiguration setJSFrameworkLibSize:[script lengthOfBytesUsingEncoding:NSUTF8StringEncoding]];
        }
        
        //execute methods which has been stored in methodQueue temporarily.
        for (NSDictionary *method in _methodQueue) {
            [self callJSMethod:method[@"method"] args:method[@"args"]];
        }
        
        [_methodQueue removeAllObjects];
        
        WX_MONITOR_PERF_END(WXPTInitalize);
    };
}

- (void)executeJsMethod:(WXCallJSMethod *)method
{
    WXAssertBridgeThread();
    
    if (!method.instance) {
        WXLogError(@"Instance doesn't exist!");
        return;
    }
    
    NSMutableArray *sendQueue = self.sendQueue[method.instance.instanceId];
    if (!sendQueue) {
        WXLogInfo(@"No send queue for instance:%@, may it has been destroyed so method:%@ is ignored", method.instance, method.methodName);
        return;
    }
    
    [sendQueue addObject:method];
    [self performSelector:@selector(_sendQueueLoop) withObject:nil];
}

- (void)callJSMethod:(NSString *)method args:(NSArray *)args onContext:(id<WXBridgeProtocol>)bridge completion:(void (^)(JSValue * value))completion
{
    NSMutableArray *newArg = nil;
    if (!bridge) {
        bridge = self.jsBridge;
    }
    if (self.frameworkLoadFinished) {
        newArg = [args mutableCopy];
        if ([newArg containsObject:completion]) {
            [newArg removeObject:completion];
        }
        WXLogDebug(@"Calling JS... method:%@, args:%@", method, args);
        if (([bridge isKindOfClass:[WXJSCoreBridge class]]) ||
            ([bridge isKindOfClass:NSClassFromString(@"WXDebugger") ]) ) {
            JSValue *value = [bridge callJSMethod:method args:args];
            if (completion) {
                completion(value);
            }
        } else {
            [bridge callJSMethod:method args:args];
        }
    } else {
        newArg = [args mutableCopy];
        if (completion) {
            [newArg addObject:completion];
        }
        [_methodQueue addObject:@{@"method":method, @"args":[newArg copy]}];
    }
}

- (JSValue *)excuteJSMethodWithResult:(WXCallJSMethod *)method
{
    WXAssertBridgeThread();
    return  [self.jsBridge callJSMethod:@"callJS" args:@[method.instance.instanceId, @[[method callJSTask]]]];
}

- (void)executeAllJsService
{
    for(NSDictionary *service in _jsServiceQueue) {
        NSString *script = [service valueForKey:@"script"];
        NSString *name = [service valueForKey:@"name"];
        [self executeJsService:script withName:name];
    }
    
    [_jsServiceQueue removeAllObjects];
}

- (void)executeJsService:(NSString *)script withName:(NSString *)name
{
    if(self.frameworkLoadFinished) {
        WXAssert(script, @"param script required!");
        if ([self.jsBridge respondsToSelector:@selector(javaScriptContext)]) {
            NSDictionary* funcInfo = @{
                                       @"func":@"executeJsService",
                                       @"arg":name?:@"unsetScriptName"
                                       };
            self.jsBridge.javaScriptContext[@"wxExtFuncInfo"] = funcInfo;
        }
        [self.jsBridge executeJavascript:script];
        if ([self.jsBridge respondsToSelector:@selector(javaScriptContext)]) {
            self.jsBridge.javaScriptContext[@"wxExtFuncInfo"] = nil;
        }
        
        if ([self.jsBridge exception]) {
            NSString *exception = [[self.jsBridge exception] toString];
            NSMutableString *errMsg = [NSMutableString stringWithFormat:@"[WX_KEY_EXCEPTION_INVOKE_JSSERVICE_EXECUTE] name:%@,arg:%@,exception :$@",name,exception];
            [WXExceptionUtils commitCriticalExceptionRT:@"WX_KEY_EXCEPTION_INVOKE" errCode:[NSString stringWithFormat:@"%d", WX_KEY_EXCEPTION_INVOKE] function:@"executeJsService" exception:errMsg extParams:nil];
        } else {
            // success
        }
    }else {
        [_jsServiceQueue addObject:@{
                                     @"name": name,
                                     @"script": script
                                     }];
    }
}
    
- (void)registerModules:(NSDictionary *)modules
{
    WXAssertBridgeThread();
    
    if(!modules) return;
    
    [self callJSMethod:@"registerModules" args:@[modules]];
    if (_dataRenderHandler) {
        WXPerformBlockOnComponentThread(^{
            [_dataRenderHandler registerModules:modules];
        });
    }
}

- (void)registerComponents:(NSArray *)components
{
    WXAssertBridgeThread();
    
    if(!components) return;
    
    [self callJSMethod:@"registerComponents" args:@[components]];
    if (_dataRenderHandler) {
        WXPerformBlockOnComponentThread(^{
            [_dataRenderHandler registerComponents:components];
        });
    }
}

- (void)callJSMethod:(NSString *)method args:(NSArray *)args
{
    if (self.frameworkLoadFinished) {
        [self.jsBridge callJSMethod:method args:args];
    }
    else {
        [_methodQueue addObject:@{@"method":method, @"args":args}];
    }
}

- (void)resetEnvironment
{
    [_jsBridge resetEnvironment];
}

#pragma mark JS Debug Management

- (void)connectToDevToolWithUrl:(NSURL *)url
{
    id webSocketBridge = [NSClassFromString(@"WXDebugger") alloc];
    if(!webSocketBridge || ![webSocketBridge respondsToSelector:NSSelectorFromString(@"connectToURL:")]) {
        return;
    } else {
        SuppressPerformSelectorLeakWarning(
           [webSocketBridge performSelector:NSSelectorFromString(@"connectToURL:") withObject:url]
        );
    }
}

- (void)connectToWebSocket:(NSURL *)url
{
    if (NSClassFromString(@"WXDebugLoggerBridge")) {
        _devToolSocketBridge = [[NSClassFromString(@"WXDebugLoggerBridge") alloc] initWithURL:url];
    }
}

- (void)logToWebSocket:(NSString *)flag message:(NSString *)message
{
    [_devToolSocketBridge callJSMethod:@"__logger" args:@[flag, message]];
}

#pragma mark Private Mehtods

- (void)_sendQueueLoop
{
    WXAssertBridgeThread();
    
    BOOL hasTask = NO;
    NSMutableArray *tasks = [NSMutableArray array];
    NSString *execIns = nil;
	
	@synchronized(self) {
		for (NSString *instance in self.insStack) {
			NSMutableArray *sendQueue = self.sendQueue[instance];
			if(sendQueue.count > 0){
				hasTask = YES;
				for(WXCallJSMethod *method in sendQueue){
					[tasks addObject:[method callJSTask]];
				}
				[sendQueue removeAllObjects];
				execIns = instance;
				break;
			}
		}
	}
    
    if ([tasks count] > 0 && execIns) {
        WXSDKInstance * execInstance = [WXSDKManager instanceForID:execIns];
        NSTimeInterval start = CACurrentMediaTime()*1000;
        
        if (execInstance.instanceJavaScriptContext && execInstance.bundleType) {
            [self callJSMethod:@"__WEEX_CALL_JAVASCRIPT__" args:@[execIns, tasks] onContext:execInstance.instanceJavaScriptContext completion:nil];
        } else {
            [self callJSMethod:@"callJS" args:@[execIns, tasks]];
        }
        if (execInstance && !(execInstance.isJSCreateFinish)) {
            NSTimeInterval diff = CACurrentMediaTime()*1000 - start;
            execInstance.performance.fsCallJsNum++;
            execInstance.performance.fsCallJsTime =  execInstance.performance.fsCallJsTime+ diff;
         }
        if (execInstance && !execInstance.apmInstance.isFSEnd) {
             NSTimeInterval diff = CACurrentMediaTime()*1000 - start;
            [execInstance.apmInstance updateFSDiffStats:KEY_PAGE_STATS_FS_CALL_JS_NUM withDiffValue:1];
            [execInstance.apmInstance updateFSDiffStats:KEY_PAGE_STATS_FS_CALL_JS_TIME withDiffValue:diff];
        }
    }
    
    if (hasTask) {
        [self performSelector:@selector(_sendQueueLoop) withObject:nil];
    }
}

+ (void)mountContextEnvironment:(JSContext*)context
{
    NSDictionary *data = [WXUtility getEnvironment];
    context[@"WXEnvironment"] = data;
    // use latin1 encode also named ISO/IEC_8859-1 more: https://en.wikipedia.org/wiki/ISO/IEC_8859-1
    // 8-bit single-byte coded graphic character sets as webkit does
    // https://github.com/WebKit/webkit/blob/master/Source/WebCore/page/Base64Utilities.cpp
    context[@"btoa"] = ^(JSValue *value ) {
        NSData *nsdata = [[value toString]
                          dataUsingEncoding:NSISOLatin1StringEncoding];
        NSString *base64Encoded = [nsdata base64EncodedStringWithOptions:0];
        return base64Encoded;
    };
    context[@"atob"] = ^(JSValue *value ) {
        NSData *nsdataFromBase64String = [[NSData alloc]
                                          initWithBase64EncodedString:[value toString] options:0];
        NSString *base64Decoded = [[NSString alloc]
                                   initWithData:nsdataFromBase64String encoding:NSISOLatin1StringEncoding];
        return base64Decoded;
    };
    
    // Avoid exceptionHandler be recursively invoked and finally cause stack overflow.
    static BOOL gInExceptionHandler = NO;
    context.exceptionHandler = ^(JSContext *context, JSValue *exception){
        if (gInExceptionHandler) {
            return;
        }
        gInExceptionHandler = YES;
        
        @try {
            BOOL tryFindInstanceInfoInGlobalContext = NO;
            context.exception = exception;
            NSString *errorCode = [NSString stringWithFormat:@"%d", WX_KEY_EXCEPTION_WXBRIDGE];;
            NSString *bundleUrl = nil;
            NSString *message = nil;
            NSDictionary *userInfo = nil;
            BOOL commitException = YES;
            WXSDKInstance * instance = nil;

            if (context.instanceId) {
                // instance page javaScript runtime exception
                 instance = [WXSDKManager instanceForID:context.instanceId];
                if (instance) {
                    // instance already existed
                    commitException = YES;
                } else {
                    // instance already destroyed
                    commitException = NO;
                }
            } else {
                // weex-main-jsfm.js runtime exception throws
                message = [NSString stringWithFormat:@"[WX_KEY_EXCEPTION_WXBRIDGE] [%@:%@:%@] %@ js stack: %@", exception[@"sourceURL"], exception[@"line"], exception[@"column"], [exception toString], [exception[@"stack"] toObject]];
                if (!JSValueIsUndefined(context.JSGlobalContextRef, exception[@"sourceURL"].JSValueRef)) {
                    bundleUrl = exception[@"sourceURL"].toString;
                } else {
                    bundleUrl = @"weex-main-jsfm";
                }
                userInfo = [NSDictionary dictionary];
            }
            
            NSDictionary* wxExtFuncInfo = [context[@"wxExtFuncInfo"] toDictionary];
            NSString* recordFunc = [wxExtFuncInfo objectForKey:@"func"];
            NSString* recordArg = [wxExtFuncInfo objectForKey:@"arg"];
            NSString* recordInsstanceId = [wxExtFuncInfo objectForKey:@"instanceId"];
            if (nil == instance) {
                instance = [WXSDKManager instanceForID:recordInsstanceId];
                tryFindInstanceInfoInGlobalContext = nil!= instance;
            }
            
            if(nil != instance && [recordFunc isEqualToString:@"createInstance"] && !instance.apmInstance.hasAddView){
                errorCode = [NSString stringWithFormat:@"%d", WX_KEY_EXCEPTION_EMPTY_SCREEN_JS];
            }

            if (instance) {
                bundleUrl = instance.pageName?:([instance.scriptURL absoluteString]?:@"WX_KEY_EXCEPTION_WXBRIDGE");
                message = [NSString stringWithFormat:@"[WX_KEY_EXCEPTION_WXBRIDGE] exception: %@\n stack:%@",[exception toString],[exception[@"stack"] toObject]];
                userInfo = @{@"jsMainBundleStringContentLength":instance.userInfo[@"jsMainBundleStringContentLength"]?:@"",
                             @"jsMainBundleStringContentMd5":instance.userInfo[@"jsMainBundleStringContentMd5"]?:@"",
                             @"sourceURL":[NSString stringWithFormat:@"%@:%@:%@",exception[@"sourceURL"],exception[@"line"],exception[@"column"]],
                             @"callCreateInstanceContext":instance.callCreateInstanceContext?:@"",
                             @"createInstanceContextResult": instance.createInstanceContextResult?:@"",
                             @"executeRaxApiResult":instance.executeRaxApiResult?:@""
                             };
            }
            
            if (commitException) {
                NSMutableDictionary* reportInfo = [[NSMutableDictionary alloc] initWithDictionary:[userInfo mutableCopy]];
                [reportInfo setObject:context.name?:@"unknownContextName" forKey:@"wxContextName"];
                
                WXJSExceptionInfo * jsExceptionInfo = [[WXJSExceptionInfo alloc] initWithInstanceId:instance.instanceId bundleUrl:bundleUrl errorCode:errorCode functionName:[NSString stringWithFormat:@"func: %@ arg:%@",recordFunc,recordArg]?:@"exceptionHandler" exception:message userInfo:reportInfo];
                
                if (nil == instance) {
                    [WXExceptionUtils commitCriticalExceptionRT:jsExceptionInfo];
                }else{
                    [WXExceptionUtils commitCriticalExceptionRT:jsExceptionInfo.instanceId errCode:jsExceptionInfo.errorCode function:jsExceptionInfo.functionName exception:jsExceptionInfo.exception extParams:jsExceptionInfo.userInfo];
                }
      
                WX_MONITOR_FAIL(WXMTJSBridge, WX_ERR_JS_EXECUTE, message);
                if (instance.onJSRuntimeException && !tryFindInstanceInfoInGlobalContext) {
                    instance.onJSRuntimeException(jsExceptionInfo);
                }
            }
        }
        @finally {
            gInExceptionHandler = NO;
        }
    };
    
    if (WX_SYS_VERSION_LESS_THAN(@"8.0")) {
        // solve iOS7 memory problem
        context[@"nativeSet"] = [WXPolyfillSet class];
    }
    context[@"console"][@"error"] = ^(){
        [WXBridgeContext handleConsoleOutputWithArgument:[JSContext currentArguments] logLevel:WXLogFlagError];
    };
    context[@"console"][@"warn"] = ^(){
        [WXBridgeContext handleConsoleOutputWithArgument:[JSContext currentArguments] logLevel:WXLogFlagWarning];
    };
    context[@"console"][@"info"] = ^(){
        [WXBridgeContext handleConsoleOutputWithArgument:[JSContext currentArguments] logLevel:WXLogFlagInfo];
    };
    context[@"console"][@"debug"] = ^(){
        [WXBridgeContext handleConsoleOutputWithArgument:[JSContext currentArguments] logLevel:WXLogFlagDebug];
    };
    context[@"console"][@"log"] = ^(){
        [WXBridgeContext handleConsoleOutputWithArgument:[JSContext currentArguments] logLevel:WXLogFlagLog];
    };
    context[@"nativeLog"] = ^() {
        static NSDictionary *levelMap;
        static dispatch_once_t onceToken;
        dispatch_once(&onceToken, ^{
            levelMap = @{
                         @"__ERROR": @(WXLogFlagError),
                         @"__WARN": @(WXLogFlagWarning),
                         @"__INFO": @(WXLogFlagInfo),
                         @"__DEBUG": @(WXLogFlagDebug),
                         @"__LOG": @(WXLogFlagLog)
                         };
        });
        NSArray * args = [JSContext currentArguments];
        NSString * levelStr = [[args lastObject] toString];
        [WXBridgeContext handleConsoleOutputWithArgument:args logLevel:(WXLogFlag)[levelMap[levelStr] integerValue]];
    };
    
    context[@"extendCallNative"] = ^(JSValue *value ) {
        return [WXBridgeContext extendCallNative:[value toDictionary]];
    };
}

+ (void)handleConsoleOutputWithArgument:(NSArray *)arguments logLevel:(WXLogFlag)logLevel
{
    NSMutableString *string = [NSMutableString string];
    [string appendString:@"jsLog: "];
    [arguments enumerateObjectsUsingBlock:^(JSValue *jsVal, NSUInteger idx, BOOL *stop) {
        [string appendFormat:@"%@ ", jsVal];
        if (idx == arguments.count - 1) {
            if (logLevel) {
                if (WXLogFlagWarning == logLevel || WXLogFlagError == logLevel) {
                    id<WXAppMonitorProtocol> appMonitorHandler = [WXSDKEngine handlerForProtocol:@protocol(WXAppMonitorProtocol)];
                    if ([appMonitorHandler respondsToSelector:@selector(commitAppMonitorAlarm:monitorPoint:success:errorCode:errorMsg:arg:)]) {
                        [appMonitorHandler commitAppMonitorAlarm:@"weex" monitorPoint:@"jswarning" success:NO errorCode:@"99999" errorMsg:string arg:[WXSDKEngine topInstance].pageName];
                    }
                }
                WX_LOG(logLevel, @"%@", string);
            } else {
                [string appendFormat:@"%@ ", jsVal];
                WXLogInfo(@"%@", string);
            }
        }
    }];
}

+(id)extendCallNative:(NSDictionary *)dict
{
    if(dict){
        return [WXExtendCallNativeManager sendExtendCallNativeEvent:dict];
    }
    return @(-1);
}
@end
