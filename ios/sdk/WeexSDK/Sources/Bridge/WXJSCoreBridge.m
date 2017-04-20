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

#import <dlfcn.h>

#import <mach/mach.h>


@interface WXJSCoreBridge ()

@property (nonatomic, strong)  JSContext *jsContext;
@property (nonatomic, strong)  NSMutableArray *timers;

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
        
        __weak typeof(self) weakSelf = self;
        
        NSDictionary *data = [WXUtility getEnvironment];
        _jsContext[@"WXEnvironment"] = data;
        
        _jsContext[@"setTimeout"] = ^(JSValue *function, JSValue *timeout) {
            // this setTimeout is used by internal logic in JS framework, normal setTimeout called by users will call WXTimerModule's method;
            [weakSelf performSelector: @selector(triggerTimeout:) withObject:^() {
                [function callWithArguments:@[]];
            } afterDelay:[timeout toDouble] / 1000];
        };
        
        _jsContext[@"setTimeoutWeex"] = ^(JSValue *appid, JSValue *ret,JSValue *arg ) {
            [weakSelf triggerTimeout:[appid toString] ret:[ret toString] arg:[arg toString]];
        };
        
        _jsContext[@"setIntervalWeex"] = ^(JSValue *appid, JSValue *ret,JSValue *arg) {
            [weakSelf triggerInterval:[appid toString] ret:[ret toString] arg:[arg toString]];
            
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
            if ([jsExceptionHandler respondsToSelector:@selector(onJSException:)]) {
                WXJSExceptionInfo * jsExceptionInfo = [[WXJSExceptionInfo alloc] initWithInstanceId:instance.instanceId bundleUrl:[instance.scriptURL absoluteString] errorCode:[NSString stringWithFormat:@"%d", WX_ERR_JS_EXECUTE] functionName:@"" exception:[NSString stringWithFormat:@"%@\n%@",[exception toString], exception[@"stack"]] userInfo:nil];
                [jsExceptionHandler onJSException:jsExceptionInfo];
            }
            WX_MONITOR_FAIL(WXMTJSBridge, WX_ERR_JS_EXECUTE, message);
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
        [_jsContext evaluateScript:frameworkScript withSourceURL:[NSURL URLWithString:@"main.js"]];
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
        
         WXLogDebug(@"callAddElement...%@, %@, %@, %ld", instanceIdString, parentRef, componentData, (long)insertIndex);
        
        return [JSValue valueWithInt32:(int32_t)callAddElement(instanceIdString, parentRef, componentData, insertIndex) inContext:[JSContext currentContext]];
    };

    _jsContext[@"callAddElement"] = callAddElementBlock;
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
    if(instance && [_timers containsObject:instance])
    {
        [_timers removeObject:instance];
    }
}

#pragma mark - Private

- (void)triggerTimeout:(void(^)())block
{
    block();
}

- (void)callBack:(NSDictionary *)dic
{
    if([dic objectForKey:@"appid"] && [_timers containsObject:[dic objectForKey:@"appid"]]) {
        [[WXSDKManager bridgeMgr] callBack:[dic objectForKey:@"appid"] funcId:[dic objectForKey:@"ret"]  params:[dic objectForKey:@"arg"] keepAlive:NO];
    }
}


- (void)callBackInterval:(NSDictionary *)dic
{
    if([dic objectForKey:@"appid"] && [_timers containsObject:[dic objectForKey:@"appid"]]) {
        [[WXSDKManager bridgeMgr] callBack:[dic objectForKey:@"appid"] funcId:[dic objectForKey:@"ret"]  params:nil keepAlive:YES];
        [self triggerInterval:[dic objectForKey:@"appid"] ret:[dic objectForKey:@"ret"] arg:[dic objectForKey:@"arg"]];
    }
    
}

- (void)triggerTimeout:(NSString *)appid ret:(NSString *)ret arg:(NSString *)arg
{
    
    double interval = [arg doubleValue]/1000.0f;
    if(WXFloatEqual(interval,0)) {
        return;
    }
    if(![_timers containsObject:appid]){
        [_timers addObject:appid];
    }
    dispatch_time_t time = dispatch_time(DISPATCH_TIME_NOW, interval*NSEC_PER_SEC);
    dispatch_after(time, dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
        NSMutableDictionary *dic = [NSMutableDictionary new];
        [dic setObject:appid forKey:@"appid"];
        [dic setObject:ret forKey:@"ret"];
        [dic setObject:arg forKey:@"arg"];
        [self performSelector:@selector(callBack:) withObject:dic ];
    });
}

- (void)triggerInterval:(NSString *)appid ret:(NSString *)ret arg:(NSString *)arg
{
    double interval = [arg doubleValue]/1000.0f;
    if(WXFloatEqual(interval,0)) {
        return;
    }
    if(![_timers containsObject:appid]){
        [_timers addObject:appid];
    }
    dispatch_time_t time = dispatch_time(DISPATCH_TIME_NOW, interval*NSEC_PER_SEC);
    dispatch_after(time, dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
        NSMutableDictionary *dic = [NSMutableDictionary new];
        [dic setObject:appid forKey:@"appid"];
        [dic setObject:ret forKey:@"ret"];
        [dic setObject:arg forKey:@"arg"];
        [self performSelector:@selector(callBackInterval:) withObject:dic ];
    });
}

@end
