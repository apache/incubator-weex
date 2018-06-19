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

#import "WXJSCoreBridge.h"
#import "WXDefine.h"
#import "WXAssert.h"
#import "WXLog.h"
#import "WXDefine.h"
#import "WXUtility.h"
#import "WXSDKEngine.h"
#import "WXSDKError.h"
#import <sys/utsname.h>
#import <JavaScriptCore/JavaScriptCore.h>
#import "JSValue+Weex.h"
#import "WXSDKManager.h"
#import "WXExtendCallNativeManager.h"
#import "WXTracingManager.h"
#import "WXExceptionUtils.h"
#import "WXBridgeContext.h"
#import "WXMonitor.h"
#import "WXPolyfillSet.h"
#import "WXAppMonitorProtocol.h"
#import "JSContext+Weex.h"
#import "WXCoreBridge.h"

#import <dlfcn.h>

#import <mach/mach.h>

#ifdef WX_IMPORT_WEEXCORE

#import <core/manager/weex_core_manager.h>
#import <iOS/bridge/jsc_runtime_ios.h>
#import <iOS/bridge/bridge_impl_ios.h>
#import <core/bridge/js_bridge.h>
#import <iOS/bridge/measure_func_adapter_impl_ios.h>
#import <core/layout/measure_func_adapter.h>


#endif

@interface WXJSCoreBridge ()
{
    NSString * _weexInstanceId;
}

@property (nonatomic, strong)  JSContext *jsContext;
@property (nonatomic, strong)  NSMutableArray *timers;
@property (nonatomic, strong)  NSMutableDictionary *intervaltimers;
@property (nonatomic)  long long intervalTimerId;
@property (nonatomic, strong)  NSMutableDictionary *callbacks;

#ifdef WX_IMPORT_WEEXCORE
@property (nonatomic, assign) WeexCore::WXCoreBridge *coreBridge;
@property (nonatomic, assign) WeexCore::Bridge_Impl_iOS* bridgeImplIOS;
@property (nonatomic, assign) WeexCore::BaseJSRunTime* jsRunTime;
@property (nonatomic, assign) WeexCore::BaseJSContext* defaultContext;

#endif

@end

@implementation WXJSCoreBridge

- (instancetype)init
{
    self = [super init];
    
    if(self){
#ifndef WX_IMPORT_WEEXCORE
        _jsContext = [[JSContext alloc] init];
        if (WX_SYS_VERSION_GREATER_THAN_OR_EQUAL_TO(@"8.0")) {
            _jsContext.name = @"Weex Context";
        }
#else
        _coreBridge = new WeexCore::WXCoreBridge();
        _bridgeImplIOS = new WeexCore::Bridge_Impl_iOS();
        WeexCore::WeexCoreManager::getInstance()->setPlatformBridge(_bridgeImplIOS);
        WeexCore::WeexCoreManager::getInstance()->setJSBridge(new WeexCore::JSBridge());
        WeexCore::WeexCoreManager::getInstance()->SetMeasureFunctionAdapter(new WeexCore::MeasureFunctionAdapterImplIOS());

        _jsRunTime = new WeexCore::JSCRunTimeIOS();
        _jsRunTime->initRunTime();
       _defaultContext =_jsRunTime->createContext();
        
        
        
        
      
#endif
        _timers = [NSMutableArray new];
        _callbacks = [NSMutableDictionary new];
        _intervalTimerId = 0;
        _intervaltimers = [NSMutableDictionary new];
        _multiContext = NO;

        __weak typeof(self) weakSelf = self;
#ifndef WX_IMPORT_WEEXCORE
        [WXBridgeContext mountContextEnvironment:_jsContext];
        
        _jsContext[@"setTimeout"] = ^(JSValue *function, JSValue *timeout) {
            // this setTimeout is used by internal logic in JS framework, normal setTimeout called by users will call WXTimerModule's method;
            [weakSelf performSelector: @selector(triggerTimeout:) withObject:^() {
                [function callWithArguments:@[]];
            } afterDelay:[timeout toDouble] / 1000];
        };
        
        _jsContext[@"setTimeoutWeex"] = ^(JSValue *appId, JSValue *ret,JSValue *arg ) {
            [weakSelf triggerTimeout:[appId toString] ret:[ret toString] arg:[arg toString]];
        };
        
        _jsContext[@"setIntervalWeex"] = ^(JSValue *appId, JSValue *function,JSValue *arg) {
            return [weakSelf triggerInterval:[appId toString] function:^() {
                [function callWithArguments:@[]];
            } arg:[arg toString]];
        };
        
        _jsContext[@"clearIntervalWeex"] = ^(JSValue *appId, JSValue *ret,JSValue *arg) {
            
            [weakSelf triggerClearInterval:[appId toString] ret:[[ret toNumber] longLongValue]];
        };
        
        _jsContext[@"clearTimeoutWeex"] = ^(JSValue *ret) {
            [weakSelf triggerClearTimeout:[ret toString]];
        };
        
        _jsContext[@"extendCallNative"] = ^(JSValue *value ) {
            return [weakSelf extendCallNative:[value toDictionary]];
        };
#endif
    }
    return self;
}

- (void)dealloc
{
    _jsContext.instanceId = nil;
#ifdef WX_IMPORT_WEEXCORE
    WeexCore::WXCoreBridge *t = _coreBridge;
    _coreBridge = nullptr;
    delete t;
#endif
}

- (void)setJSContext:(JSContext *)context
{
    _jsContext = context;
}

- (JSContext *)javaScriptContext
{
    return _jsContext;
}

- (NSString *)weexInstanceId
{
    return _weexInstanceId;
}

- (void)setWeexInstanceId:(NSString *)weexInstanceId
{
    _jsContext.instanceId = weexInstanceId;
    _weexInstanceId = weexInstanceId;
}

#pragma mark - WXBridgeProtocol

- (void)executeJSFramework:(NSString *)frameworkScript
{

    WXAssertParam(frameworkScript);
#ifndef WX_IMPORT_WEEXCORE
    if (WX_SYS_VERSION_GREATER_THAN_OR_EQUAL_TO(@"8.0")) {
        NSString * fileName = @"native-bundle-main.js";
        if ([WXSDKManager sharedInstance].multiContext) {
            fileName = @"weex-main-jsfm.js";
        }
        [_jsContext evaluateScript:frameworkScript withSourceURL:[NSURL URLWithString:fileName]];
    }else{
        [_jsContext evaluateScript:frameworkScript];
    }
#else
    _defaultContext->executeJavascript([frameworkScript UTF8String],"");
#endif
}

- (JSValue *)callJSMethod:(NSString *)method args:(NSArray *)args
{
    WXLogDebug(@"Calling JS... method:%@, args:%@", method, args);
    return [[_jsContext globalObject] invokeMethod:method withArguments:args];
}

- (void)registerCallNative:(WXJSCallNative)callNative
{
#ifndef WX_IMPORT_WEEXCORE
    JSValue* (^callNativeBlock)(JSValue *, JSValue *, JSValue *) = ^JSValue*(JSValue *instance, JSValue *tasks, JSValue *callback){
        NSString *instanceId = [instance toString];
        NSArray *tasksArray = [tasks toArray];
        NSString *callbackId = [callback toString];
        WXLogDebug(@"Calling native... instance:%@, tasks:%@, callback:%@", instanceId, tasksArray, callbackId);
        return [JSValue valueWithInt32:(int32_t)callNative(instanceId, tasksArray, callbackId) inContext:[JSContext currentContext]];
    };
    
    _jsContext[@"callNative"] = callNativeBlock;
#else
    id WXJSCallNativeBlock = ^NSInteger(NSString *instance, NSArray *tasks, NSString *callback){
        
        WXLogDebug(@"Calling native... instance:%@, tasks:%@, callback:%@", instance, tasks, callback);
        
        return callNative(instance,tasks,callback);
    };
    _coreBridge->registerEventWithType(WeexCoreEventBlockTypeCallAddEvent, (__bridge void*)WXJSCallNativeBlock);
#warning todo
//    _jsContext[@"callNative"] = callNativeBlock;
#endif
    
}

- (void)executeJavascript:(NSString *)script
{
    WXAssertParam(script);
    [_jsContext evaluateScript:script];
}

- (JSValue*)executeJavascript:(NSString *)script withSourceURL:(NSURL*)sourceURL
{
    WXAssertParam(script);
#ifndef WX_IMPORT_WEEXCORE
    if (sourceURL) {
        return [_jsContext evaluateScript:script withSourceURL:sourceURL];
    } else {
        return [_jsContext evaluateScript:script];
    }
#else
    _defaultContext->executeJavascript([script UTF8String],"");
#endif
}

- (void)registerCallAddElement:(WXJSCallAddElement)callAddElement
{
    id callAddElementBlock = ^(JSValue *instanceId, JSValue *ref, JSValue *element, JSValue *index, JSValue *ifCallback) {
        
        NSString *instanceIdString = [instanceId toString];
        NSDictionary *componentData = [element toDictionary];
        NSString *parentRef = [ref toString];
        NSInteger insertIndex = [[index toNumber] integerValue];
        [WXTracingManager startTracingWithInstanceId:instanceIdString ref:componentData[@"ref"] className:nil name:WXTJSCall phase:WXTracingBegin functionName:@"addElement" options:@{@"threadName":WXTJSBridgeThread,@"componentData":componentData}];
         WXLogDebug(@"callAddElement...%@, %@, %@, %ld", instanceIdString, parentRef, componentData, (long)insertIndex);
        
        return [JSValue valueWithInt32:(int32_t)callAddElement(instanceIdString, parentRef, componentData, insertIndex) inContext:[JSContext currentContext]];
    };
    
    _jsContext[@"callAddElement"] = callAddElementBlock;
}

- (void)registerCallCreateBody:(WXJSCallCreateBody)callCreateBody
{
    id WXJSCallCreateBodyBlock = ^(JSValue *instanceId, JSValue *body,JSValue *ifCallback) {
        
        NSString *instanceIdString = [instanceId toString];
        NSDictionary *bodyData = [body toDictionary];
        
        WXLogDebug(@"callCreateBody...%@, %@,", instanceIdString, bodyData);
        [WXTracingManager startTracingWithInstanceId:instanceIdString ref:bodyData[@"ref"] className:nil name:WXTJSCall phase:WXTracingBegin functionName:@"createBody" options:@{@"threadName":WXTJSBridgeThread}];
        return [JSValue valueWithInt32:(int32_t)callCreateBody(instanceIdString, bodyData) inContext:[JSContext currentContext]];
    };
    
    _jsContext[@"callCreateBody"] = WXJSCallCreateBodyBlock;
}

- (void)registerCallRemoveElement:(WXJSCallRemoveElement)callRemoveElement
{
    id WXJSCallCreateBodyBlock = ^(JSValue *instanceId, JSValue *ref,JSValue *ifCallback) {
        
        NSString *instanceIdString = [instanceId toString];
        NSString *refString = [ref toString];
        
        WXLogDebug(@"callRemoveElement...%@, %@,", instanceIdString, refString);
        [WXTracingManager startTracingWithInstanceId:instanceIdString ref:nil className:nil name:WXTJSCall phase:WXTracingBegin functionName:@"removeElement" options:nil];
        return [JSValue valueWithInt32:(int32_t)callRemoveElement(instanceIdString, refString) inContext:[JSContext currentContext]];
    };
    
    _jsContext[@"callRemoveElement"] = WXJSCallCreateBodyBlock;
}

- (void)registerCallMoveElement:(WXJSCallMoveElement)callMoveElement
{
    id WXJSCallMoveElementBlock = ^(JSValue *instanceId, JSValue *ref,JSValue *parentRef,JSValue *index, JSValue *ifCallback) {
        
        NSString *instanceIdString = [instanceId toString];
        NSString *refString = [ref toString];
        NSString *parentRefString = [parentRef toString];
        NSInteger moveIndex = [[index toNumber] integerValue];
        
        WXLogDebug(@"callAddElement...%@, %@,", instanceIdString, refString);
        [WXTracingManager startTracingWithInstanceId:instanceIdString ref:refString className:nil name:WXTJSCall phase:WXTracingBegin functionName:@"moveElement" options:nil];
        return [JSValue valueWithInt32:(int32_t)callMoveElement(instanceIdString, refString,parentRefString,moveIndex) inContext:[JSContext currentContext]];
    };
    
    _jsContext[@"callMoveElement"] = WXJSCallMoveElementBlock;
}

- (void)registerCallUpdateAttrs:(WXJSCallUpdateAttrs)callUpdateAttrs
{
    id WXJSCallUpdateAttrsBlock = ^(JSValue *instanceId, JSValue *ref,JSValue *attrs, JSValue *ifCallback) {
        
        NSString *instanceIdString = [instanceId toString];
        NSString *refString = [ref toString];
        NSDictionary *attrsData = [attrs toDictionary];
        
        WXLogDebug(@"callUpdateAttrs...%@, %@, %@", instanceIdString, refString,attrsData);
        [WXTracingManager startTracingWithInstanceId:instanceIdString ref:refString className:nil name:WXTJSCall phase:WXTracingBegin functionName:@"updateAttrs" options:@{@"threadName":WXTJSBridgeThread}];
        return [JSValue valueWithInt32:(int32_t)callUpdateAttrs(instanceIdString, refString,attrsData) inContext:[JSContext currentContext]];
    };
    
    _jsContext[@"callUpdateAttrs"] = WXJSCallUpdateAttrsBlock;
}

- (void)registerCallUpdateStyle:(WXJSCallUpdateStyle)callUpdateStyle
{
    id WXJSCallUpdateStyleBlock = ^(JSValue *instanceId, JSValue *ref,JSValue *styles, JSValue *ifCallback) {
        
        NSString *instanceIdString = [instanceId toString];
        NSString *refString = [ref toString];
        NSDictionary *stylessData = [styles toDictionary];
        
        WXLogDebug(@"callUpdateStyle...%@, %@, %@", instanceIdString, refString,stylessData);
        [WXTracingManager startTracingWithInstanceId:instanceIdString ref:refString className:nil name:WXTJSCall phase:WXTracingBegin functionName:@"updateStyle" options:@{@"threadName":WXTJSBridgeThread}];
        return [JSValue valueWithInt32:(int32_t)callUpdateStyle(instanceIdString, refString,stylessData) inContext:[JSContext currentContext]];
    };
    
    _jsContext[@"callUpdateStyle"] = WXJSCallUpdateStyleBlock;
}

- (void)registerCallAddEvent:(WXJSCallAddEvent)callAddEvent
{
#ifndef WX_IMPORT_WEEXCORE
    id WXJSCallAddEventBlock = ^(JSValue *instanceId, JSValue *ref,JSValue *event, JSValue *ifCallback) {
        
        NSString *instanceIdString = [instanceId toString];
        NSString *refString = [ref toString];
        NSString *eventString = [event toString];
        
        WXLogDebug(@"callAddEvent...%@, %@, %@", instanceIdString, refString,eventString);
        [WXTracingManager startTracingWithInstanceId:instanceIdString ref:refString className:nil name:WXTJSCall phase:WXTracingBegin functionName:@"addEvent" options:nil];
        return [JSValue valueWithInt32:(int32_t)callAddEvent(instanceIdString, refString,eventString) inContext:[JSContext currentContext]];
    };
    
    _jsContext[@"callAddEvent"] = WXJSCallAddEventBlock;
#else
    id WXJSCallAddEventBlock = ^NSInteger(NSString *instanceId,NSString *ref,NSString *event){
        
        WXLogDebug(@"callAddEvent...%@, %@, %@", instanceId, ref,event);
        [WXTracingManager startTracingWithInstanceId:instanceId ref:ref className:nil name:WXTJSCall phase:WXTracingBegin functionName:@"addEvent" options:nil];
        
        return callAddEvent(instanceId,ref,event);
    };
    
    _coreBridge->registerEventWithType(WeexCoreEventBlockTypeCallAddEvent, (__bridge void*)WXJSCallAddEventBlock);
    
#warning todo
//    _jsContext[@"callAddEvent"] = WXJSCallAddEventBlock;
#endif
    
}

- (void)registerCallRemoveEvent:(WXJSCallRemoveEvent)callRemoveEvent
{
#ifndef WX_IMPORT_WEEXCORE
    id WXJSCallRemoveEventBlock = ^(JSValue *instanceId, JSValue *ref,JSValue *event, JSValue *ifCallback) {
        
        NSString *instanceIdString = [instanceId toString];
        NSString *refString = [ref toString];
        NSString *eventString = [event toString];
        
        WXLogDebug(@"callRemoveEvent...%@, %@, %@", instanceIdString, refString,eventString);
        [WXTracingManager startTracingWithInstanceId:instanceIdString ref:refString className:nil name:WXTJSCall phase:WXTracingBegin functionName:@"removeEvent" options:nil];
        return [JSValue valueWithInt32:(int32_t)callRemoveEvent(instanceIdString, refString,eventString) inContext:[JSContext currentContext]];
    };
#else
    id WXJSCallRemoveEventBlock = ^NSInteger (NSString *instanceId,NSString *ref,NSString *event){
        WXLogDebug(@"callRemoveEvent...%@, %@, %@", instanceId, ref,event);
        [WXTracingManager startTracingWithInstanceId:instanceId ref:ref className:nil name:WXTJSCall phase:WXTracingBegin functionName:@"removeEvent" options:nil];
        return callRemoveEvent(instanceId,ref,event);
    };
    _coreBridge->registerEventWithType(WeexCoreEventBlockTypeCallRemoveEvent, (__bridge void *)WXJSCallRemoveEventBlock);
#endif
    
//    _jsContext[@"callRemoveEvent"] = WXJSCallRemoveEventBlock;
}
    
- (void)registerCallCreateFinish:(WXJSCallCreateFinish)callCreateFinish
{
    id WXJSCallCreateFinishBlock = ^(JSValue *instanceId, JSValue *ifCallback) {
            
        NSString *instanceIdString = [instanceId toString];
        
        WXLogDebug(@"callRCreateFinish...%@", instanceIdString);
        [WXTracingManager startTracingWithInstanceId:instanceIdString ref:nil className:nil name:WXTJSCall phase:WXTracingBegin functionName:@"createFinish" options:@{@"threadName":WXTJSBridgeThread}];
        return [JSValue valueWithInt32:(int32_t)callCreateFinish(instanceIdString) inContext:[JSContext currentContext]];
    };
        
    _jsContext[@"callCreateFinish"] = WXJSCallCreateFinishBlock;
}

- (void)registerCallNativeModule:(WXJSCallNativeModule)callNativeModuleBlock
{
#ifndef WX_IMPORT_WEEXCORE
    _jsContext[@"callNativeModule"] = ^JSValue *(JSValue *instanceId, JSValue *moduleName, JSValue *methodName, JSValue *args, JSValue *options) {
        NSString *instanceIdString = [instanceId toString];
        NSString *moduleNameString = [moduleName toString];
        NSString *methodNameString = [methodName toString];
        NSArray *argsArray = [args toArray];
        NSDictionary *optionsDic = [options toDictionary];
        
        WXLogDebug(@"callNativeModule...%@,%@,%@,%@", instanceIdString, moduleNameString, methodNameString, argsArray);
        
        NSInvocation *invocation = callNativeModuleBlock(instanceIdString, moduleNameString, methodNameString, argsArray, optionsDic);
        JSValue *returnValue = [JSValue wx_valueWithReturnValueFromInvocation:invocation inContext:[JSContext currentContext]];
        [WXTracingManager startTracingWithInstanceId:instanceIdString ref:nil className:nil name:moduleNameString phase:WXTracingInstant functionName:methodNameString options:nil];
        return returnValue;
    };
#else
    
    id targetFunc = ^NSInvocation *(NSString *instanceId, NSString *moduleName, NSString *methodName, NSArray *args, NSDictionary *options){
        
        WXLogDebug(@"callNativeModule...%@,%@,%@,%@", instanceId, moduleName, methodName, args);
        
        NSInvocation *invocation = callNativeModuleBlock(instanceId, moduleName, methodName, args, options);
        
        [WXTracingManager startTracingWithInstanceId:instanceId ref:nil className:nil name:moduleName phase:WXTracingInstant functionName:methodName options:nil];
        return invocation;
    };
    _coreBridge->registerEventWithType(WeexCoreEventBlockTypeCallNativeModule, (__bridge void*)targetFunc);
    
#warning todo
//    _jsContext[@"callNativeModule"]=
#endif
}

- (void)registerCallNativeComponent:(WXJSCallNativeComponent)callNativeComponentBlock
{
#ifndef WX_IMPORT_WEEXCORE
    _jsContext[@"callNativeComponent"] = ^void(JSValue *instanceId, JSValue *componentName, JSValue *methodName, JSValue *args, JSValue *options) {
        NSString *instanceIdString = [instanceId toString];
        NSString *componentNameString = [componentName toString];
        NSString *methodNameString = [methodName toString];
        NSArray *argsArray = [args toArray];
        NSDictionary *optionsDic = [options toDictionary];
        
        WXLogDebug(@"callNativeComponent...%@,%@,%@,%@", instanceIdString, componentNameString, methodNameString, argsArray);
        
        callNativeComponentBlock(instanceIdString, componentNameString, methodNameString, argsArray, optionsDic);
    };
#else
    id targetFunc = ^(NSString *instanceId, NSString *componentRef, NSString *methodName, NSArray *args, NSDictionary *options){
        WXLogDebug(@"callNativeComponent...%@,%@,%@,%@", instanceId, componentRef, methodName, args);
        callNativeComponentBlock(instanceId,componentRef,methodName,args,options);
    };
    _coreBridge->registerEventWithType(WeexCoreEventBlockTypeCallNativeComponent, (__bridge void*) targetFunc);
#endif
}

#ifdef WX_IMPORT_WEEXCORE

- (void)registerCallLayout:(WXCoreCallLayout)callLayoutBlock
{
    //when WeexCore finish calculate frame
    id targetFunc = ^NSInteger(NSString *instanceId,NSString *ref,NSInteger top,NSInteger bottom,NSInteger left,NSInteger right,NSInteger height,NSInteger width,NSInteger index){
        WXLogDebug(@"callLayout...%@,%@,%ld,%ld,%ld,%ld", instanceId, ref, top, bottom, left,right);
        return callLayoutBlock(instanceId,ref,top,bottom,left,right,height,width,index);
    };
    _coreBridge->registerEventWithType(WeexCoreEventBlockTypeCallLayout, (__bridge void*)targetFunc);
}

#endif

- (JSValue*)exception
{
    return _jsContext.exception;
}

- (void)resetEnvironment
{
    NSDictionary *data = [WXUtility getEnvironment];
    _jsContext[@"WXEnvironment"] = data;
}

//typedef void (*WXJSCGarbageCollect)(JSContextRef);

- (void)garbageCollect
{
    // for dev and debug only!!
//    char str[80];
//    strcpy(str, "JSSynchron");
//    strcat(str, "ousGarbageColl");
//    strcat(str, "ectForDebugging");
//    WXJSCGarbageCollect garbageCollect = dlsym(RTLD_DEFAULT, str);
//    
//    if (garbageCollect != NULL) {
//        garbageCollect(_jsContext.JSGlobalContextRef);
//    }
//    JSGarbageCollect(_jsContext.JSGlobalContextRef);
}

#pragma mark - Public
-(void)removeTimers:(NSString *)instance
{
    // remove timers
    if([_callbacks objectForKey:instance]){
        NSMutableArray *arr = [_callbacks objectForKey:instance];
        if(arr && [arr count]>0){
            for (NSString *callback in arr) {
                if([_timers containsObject:callback]){
                    [_timers removeObject:callback];
                }
            }
        }
    }
    // remove intervaltimers
    if(_intervaltimers && [_intervaltimers objectForKey:instance]){
        [_intervaltimers removeObjectForKey:instance];
    }
}

#pragma mark - Private
-(void)addInstance:(NSString *)instance callback:(NSString *)callback
{
    if(instance.length > 0){
        if([_callbacks objectForKey:instance]){
            NSMutableArray *arr = [_callbacks objectForKey:instance];
            if (callback.length>0 && ![arr containsObject:callback]) {
                [arr addObject:callback];
                [_callbacks setObject:arr forKey:instance];
            }
        }else {
            NSMutableArray *arr = [NSMutableArray new];
            if (callback.length>0 && ![arr containsObject:callback]) {
                [arr addObject:callback];
                [_callbacks setObject:arr forKey:instance];
            }
        }
    }
}

- (void)triggerTimeout:(void(^)(void))block
{
    block();
}

- (void)callBack:(NSDictionary *)dic
{
    if([dic objectForKey:@"ret"] && [_timers containsObject:[dic objectForKey:@"ret"]]) {
        [[WXSDKManager bridgeMgr] callBack:[dic objectForKey:@"appId"] funcId:[dic objectForKey:@"ret"]  params:[dic objectForKey:@"arg"] keepAlive:NO];
    }

}


- (void)callBackInterval:(NSDictionary *)dic
{
    if(dic[@"function"] && [dic objectForKey:@"appId"] && [_intervaltimers objectForKey:[dic objectForKey:@"appId"]]){
        NSMutableArray *timers = [_intervaltimers objectForKey:[dic objectForKey:@"appId"]];
        void(^block)(void) = ((void(^)(void))dic[@"function"]);
        if(block && [timers containsObject:[dic objectForKey:@"timerId"]]){
            block();
            [self executeInterval:[dic objectForKey:@"appId"] function:block arg:[dic objectForKey:@"arg"] timerId:[[dic objectForKey:@"timerId"] longLongValue]];
        }
    }
}

- (void)triggerTimeout:(NSString *)appId ret:(NSString *)ret arg:(NSString *)arg
{
    double interval = [arg doubleValue]/1000.0f;
    if(WXFloatEqual(interval,0)) {
        return;
    }
    if(![_timers containsObject:ret]){
        [_timers addObject:ret];
        [self addInstance:appId callback:ret];
    }
    
    NSMutableDictionary *timeoutInfo = [NSMutableDictionary new];
    [timeoutInfo setObject:appId forKey:@"appId"];
    [timeoutInfo setObject:ret forKey:@"ret"];
    [timeoutInfo setObject:arg forKey:@"arg"];
    [self performSelector:@selector(callBack:) withObject:timeoutInfo afterDelay:interval inModes:@[NSRunLoopCommonModes]];
}

- (long long)triggerInterval:(NSString *)appId function:(void(^)(void))block arg:(NSString *)arg
{
    double interval = [arg doubleValue]/1000.0f;
    _intervalTimerId = _intervalTimerId + 1; // timerId must auto-increment.
    long long timerId = _intervalTimerId;
    if(WXFloatEqual(interval,0)) {
        return timerId;
    }
    if([_intervaltimers objectForKey:appId]){
        NSMutableArray *timers = [[_intervaltimers objectForKey:appId] mutableCopy];
        [timers addObject:@(timerId)];
        [_intervaltimers setObject:timers forKey:appId];
    }else {
        NSMutableArray *timers = [NSMutableArray new];
        [timers addObject:@(timerId)];
        [_intervaltimers setObject:timers forKey:appId];
    }
    [self executeInterval:appId function:block arg:arg timerId:timerId];
    return timerId;
}

-(void)executeInterval:(NSString *)appId function:(void(^)(void))block arg:(NSString *)arg timerId:(long long)timerId
{
    double interval = [arg doubleValue]/1000.0f;
    NSMutableDictionary *intervalInfo = [NSMutableDictionary new];
    [intervalInfo setObject:appId forKey:@"appId"];
    [intervalInfo setObject:arg forKey:@"arg"];
    [intervalInfo setObject:@(timerId) forKey:@"timerId"];
    [intervalInfo setObject:[block copy] forKey:@"function"];
    [self performSelector:@selector(callBackInterval:) withObject:intervalInfo afterDelay:interval inModes:@[NSRunLoopCommonModes]];
}

- (void)triggerClearInterval:(NSString *)instanceId ret:(long long)timerId
{
    if(_intervaltimers && [_intervaltimers objectForKey:instanceId]){
        NSMutableArray *timers = [_intervaltimers objectForKey:instanceId];
        if(timers && [timers containsObject:@(timerId)]){
            [timers removeObject:@(timerId)];
        }
    }
}

- (void)triggerClearTimeout:(NSString *)ret
{
    if([_timers containsObject:ret]){
        [_timers removeObject:ret];
    }
}

-(id)extendCallNative:(NSDictionary *)dict
{
    if(dict){
        return [WXExtendCallNativeManager sendExtendCallNativeEvent:dict];
    }
    return @(-1);
}

@end
