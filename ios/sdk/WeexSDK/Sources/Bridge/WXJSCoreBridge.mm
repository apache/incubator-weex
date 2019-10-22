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
#import "WXExceptionUtils.h"
#import "WXBridgeContext.h"
#import "WXMonitor.h"
#import "WXPolyfillSet.h"
#import "WXAppMonitorProtocol.h"
#import "JSContext+Weex.h"
#import "WXCoreBridge.h"
#import "WXAnalyzerCenter.h"

#import <dlfcn.h>

#import <mach/mach.h>

@interface WXJSCoreBridge ()
{
    NSString * _weexInstanceId;
}

@property (nonatomic, strong)  JSContext *jsContext;
@property (nonatomic, strong)  NSMutableArray *timers;
@property (nonatomic, strong)  NSMutableDictionary *intervaltimers;
@property (nonatomic)  long long intervalTimerId;
@property (nonatomic, strong)  NSMutableDictionary *callbacks;

@end

@implementation WXJSCoreBridge

- (instancetype)init
{
    self = [self initWithoutDefaultContext];
    [self createDefaultContext];
    return self;
}

- (instancetype)initWithoutDefaultContext
{
    if (self = [super init]) {
        _timers = [NSMutableArray new];
        _callbacks = [NSMutableDictionary new];
        _intervalTimerId = 0;
        _intervaltimers = [NSMutableDictionary new];
    }
    return self;
}

- (void)dealloc
{
    _jsContext.instanceId = nil;
    __block JSContext* theContext = _jsContext;
    _jsContext = nil; // Make sure that the context MUST be freed in JS thread.
    WXPerformBlockOnBridgeThreadForInstance(^{
         theContext = nil; // release the context in js thread to avoid main-thread deadlock
    }, _weexInstanceId);
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
    [_jsContext evaluateScript:frameworkScript withSourceURL:[NSURL URLWithString:@"weex-main-jsfm.js"]];
}

//static void __checkMutable(id container) {
//    if ([container isKindOfClass:[NSArray class]]) {
//        if ([container isKindOfClass:[NSMutableArray class]]) {
//            printf("This is mutable.");
//        }
//        else {
//            NSUInteger count = [container count];
//            for (NSUInteger index = 0; index < count; ++index) {
//                __checkMutable(container[index]);
//            }
//        }
//    }
//    else if ([container isKindOfClass:[NSDictionary class]]) {
//        if ([container isKindOfClass:[NSMutableDictionary class]]) {
//            printf("This is mutable.");
//        }
//        else {
//            NSArray* allKeys = [container allKeys];
//            for (id key in allKeys) {
//                __checkMutable(container[key]);
//            }
//        }
//    }
//}

- (JSValue *)callJSMethod:(NSString *)method args:(NSArray *)args
{
    WXLogDebug(@"Calling JS... method:%@, args:%@", method, args);
//    __checkMutable(args);
    return [[_jsContext globalObject] invokeMethod:method withArguments:args];
}

- (void)registerCallNative:(WXJSCallNative)callNative
{
    JSValue* (^callNativeBlock)(JSValue *, JSValue *, JSValue *) = ^JSValue*(JSValue *instance, JSValue *tasks, JSValue *callback){
        NSString *instanceId = [instance toString];
        NSArray *tasksArray = [tasks toArray];
        NSString *callbackId = [callback toString];
        WXLogDebug(@"Calling native... instance:%@, tasks:%@, callback:%@", instanceId, tasksArray, callbackId);
        return [JSValue valueWithInt32:(int32_t)callNative(instanceId, tasksArray, callbackId) inContext:[JSContext currentContext]];
    };
    
    _jsContext[@"callNative"] = callNativeBlock;
}

- (void)executeJavascript:(NSString *)script
{
    WXAssertParam(script);
    [_jsContext evaluateScript:script];
}

- (JSValue*)executeJavascript:(NSString *)script withSourceURL:(NSURL*)sourceURL
{
    WXAssertParam(script);

    if (sourceURL) {
        return [_jsContext evaluateScript:script withSourceURL:sourceURL];
    } else {
        return [_jsContext evaluateScript:script];
    }
}

- (void)registerCallUpdateComponentData:(WXJSCallUpdateComponentData)callUpdateComponentData;
{
    id callUpdateComponentDataBlock = ^(JSValue *instanceId, JSValue *cid, JSValue *data, JSValue *ifCallback) {
        NSString *instanceIdString = [instanceId toString];
        NSString *componentId = [cid toString];
        NSDictionary* jsonData = [data toDictionary];
        NSString* dataString = [WXUtility JSONString:jsonData];
        WXLogDebug(@"CallUpdateComponentData...%@, %@, %@", instanceIdString, componentId, jsonData);

        return [JSValue valueWithInt32:(int32_t)callUpdateComponentData(instanceIdString, componentId, dataString) inContext:[JSContext currentContext]];
    };

    _jsContext[@"__updateComponentData"] = callUpdateComponentDataBlock;
}

- (void)registerCallAddElement:(WXJSCallAddElement)callAddElement
{
    id callAddElementBlock = ^(JSValue *instanceId, JSValue *ref, JSValue *element, JSValue *index, JSValue *ifCallback) {
        
        NSString *instanceIdString = [instanceId toString];
        NSDictionary *componentData = [element toDictionary];
        NSString *parentRef = [ref toString];
        NSInteger insertIndex = [[index toNumber] integerValue];
        if (WXAnalyzerCenter.isInteractionLogOpen) {
            WXLogDebug(@"wxInteractionAnalyzer : [jsengin][addElementStart],%@,%@",instanceIdString,componentData[@"ref"]);
        }
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
        
        WXLogDebug(@"callMoveElement...%@, %@,", instanceIdString, refString);
        return [JSValue valueWithInt32:(int32_t)callMoveElement(instanceIdString, refString, parentRefString, moveIndex) inContext:[JSContext currentContext]];
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
        
        WXLogDebug(@"callUpdateStyle...%@, %@, %@", instanceIdString, refString, stylessData);
        return [JSValue valueWithInt32:(int32_t)callUpdateStyle(instanceIdString, refString,stylessData) inContext:[JSContext currentContext]];
    };
    
    _jsContext[@"callUpdateStyle"] = WXJSCallUpdateStyleBlock;
}

- (void)registerCallAddEvent:(WXJSCallAddEvent)callAddEvent
{
    id WXJSCallAddEventBlock = ^(JSValue *instanceId, JSValue *ref,JSValue *event, JSValue *ifCallback) {
        
        NSString *instanceIdString = [instanceId toString];
        NSString *refString = [ref toString];
        NSString *eventString = [event toString];
        
        WXLogDebug(@"callAddEvent...%@, %@, %@", instanceIdString, refString, eventString);
        return [JSValue valueWithInt32:(int32_t)callAddEvent(instanceIdString, refString,eventString) inContext:[JSContext currentContext]];
    };
    
    _jsContext[@"callAddEvent"] = WXJSCallAddEventBlock;
}

- (void)registerCallRemoveEvent:(WXJSCallRemoveEvent)callRemoveEvent
{
    id WXJSCallRemoveEventBlock = ^(JSValue *instanceId, JSValue *ref,JSValue *event, JSValue *ifCallback) {
        
        NSString *instanceIdString = [instanceId toString];
        NSString *refString = [ref toString];
        NSString *eventString = [event toString];
        
        WXLogDebug(@"callRemoveEvent...%@, %@, %@", instanceIdString, refString,eventString);
        return [JSValue valueWithInt32:(int32_t)callRemoveEvent(instanceIdString, refString,eventString) inContext:[JSContext currentContext]];
    };
    
    _jsContext[@"callRemoveEvent"] = WXJSCallRemoveEventBlock;
}
    
- (void)registerCallCreateFinish:(WXJSCallCreateFinish)callCreateFinish
{
    id WXJSCallCreateFinishBlock = ^(JSValue *instanceId, JSValue *ifCallback) {
        NSString *instanceIdString = [instanceId toString];
        WXLogDebug(@"callCreateFinish...%@", instanceIdString);
        return [JSValue valueWithInt32:(int32_t)callCreateFinish(instanceIdString) inContext:[JSContext currentContext]];
    };
        
    _jsContext[@"callCreateFinish"] = WXJSCallCreateFinishBlock;
}

- (void)registerCallRefreshFinish:(WXJSCallRefreshFinish)callRefreshFinish
{
    id WXJSCallCreateFinishBlock = ^(JSValue *instanceId, JSValue *ifCallback) {
        NSString *instanceIdString = [instanceId toString];
        WXLogDebug(@"callRefreshFinish...%@", instanceIdString);
        return [JSValue valueWithInt32:(int32_t)callRefreshFinish(instanceIdString) inContext:[JSContext currentContext]];
    };
    
    _jsContext[@"callRefreshFinish"] = WXJSCallCreateFinishBlock;
}

- (void)registerCallUpdateFinish:(WXJSCallUpdateFinish)callUpdateFinish
{
    id WXJSCallCreateFinishBlock = ^(JSValue *instanceId, JSValue *ifCallback) {
        NSString *instanceIdString = [instanceId toString];
        WXLogDebug(@"callUpdateFinish...%@", instanceIdString);
        return [JSValue valueWithInt32:(int32_t)callUpdateFinish(instanceIdString) inContext:[JSContext currentContext]];
    };
    
    _jsContext[@"callUpdateFinish"] = WXJSCallCreateFinishBlock;
}

- (void)registerCallNativeModule:(WXJSCallNativeModule)callNativeModuleBlock
{
    _jsContext[@"callNativeModule"] = ^JSValue *(JSValue *instanceId, JSValue *moduleName, JSValue *methodName, JSValue *args, JSValue *options) {
        NSString *instanceIdString = [instanceId toString];
        NSString *moduleNameString = [moduleName toString];
        NSString *methodNameString = [methodName toString];
        NSArray *argsArray = [args toArray];
        NSDictionary *optionsDic = [options toDictionary];
        
        WXLogDebug(@"callNativeModule...%@,%@,%@,%@", instanceIdString, moduleNameString, methodNameString, argsArray);
        
        id result = nil;
        if ([WXCustomPageBridge isCustomPage:instanceIdString]) {
            if ([[WXCustomPageBridge sharedInstance] forwardCallNativeModuleToCustomPage:instanceIdString moduleName:moduleNameString methodName:methodNameString arguments:argsArray options:optionsDic returnValue:&result]) {
                return [JSValue valueWithObject:result inContext:[JSContext currentContext]];
            }
        }
        
        NSInvocation *invocation = callNativeModuleBlock(instanceIdString, moduleNameString, methodNameString, argsArray, optionsDic);
        JSValue *returnValue = [JSValue wx_valueWithReturnValueFromInvocation:invocation inContext:[JSContext currentContext]];
        return returnValue;
    };
}

- (void)registerCallNativeComponent:(WXJSCallNativeComponent)callNativeComponentBlock
{
    _jsContext[@"callNativeComponent"] = ^void(JSValue *instanceId, JSValue *componentName, JSValue *methodName, JSValue *args, JSValue *options) {
        NSString *instanceIdString = [instanceId toString];
        NSString *componentNameString = [componentName toString];
        NSString *methodNameString = [methodName toString];
        NSArray *argsArray = [args toArray];
        NSDictionary *optionsDic = [options toDictionary];
        
        WXLogDebug(@"callNativeComponent...%@,%@,%@,%@", instanceIdString, componentNameString, methodNameString, argsArray);
        
        if ([WXCustomPageBridge isCustomPage:instanceIdString]) {
            [[WXCustomPageBridge sharedInstance] forwardCallComponentToCustomPage:instanceIdString ref:componentNameString methodName:methodNameString arguments:argsArray options:optionsDic];
            return;
        }
        
        callNativeComponentBlock(instanceIdString, componentNameString, methodNameString, argsArray, optionsDic);
    };
}

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

- (void)createDefaultContext
{
    __weak typeof(self) weakSelf = self;
    
    _jsContext = [[JSContext alloc] init];
    _jsContext.name = @"Weex Context";
    
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
}

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

@end
