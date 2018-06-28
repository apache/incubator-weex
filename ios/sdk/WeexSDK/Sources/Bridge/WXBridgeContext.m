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
#import "WXThreadSafeMutableArray.h"
#import "WXAppConfiguration.h"
#import "WXInvocationConfig.h"
#import "WXComponentMethod.h"
#import "WXModuleMethod.h"
#import "WXCallJSMethod.h"
#import "WXSDKInstance_private.h"
#import "WXPrerenderManager.h"
#import "WXTracingManager.h"
#import "WXExceptionUtils.h"
#import "WXSDKEngine.h"
#import "WXPolyfillSet.h"
#import "WXJSExceptionProtocol.h"
#import "WXMonitor.h"
#import "WXAppMonitorProtocol.h"
#import "WXConfigCenterProtocol.h"
#import "WXSDKInstance_performance.h"
#import "JSContext+Weex.h"

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
@property (nonatomic, strong) WXThreadSafeMutableArray    *insStack;
//identify if the JSFramework has been loaded
@property (nonatomic) BOOL frameworkLoadFinished;
//store some methods temporarily before JSFramework is loaded
@property (nonatomic, strong) NSMutableArray *methodQueue;
// store service
@property (nonatomic, strong) NSMutableArray *jsServiceQueue;

@end

@implementation WXBridgeContext

- (instancetype) init
{
    self = [super init];
    if (self) {
        _methodQueue = [NSMutableArray new];
        _frameworkLoadFinished = NO;
        _jsServiceQueue = [NSMutableArray new];
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
    [_jsBridge registerCallAddElement:^NSInteger(NSString *instanceId, NSString *parentRef, NSDictionary *elementData, NSInteger index) {
        
        // Temporary here , in order to improve performance, will be refactored next version.
        WXSDKInstance *instance = [WXSDKManager instanceForID:instanceId];
        
        if(![weakSelf checkInstance:instance]) {
            return -1;
        }
        [WXTracingManager startTracingWithInstanceId:instanceId ref:elementData[@"ref"] className:nil name:WXTJSCall phase:WXTracingEnd functionName:@"addElement" options:nil];
        WXPerformBlockOnComponentThread(^{
#ifdef DEBUG
            WXLogDebug(@"flexLayout -> action: addElement : %@",elementData[@"type"]);
#endif
            WXComponentManager *manager = instance.componentManager;
            if (!manager.isValid) {
                return;
            }
            [WXTracingManager startTracingWithInstanceId:instanceId ref:elementData[@"ref"] className:nil name:WXTDomCall phase:WXTracingBegin functionName:@"addElement" options:@{@"threadName":WXTDOMThread}];
            [manager startComponentTasks];
            [manager addComponent:elementData toSupercomponent:parentRef atIndex:index appendingInTree:NO];
            [WXTracingManager startTracingWithInstanceId:instanceId ref:elementData[@"ref"] className:nil name:WXTDomCall phase:WXTracingEnd functionName:@"addElement" options:@{@"threadName":WXTDOMThread}];
        });
        
        return 0;
    }];
    
    [_jsBridge registerCallCreateBody:^NSInteger(NSString *instanceId, NSDictionary *bodyData) {
        
        // Temporary here , in order to improve performance, will be refactored next version.
        WXSDKInstance *instance = [WXSDKManager instanceForID:instanceId];
        
        if(![weakSelf checkInstance:instance]) {
            return -1;
        }
        [WXTracingManager startTracingWithInstanceId:instanceId ref:bodyData[@"ref"] className:nil name:WXTJSCall phase:WXTracingEnd functionName:@"createBody" options:@{@"threadName":WXTJSBridgeThread}];
        WXPerformBlockOnComponentThread(^{
#ifdef DEBUG
            WXLogDebug(@"flexLayout -> action: createBody %@ ref:%@",bodyData[@"type"],bodyData[@"ref"]);
#endif
            WXComponentManager *manager = instance.componentManager;
            if (!manager.isValid) {
                return;
            }
            [WXTracingManager startTracingWithInstanceId:instanceId ref:bodyData[@"ref"] className:nil name:WXTDomCall phase:WXTracingBegin functionName:@"createBody" options:@{@"threadName":WXTDOMThread}];
            [manager startComponentTasks];
            [manager createRoot:bodyData];
            [WXTracingManager startTracingWithInstanceId:instanceId ref:bodyData[@"ref"] className:nil name:WXTDomCall phase:WXTracingEnd functionName:@"createBody" options:@{@"threadName":WXTDOMThread}];
            
        });
        
        return 0;
    }];
    
    [_jsBridge registerCallRemoveElement:^NSInteger(NSString *instanceId, NSString *ref) {
        
        // Temporary here , in order to improve performance, will be refactored next version.
        WXSDKInstance *instance = [WXSDKManager instanceForID:instanceId];
        
        if(![weakSelf checkInstance:instance]) {
            return -1;
        }
        [WXTracingManager startTracingWithInstanceId:instanceId ref:ref className:nil name:WXTJSCall phase:WXTracingEnd functionName:@"removeElement" options:nil];
        WXPerformBlockOnComponentThread(^{
#ifdef DEBUG
            WXLogDebug(@"flexLayout -> action: removeElement ref:%@",ref);
#endif
            WXComponentManager *manager = instance.componentManager;
            if (!manager.isValid) {
                return;
            }
            [WXTracingManager startTracingWithInstanceId:instanceId ref:ref className:nil name:WXTDomCall phase:WXTracingBegin functionName:@"removeElement" options:@{@"threadName":WXTDOMThread}];
            [manager startComponentTasks];
            [manager removeComponent:ref];
            [WXTracingManager startTracingWithInstanceId:instanceId ref:ref className:nil name:WXTDomCall phase:WXTracingEnd functionName:@"removeElement" options:@{@"threadName":WXTDOMThread}];
        });
        
        return 0;
    }];
    
    [_jsBridge registerCallMoveElement:^NSInteger(NSString *instanceId,NSString *ref,NSString *parentRef,NSInteger index) {
        
        // Temporary here , in order to improve performance, will be refactored next version.
        WXSDKInstance *instance = [WXSDKManager instanceForID:instanceId];
        
        if(![weakSelf checkInstance:instance]) {
            return -1;
        }
        [WXTracingManager startTracingWithInstanceId:instanceId ref:ref className:nil name:WXTJSCall phase:WXTracingEnd functionName:@"moveElement" options:nil];
        WXPerformBlockOnComponentThread(^{
#ifdef DEBUG
            WXLogDebug(@"flexLayout -> action: moveElement ,ref:%@ to ref:%@",ref,parentRef);
#endif
            WXComponentManager *manager = instance.componentManager;
            if (!manager.isValid) {
                return;
            }
            [manager startComponentTasks];
            [manager moveComponent:ref toSuper:parentRef atIndex:index];
        });
        
        return 0;
    }];
    
    [_jsBridge registerCallUpdateAttrs:^NSInteger(NSString *instanceId,NSString *ref,NSDictionary *attrsData) {
        
        // Temporary here , in order to improve performance, will be refactored next version.
        WXSDKInstance *instance = [WXSDKManager instanceForID:instanceId];
        
        if(![weakSelf checkInstance:instance]) {
            return -1;
        }
        
        [WXTracingManager startTracingWithInstanceId:instanceId ref:ref className:nil name:WXTJSCall phase:WXTracingEnd functionName:@"updateAttrs" options:@{@"threadName":WXTJSBridgeThread}];
        WXPerformBlockOnComponentThread(^{
#ifdef DEBUG
            WXLogDebug(@"flexLayout -> action: updateAttrs ref:%@,attr:%@",ref,attrsData);
#endif
            WXComponentManager *manager = instance.componentManager;
            if (!manager.isValid) {
                return;
            }
            [WXTracingManager startTracingWithInstanceId:instanceId ref:ref className:nil name:WXTDomCall phase:WXTracingBegin functionName:@"updateAttrs" options:@{@"threadName":WXTDOMThread}];
            [manager startComponentTasks];
            [manager updateAttributes:attrsData forComponent:ref];
            [WXTracingManager startTracingWithInstanceId:instanceId ref:ref className:nil name:WXTDomCall phase:WXTracingEnd functionName:@"updateAttrs" options:@{@"threadName":WXTDOMThread}];
        });
        
        return 0;
    }];
    
    [_jsBridge registerCallUpdateStyle:^NSInteger(NSString *instanceId,NSString *ref,NSDictionary *stylesData) {
        
        // Temporary here , in order to improve performance, will be refactored next version.
        WXSDKInstance *instance = [WXSDKManager instanceForID:instanceId];
        
        if(![weakSelf checkInstance:instance]) {
            return -1;
        }
        [WXTracingManager startTracingWithInstanceId:instanceId ref:ref className:nil name:WXTJSCall phase:WXTracingEnd functionName:@"updateStyles" options:@{@"threadName":WXTJSBridgeThread}];
        WXPerformBlockOnComponentThread(^{
            
#ifdef DEBUG
            WXLogDebug(@"flexLayout -> action: updateStyles ref:%@,styles:%@",ref,stylesData);
#endif
            WXComponentManager *manager = instance.componentManager;
            if (!manager.isValid) {
                return;
            }
            [manager startComponentTasks];
            [manager updateStyles:stylesData forComponent:ref];
            
        });
        
        return 0;
    }];
    
    [_jsBridge registerCallAddEvent:^NSInteger(NSString *instanceId,NSString *ref,NSString *event) {
        
        // Temporary here , in order to improve performance, will be refactored next version.
        WXSDKInstance *instance = [WXSDKManager instanceForID:instanceId];
        
        if(![weakSelf checkInstance:instance]) {
            return -1;
        }
        
        WXPerformBlockOnComponentThread(^{
#ifdef DEBUG
            WXLogDebug(@"flexLayout -> action: addEvent ref:%@",ref);
#endif
            
            WXComponentManager *manager = instance.componentManager;
            if (!manager.isValid) {
                return;
            }
            [manager startComponentTasks];
            [manager addEvent:event toComponent:ref];
            [WXTracingManager startTracingWithInstanceId:instanceId ref:ref className:nil name:WXTJSCall phase:WXTracingEnd functionName:@"addEvent" options:nil];
        });
        
        return 0;
    }];
    
    [_jsBridge registerCallRemoveEvent:^NSInteger(NSString *instanceId,NSString *ref,NSString *event) {
        
        // Temporary here , in order to improve performance, will be refactored next version.
        WXSDKInstance *instance = [WXSDKManager instanceForID:instanceId];
        
        if(![weakSelf checkInstance:instance]) {
            return -1;
        }
        
        WXPerformBlockOnComponentThread(^{
#ifdef DEBUG
            WXLogDebug(@"flexLayout -> action :removeEvent ref:%@",ref);
#endif
            WXComponentManager *manager = instance.componentManager;
            if (!manager.isValid) {
                return;
            }
            [manager startComponentTasks];
            [manager removeEvent:event fromComponent:ref];
            [WXTracingManager startTracingWithInstanceId:instanceId ref:ref className:nil name:WXTJSCall phase:WXTracingEnd functionName:@"removeEvent" options:nil];
        });
        
        return 0;
    }];
    
    [_jsBridge registerCallCreateFinish:^NSInteger(NSString *instanceId) {
        
        // Temporary here , in order to improve performance, will be refactored next version.
        WXSDKInstance *instance = [WXSDKManager instanceForID:instanceId];
        
        if(![weakSelf checkInstance:instance]) {
            return -1;
        }
        [WXTracingManager startTracingWithInstanceId:instanceId ref:nil className:nil name:WXTJSCall phase:WXTracingEnd functionName:@"createFinish" options:@{@"threadName":WXTJSBridgeThread}];
        WXPerformBlockOnComponentThread(^{
#ifdef DEBUG
            WXLogDebug(@"flexLayout -> action: createFinish :%@",instanceId);
#endif
            
            WXComponentManager *manager = instance.componentManager;
            if (!manager.isValid) {
                return;
            }
            [WXTracingManager startTracingWithInstanceId:instanceId ref:nil className:nil name:WXTDomCall phase:WXTracingBegin functionName:@"createFinish" options:@{@"threadName":WXTDOMThread}];
            [manager startComponentTasks];
            [manager createFinish];
            [WXTracingManager startTracingWithInstanceId:instanceId ref:nil className:nil name:WXTDomCall phase:WXTracingEnd functionName:@"createFinish" options:@{@"threadName":WXTDOMThread}];
            
        });
        
        return 0;
    }];
    
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
        
        if ([WXSDKManager sharedInstance].multiContext && [instance.bundleType.lowercaseString isEqualToString:@"rax"]) {
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

    _insStack = [WXThreadSafeMutableArray array];
    
    return _insStack;
}

- (WXSDKInstance *)topInstance
{
    if (self.insStack.count > 0) {
        WXSDKInstance *topInstance = [WXSDKManager instanceForID:[self.insStack firstObject]];
        return topInstance;
    }
    
    return nil;
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
            [WXTracingManager startTracingWithInstanceId:instanceId ref:task[@"ref"] className:nil name:task[@"component"] phase:WXTracingBegin functionName:task[@"method"] options:nil];
        } else {
            NSString *moduleName = task[@"module"];
            NSDictionary *options = task[@"options"];
            WXModuleMethod *method = [[WXModuleMethod alloc] initWithModuleName:moduleName methodName:methodName arguments:arguments options:options instance:instance];
            [WXTracingManager startTracingWithInstanceId:instanceId ref:nil className:nil name:task[@"module"] phase:WXTracingBegin functionName:task[@"method"] options:@{@"threadName":WXTJSBridgeThread}];
            [method invoke];
        }
    }
    
    [self performSelector:@selector(_sendQueueLoop) withObject:nil];
    if (!instance.isJSCreateFinish) {
        NSTimeInterval diff = CACurrentMediaTime()*1000-startTime;
        instance.performance.fsCallNativeNum++;
        instance.performance.fsCallNativeTime =  instance.performance.fsCallNativeTime + diff;
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
    
    if (![self.insStack containsObject:instanceIdString]) {
        if ([options[@"RENDER_IN_ORDER"] boolValue]) {
            [self.insStack addObject:instanceIdString];
        } else {
            [self.insStack insertObject:instanceIdString atIndex:0];
        }
    }
    
    //create a sendQueue bind to the current instance
    NSMutableArray *sendQueue = [NSMutableArray array];
    [self.sendQueue setValue:sendQueue forKey:instanceIdString];
    NSArray *args = nil;
    WX_MONITOR_INSTANCE_PERF_START(WXFirstScreenJSFExecuteTime, [WXSDKManager instanceForID:instanceIdString]);
    WX_MONITOR_INSTANCE_PERF_START(WXPTJSCreateInstance, [WXSDKManager instanceForID:instanceIdString]);
    BOOL shoudMultiContext = [WXSDKManager sharedInstance].multiContext;
    __weak typeof(self) weakSelf = self;
    NSString * bundleType = nil;
    
    if (shoudMultiContext) {
        bundleType = [self _pareJSBundleType:instanceIdString jsBundleString:jsBundleString]; // bundleType can be Vue, Rax and the new framework.
    }
    if (bundleType&&shoudMultiContext) {
        NSMutableDictionary *newOptions = [options mutableCopy];
        if (!options) {
            newOptions = [NSMutableDictionary new];
        }
        [newOptions addEntriesFromDictionary:@{@"env":[WXUtility getEnvironment]}];
        newOptions[@"bundleType"] = bundleType;
        NSString *raxAPIScript = nil;
        NSString *raxAPIScriptPath = nil;
        WXSDKInstance *sdkInstance = [WXSDKManager instanceForID:instanceIdString];
        sdkInstance.bundleType = bundleType;
        if ([bundleType.lowercaseString isEqualToString:@"rax"]) {
             raxAPIScriptPath = [[NSBundle bundleForClass:[weakSelf class]] pathForResource:@"weex-rax-api" ofType:@"js"];
            raxAPIScript = [NSString stringWithContentsOfFile:raxAPIScriptPath encoding:NSUTF8StringEncoding error:nil];
            if (!raxAPIScript) {
                WXLogError(@"weex-rax-api can not found");
            }
        }
        
        if ([WXDebugTool isDevToolDebug]) {
            [self callJSMethod:@"createInstanceContext" args:@[instanceIdString, newOptions, data?:@[],raxAPIScript?:@""]];
            
            if ([NSURL URLWithString:sdkInstance.pageName]) {
                [sdkInstance.instanceJavaScriptContext executeJavascript:jsBundleString withSourceURL:[NSURL URLWithString:sdkInstance.pageName]];
            } else {
                [sdkInstance.instanceJavaScriptContext executeJavascript:jsBundleString];
            }
        } else {
            sdkInstance.callCreateInstanceContext = [NSString stringWithFormat:@"instanceId:%@\noptions:%@\ndata:%@",instanceIdString, newOptions,data];
            [self callJSMethod:@"createInstanceContext" args:@[instanceIdString, newOptions, data?:@[]] onContext:nil completion:^(JSValue *instanceContextEnvironment) {
                if (sdkInstance.pageName) {
                    if (@available(iOS 8.0, *)) {
                          [sdkInstance.instanceJavaScriptContext.javaScriptContext setName:sdkInstance.pageName];
                    } else {
                        // Fallback
                    }
                }
                sdkInstance.createInstanceContextResult = [NSString stringWithFormat:@"%@", [[instanceContextEnvironment toDictionary] allKeys]];
                JSGlobalContextRef instanceContextRef = sdkInstance.instanceJavaScriptContext.javaScriptContext.JSGlobalContextRef;
                JSObjectRef instanceGlobalObject = JSContextGetGlobalObject(instanceContextRef);
                for (NSString * key in [[instanceContextEnvironment toDictionary] allKeys]) {
                    JSStringRef propertyName = JSStringCreateWithUTF8CString([key cStringUsingEncoding:NSUTF8StringEncoding]);
                    if ([key isEqualToString:@"Vue"]) {
                        JSObjectSetPrototype(instanceContextRef, JSValueToObject(instanceContextRef, [instanceContextEnvironment valueForProperty:key].JSValueRef, NULL), JSObjectGetPrototype(instanceContextRef, instanceGlobalObject));
                    }
                    JSObjectSetProperty(instanceContextRef, instanceGlobalObject, propertyName, [instanceContextEnvironment valueForProperty:key].JSValueRef, 0, NULL);
                }
                
                if (WX_SYS_VERSION_LESS_THAN(@"10.2")) {
                    NSString *filePath = [[NSBundle bundleForClass:[weakSelf class]] pathForResource:@"weex-polyfill" ofType:@"js"];
                    NSString *script = [NSString stringWithContentsOfFile:filePath encoding:NSUTF8StringEncoding error:nil];
                    if (script) {
                        [sdkInstance.instanceJavaScriptContext executeJavascript:script withSourceURL:[NSURL URLWithString:filePath]];
                    } else {
                        WXLogError(@"weex-pollyfill can not found");
                    }
                }
                
                if (raxAPIScript) {
                    [sdkInstance.instanceJavaScriptContext executeJavascript:raxAPIScript withSourceURL:[NSURL URLWithString:raxAPIScriptPath]];
                    sdkInstance.executeRaxApiResult = [NSString stringWithFormat:@"%@", [[sdkInstance.instanceJavaScriptContext.javaScriptContext.globalObject toDictionary] allKeys]];
                }
                
                if ([NSURL URLWithString:sdkInstance.pageName] || sdkInstance.scriptURL) {
                    [sdkInstance.instanceJavaScriptContext executeJavascript:jsBundleString withSourceURL:[NSURL URLWithString:sdkInstance.pageName]?:sdkInstance.scriptURL];
                } else {
                    [sdkInstance.instanceJavaScriptContext executeJavascript:jsBundleString];
                }
                
            }];
        }
        
    } else {
        if (data){
            args = @[instanceIdString, jsBundleString, options ?: @{}, data];
        } else {
            args = @[instanceIdString, jsBundleString, options ?: @{}];
        }
        [self callJSMethod:@"createInstance" args:args];
    }
    WX_MONITOR_INSTANCE_PERF_END(WXPTJSCreateInstance, [WXSDKManager instanceForID:instanceIdString]);
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
    // trim like whiteSpace and newline charset
    jsBundleString = [jsBundleString stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceAndNewlineCharacterSet]];
    
    // use the top 100 characters match the bundleType
    if (jsBundleString.length > 100) {
        jsBundleString = [jsBundleString substringWithRange:NSMakeRange(0, 100)];
    }
    
    if (!jsBundleString ) {
        return bundleType;
    }
    if ([jsBundleString hasPrefix:@"// { \"framework\": \"Vue\""] || [jsBundleString hasPrefix:@"// { \"framework\": \"vue\""]) {
        bundleType = @"Vue";
    } else if ([jsBundleString hasPrefix:@"// { \"framework\": \"Rax\""] || [jsBundleString hasPrefix:@"// { \"framework\": \"rax\""] || [jsBundleString hasPrefix:@"// {\"framework\" : \"Rax\"}"] || [jsBundleString hasPrefix:@"// {\"framework\" : \"rax\"}"]) {
        bundleType = @"Rax";
    }else {
        NSRegularExpression * regEx = [NSRegularExpression regularExpressionWithPattern:@"(use)(\\s+)(weex:vue)" options:NSRegularExpressionCaseInsensitive error:NULL];
        NSTextCheckingResult *match = [regEx firstMatchInString:jsBundleString options:0 range:NSMakeRange(0, jsBundleString.length)];
        if (match) {
            bundleType = [jsBundleString substringWithRange:match.range];
            return bundleType;
        }
        regEx = [NSRegularExpression regularExpressionWithPattern:@"(use)(\\s+)(weex:rax)" options:NSRegularExpressionCaseInsensitive error:NULL];
         match = [regEx firstMatchInString:jsBundleString options:0 range:NSMakeRange(0, jsBundleString.length)];
        if (match) {
            bundleType = [jsBundleString substringWithRange:match.range];
        }
    }
    
    return bundleType;
}

- (void)destroyInstance:(NSString *)instance
{
    WXAssertBridgeThread();
    WXAssertParam(instance);
    
    //remove instance from stack
    if([self.insStack containsObject:instance]){
        [self.insStack removeObject:instance];
    }
    
    if(_jsBridge && [_jsBridge respondsToSelector:@selector(removeTimers:)]){
        [_jsBridge removeTimers:instance];
    }

    if(self.sendQueue[instance]){
        [self.sendQueue removeObjectForKey:instance];
    }
    
    [self callJSMethod:@"destroyInstance" args:@[instance]];
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
    
    [self callJSMethod:@"refreshInstance" args:@[instance, data]];
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
        WX_MONITOR_FAIL(WXMTJSFramework, WX_ERR_JSFRAMEWORK_EXECUTE, errMsg);
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

- (void)callJSMethod:(NSString *)method args:(NSArray *)args onContext:(id<WXBridgeProtocol>)bridge completion:(void (^)(JSValue * value))complection
{
    NSMutableArray *newArg = nil;
    if (!bridge) {
        bridge = self.jsBridge;
    }
    if (self.frameworkLoadFinished) {
        newArg = [args mutableCopy];
        if ([newArg containsObject:complection]) {
            [newArg removeObject:complection];
        }
        WXLogDebug(@"Calling JS... method:%@, args:%@", method, args);
        if ([bridge isKindOfClass:[WXJSCoreBridge class]]) {
            JSValue *value = [bridge callJSMethod:method args:args];
            if (complection) {
                complection(value);
            }
        } else {
            [bridge callJSMethod:method args:args];
        }
    } else {
        newArg = [args mutableCopy];
        if (complection) {
            [newArg addObject:complection];
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
        [self.jsBridge executeJavascript:script];
        
        if ([self.jsBridge exception]) {
            NSString *exception = [[self.jsBridge exception] toString];
            NSMutableString *errMsg = [NSMutableString stringWithFormat:@"[WX_KEY_EXCEPTION_INVOKE_JSSERVICE_EXECUTE] %@",exception];
            [WXExceptionUtils commitCriticalExceptionRT:@"WX_KEY_EXCEPTION_INVOKE" errCode:[NSString stringWithFormat:@"%d", WX_KEY_EXCEPTION_INVOKE] function:@"" exception:errMsg extParams:nil];
            WX_MONITOR_FAIL(WXMTJSService, WX_ERR_JSFRAMEWORK_EXECUTE, errMsg);
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
}

- (void)registerComponents:(NSArray *)components
{
    WXAssertBridgeThread();
    
    if(!components) return;
    
    [self callJSMethod:@"registerComponents" args:@[components]];
}

- (void)callJSMethod:(NSString *)method args:(NSArray *)args
{
    if (self.frameworkLoadFinished) {
        [self.jsBridge callJSMethod:method args:args];
    } else {
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
    
    if ([tasks count] > 0 && execIns) {
        WXSDKInstance * execInstance = [WXSDKManager instanceForID:execIns];
        NSTimeInterval start = -1;
        if (execInstance && !(execInstance.isJSCreateFinish)) {
            start = CACurrentMediaTime()*1000;
        }
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
    context.exceptionHandler = ^(JSContext *context, JSValue *exception){
        context.exception = exception;
        NSString *errorCode = [NSString stringWithFormat:@"%d", WX_KEY_EXCEPTION_WXBRIDGE];;
        NSString *bundleUrl = nil;
        NSString *message = nil;
        NSDictionary *userInfo = nil;
        BOOL commitException = YES;
        WXSDKInstance * instance = nil;
        if ([WXSDKManager sharedInstance].multiContext) {
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
                message = [NSString stringWithFormat:@"[WX_KEY_EXCEPTION_WXBRIDGE] [%@:%@:%@] %@\n%@", exception[@"sourceURL"], exception[@"line"], exception[@"column"], [exception toString], [exception[@"stack"] toObject]];
                if (!JSValueIsUndefined(context.JSGlobalContextRef, exception[@"sourceURL"].JSValueRef)) {
                    bundleUrl = exception[@"sourceURL"].toString;
                } else {
                    bundleUrl = @"weex-main-jsfm";
                }
                userInfo = [NSDictionary dictionary];
            }
        } else {
            instance = [WXSDKEngine topInstance];
        }
        
        if (instance) {
            bundleUrl = instance.pageName?:([instance.scriptURL absoluteString]?:@"WX_KEY_EXCEPTION_WXBRIDGE");
            message = [NSString stringWithFormat:@"[WX_KEY_EXCEPTION_WXBRIDGE] [%@:%@:%@] %@\n%@\n%@\n%@\n%@\n%@", exception[@"sourceURL"], exception[@"line"], exception[@"column"], [exception toString], [exception[@"stack"] toObject], instance.scriptURL.absoluteString, instance.callCreateInstanceContext?:@"", instance.createInstanceContextResult?:@"", instance.executeRaxApiResult?:@""];
            userInfo = @{@"jsMainBundleStringContentLength":instance.userInfo[@"jsMainBundleStringContentLength"]?:@"",
                         @"jsMainBundleStringContentMd5":instance.userInfo[@"jsMainBundleStringContentMd5"]?:@""};
            if ([self checkEmptyScreen:instance]) {
                errorCode = [NSString stringWithFormat:@"%d", WX_KEY_EXCEPTION_EMPTY_SCREEN_JS];
            }
        }
        
        if (commitException) {
            WXJSExceptionInfo * jsExceptionInfo = [[WXJSExceptionInfo alloc] initWithInstanceId:instance.instanceId bundleUrl:bundleUrl errorCode:errorCode functionName:@"" exception:message userInfo:[userInfo mutableCopy]];
            
            [WXExceptionUtils commitCriticalExceptionRT:jsExceptionInfo];
            WX_MONITOR_FAIL(WXMTJSBridge, WX_ERR_JS_EXECUTE, message);
            if (instance.onJSRuntimeException) {
                instance.onJSRuntimeException(jsExceptionInfo);
            }
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
        [WXBridgeContext handleConsoleOutputWithArgument:args logLevel:(WXLogFlag)levelMap[levelStr]];
        
    };
}
+ (void)handleConsoleOutputWithArgument:(NSArray*)arguments logLevel:(WXLogFlag)logLevel
{
    NSMutableString *string = [NSMutableString string];
    [string appendString:@"jsLog: "];
    [arguments enumerateObjectsUsingBlock:^(JSValue *jsVal, NSUInteger idx, BOOL *stop) {
        [string appendFormat:@"%@ ", jsVal];
        if (idx == arguments.count - 1) {
            if (logLevel) {
                if (WXLogFlagWarning == logLevel) {
                    id<WXAppMonitorProtocol> appMonitorHandler = [WXSDKEngine handlerForProtocol:@protocol(WXAppMonitorProtocol)];
                    if ([appMonitorHandler respondsToSelector:@selector(commitAppMonitorAlarm:monitorPoint:success:errorCode:errorMsg:arg:)]) {
                        [appMonitorHandler commitAppMonitorAlarm:@"weex" monitorPoint:@"jswarning" success:FALSE errorCode:@"99999" errorMsg:string arg:[WXSDKEngine topInstance].pageName];
                    }
                }
                WX_LOG(logLevel, @"%@", string);
            } else {
                [string appendFormat:@"%@ ", jsVal]                                  ;
                WXLogInfo(@"%@", string);
            }
        }
    }];
}

+ (BOOL) checkEmptyScreen:(WXSDKInstance *) instance
{
    if(!instance){
        return false;
    }
    if (!(instance.rootView)  ) {
        return true;
    }
    CGRect rootFrame = instance.rootView.frame;

    if (rootFrame.size.height <=0 || rootFrame.size.width <=0) {
        return true;
    }
    
    if (!(instance.rootView.subviews) || instance.rootView.subviews.count <=0) {
        return true;
    }
    return false;
}
@end
