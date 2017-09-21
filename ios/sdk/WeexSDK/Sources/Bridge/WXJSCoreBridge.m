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
#import "WXMonitor.h"
#import "WXAppMonitorProtocol.h"
#import "WXHandlerFactory.h"
#import <sys/utsname.h>
#import <JavaScriptCore/JavaScriptCore.h>
#import "WXPolyfillSet.h"
#import "JSValue+Weex.h"
#import "WXJSExceptionProtocol.h"
#import "WXSDKManager.h"
#import "WXExtendCallNativeManager.h"
#import "WXTracingManager.h"

#import <dlfcn.h>

#import <mach/mach.h>


@interface WXJSCoreBridge ()

@property (nonatomic, strong)  JSContext *jsContext;
@property (nonatomic, strong)  NSMutableArray *timers;
@property (nonatomic, strong)  NSMutableDictionary *intervaltimers;
@property (nonatomic)  long long intervalTimerId;
@property (nonatomic, strong)  NSMutableDictionary *callbacks;

@end

@implementation WXJSCoreBridge

- (instancetype)init
{
    self = [super init];
    
    if(self){
        _jsContext = [[JSContext alloc] init];
        if (WX_SYS_VERSION_GREATER_THAN_OR_EQUAL_TO(@"8.0")) {
            _jsContext.name = @"Weex Context";
        }
        _timers = [NSMutableArray new];
        _callbacks = [NSMutableDictionary new];
        _intervalTimerId = 0;
        _intervaltimers = [NSMutableDictionary new];

        __weak typeof(self) weakSelf = self;
        
        NSDictionary *data = [WXUtility getEnvironment];
        _jsContext[@"WXEnvironment"] = data;
        
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
        
        _jsContext[@"btoa"] = ^(JSValue *value ) {
            NSData *nsdata = [[value toString]
                              dataUsingEncoding:NSUTF8StringEncoding];
            NSString *base64Encoded = [nsdata base64EncodedStringWithOptions:0];
            return base64Encoded;
        };
        _jsContext[@"atob"] = ^(JSValue *value ) {
            NSData *nsdataFromBase64String = [[NSData alloc]
                                              initWithBase64EncodedString:[value toString] options:0];
            NSString *base64Decoded = [[NSString alloc]
                                       initWithData:nsdataFromBase64String encoding:NSUTF8StringEncoding];
            return base64Decoded;
        };
        _jsContext[@"extendCallNative"] = ^(JSValue *value ) {
            return [weakSelf extendCallNative:[value toDictionary]];
        };
        
        _jsContext[@"nativeLog"] = ^() {
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
            NSMutableString *string = [NSMutableString string];
            [string appendString:@"jsLog: "];
            NSArray *args = [JSContext currentArguments];
            
            [args enumerateObjectsUsingBlock:^(JSValue *jsVal, NSUInteger idx, BOOL *stop) {
                if (idx == args.count - 1) {
                    NSNumber *flag = levelMap[[jsVal toString]];
                    if (flag) {
                        if ([flag isEqualToNumber:[NSNumber numberWithInteger:WXLogFlagWarning]]) {
                            id<WXAppMonitorProtocol> appMonitorHandler = [WXHandlerFactory handlerForProtocol:@protocol(WXAppMonitorProtocol)];
                            if ([appMonitorHandler respondsToSelector:@selector(commitAppMonitorAlarm:monitorPoint:success:errorCode:errorMsg:arg:)]) {
                                [appMonitorHandler commitAppMonitorAlarm:@"weex" monitorPoint:@"jswarning" success:FALSE errorCode:@"99999" errorMsg:string arg:[WXSDKEngine topInstance].pageName];
                            }
                        }
                        WX_LOG([flag unsignedIntegerValue], @"%@", string);
                    } else {
                        [string appendFormat:@"%@ ", jsVal];
                        WXLogInfo(@"%@", string);
                    }
                }
                [string appendFormat:@"%@ ", jsVal ];
            }];
        };
        
        _jsContext.exceptionHandler = ^(JSContext *context, JSValue *exception){
            context.exception = exception;
            NSString *message = [NSString stringWithFormat:@"[%@:%@:%@] %@\n%@", exception[@"sourceURL"], exception[@"line"], exception[@"column"], exception, [exception[@"stack"] toObject]];
            id<WXJSExceptionProtocol> jsExceptionHandler = [WXHandlerFactory handlerForProtocol:@protocol(WXJSExceptionProtocol)];
            
            WXSDKInstance *instance = [WXSDKEngine topInstance];
            WXJSExceptionInfo * jsExceptionInfo = [[WXJSExceptionInfo alloc] initWithInstanceId:instance.instanceId bundleUrl:[instance.scriptURL absoluteString] errorCode:[NSString stringWithFormat:@"%d", WX_ERR_JS_EXECUTE] functionName:@"" exception:[NSString stringWithFormat:@"[%@:%@] %@\n%@",exception[@"line"], exception[@"column"],[exception toString], exception[@"stack"]] userInfo:nil];
            if ([jsExceptionHandler respondsToSelector:@selector(onJSException:)]) {
                [jsExceptionHandler onJSException:jsExceptionInfo];
            }
            WX_MONITOR_FAIL(WXMTJSBridge, WX_ERR_JS_EXECUTE, message);
            if (instance.onJSRuntimeException) {
                instance.onJSRuntimeException(jsExceptionInfo);
            }
        };
        
        if (WX_SYS_VERSION_LESS_THAN(@"8.0")) {
            // solve iOS7 memory problem
            _jsContext[@"nativeSet"] = [WXPolyfillSet class];
        }
    }
    return self;
}

#pragma mark - WXBridgeProtocol

- (void)executeJSFramework:(NSString *)frameworkScript
{
    WXAssertParam(frameworkScript);
    if (WX_SYS_VERSION_GREATER_THAN_OR_EQUAL_TO(@"8.0")) {
        [_jsContext evaluateScript:frameworkScript withSourceURL:[NSURL URLWithString:@"native-bundle-main.js"]];
    }else{
        [_jsContext evaluateScript:frameworkScript];
    }
}

- (JSValue *)callJSMethod:(NSString *)method args:(NSArray *)args
{
    WXLogDebug(@"Calling JS... method:%@, args:%@", method, args);
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
    id WXJSCallAddEventBlock = ^(JSValue *instanceId, JSValue *ref,JSValue *event, JSValue *ifCallback) {
        
        NSString *instanceIdString = [instanceId toString];
        NSString *refString = [ref toString];
        NSString *eventString = [event toString];
        
        WXLogDebug(@"callAddEvent...%@, %@, %@", instanceIdString, refString,eventString);
        [WXTracingManager startTracingWithInstanceId:instanceIdString ref:refString className:nil name:WXTJSCall phase:WXTracingBegin functionName:@"addEvent" options:nil];
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
        [WXTracingManager startTracingWithInstanceId:instanceIdString ref:refString className:nil name:WXTJSCall phase:WXTracingBegin functionName:@"removeEvent" options:nil];
        return [JSValue valueWithInt32:(int32_t)callRemoveEvent(instanceIdString, refString,eventString) inContext:[JSContext currentContext]];
    };
    
    _jsContext[@"callRemoveEvent"] = WXJSCallRemoveEventBlock;
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

- (void)triggerTimeout:(void(^)())block
{
    block();
}

- (void)callBack:(NSDictionary *)dic
{
    if([dic objectForKey:@"ret"] && [_timers containsObject:[dic objectForKey:@"ret"]]) {
        [[WXSDKManager bridgeMgr] callBack:[dic objectForKey:@"appId"] funcId:[dic objectForKey:@"ret"]  params:[dic objectForKey:@"arg"] keepAlive:NO];
    }

}


- (void)callBackInterval:(NSDictionary *)dic functon:(void(^)())block
{
    if([dic objectForKey:@"appId"] && [_intervaltimers objectForKey:[dic objectForKey:@"appId"]]){
        NSMutableArray *timers = [_intervaltimers objectForKey:[dic objectForKey:@"appId"]];
        if([timers containsObject:[dic objectForKey:@"timerId"]]){
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
    __weak typeof(self) weakSelf = self;
    dispatch_time_t time = dispatch_time(DISPATCH_TIME_NOW, interval*NSEC_PER_SEC);
    dispatch_after(time, dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
        NSMutableDictionary *dic = [NSMutableDictionary new];
        [dic setObject:appId forKey:@"appId"];
        [dic setObject:ret forKey:@"ret"];
        [dic setObject:arg forKey:@"arg"];
        [weakSelf performSelector:@selector(callBack:) withObject:dic ];
    });
}

- (long long)triggerInterval:(NSString *)appId function:(void(^)())block arg:(NSString *)arg
{
    double interval = [arg doubleValue]/1000.0f;
    long long timerId = _intervalTimerId + 1;
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

-(void)executeInterval:(NSString *)appId function:(void(^)())block arg:(NSString *)arg timerId:(long long)timerId
{
    double interval = [arg doubleValue]/1000.0f;
    dispatch_time_t time = dispatch_time(DISPATCH_TIME_NOW, interval*NSEC_PER_SEC);
    __weak typeof(self) weakSelf = self;
    dispatch_after(time, dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
        NSMutableDictionary *dic = [NSMutableDictionary new];
        [dic setObject:appId forKey:@"appId"];
        [dic setObject:arg forKey:@"arg"];
        [dic setObject:@(timerId) forKey:@"timerId"];
        [weakSelf performSelector:@selector(callBackInterval:functon:) withObject:dic withObject:block];
    });
}

- (void)triggerClearInterval:(NSString *)appId ret:(long long)ret
{
    if(_intervaltimers && [_intervaltimers objectForKey:@"appId"]){
        NSMutableArray *timers = [_intervaltimers objectForKey:@"appId"];
        if(timers && [timers containsObject:@(ret)]){
            [timers removeObject:@(ret)];
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
