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

#import "WXModuleMethod.h"
#import "WXModuleFactory.h"
#import "WXMonitor.h"
#import "WXModuleProtocol.h"
#import "WXAssert.h"
#import "WXUtility.h"
#import "WXSDKInstance_private.h"
#import "WXHandlerFactory.h"
#import "WXValidateProtocol.h"

@implementation WXModuleMethod

- (instancetype)initWithModuleName:(NSString *)moduleName
                        methodName:(NSString *)methodName
                         arguments:(NSArray *)arguments
                          instance:(WXSDKInstance *)instance
{
    if (self = [super initWithMethodName:methodName arguments:arguments instance:instance]) {
        _moduleName = moduleName;
    }
    
    return self;
}

- (NSInvocation *)invoke
{
    
    if (self.instance.needValidate) {
        id<WXValidateProtocol> validateHandler = [WXHandlerFactory handlerForProtocol:@protocol(WXValidateProtocol)];
        if (validateHandler) {
            WXModuleValidateResult* result =  [validateHandler validateWithWXSDKInstance:self.instance module:self.moduleName method:self.methodName args:self.arguments];
            if (result && !result.isSuccess) {
                NSString *errorMessage = [result.error.userInfo  objectForKey:@"errorMsg"];
                WXLogError(@"%@", errorMessage);
                WX_MONITOR_FAIL(WXMTJSBridge, WX_ERR_INVOKE_NATIVE, errorMessage);
                if ([result.error respondsToSelector:@selector(userInfo)]) {
                    NSInvocation *invocation = [self invocationWithTarget:result.error selector:@selector(userInfo)];
                    [invocation invoke];
                    return invocation;
                }else{
                    return nil;
                }
            }
        }
    }
    
    Class moduleClass =  [WXModuleFactory classWithModuleName:_moduleName];
    if (!moduleClass) {
        NSString *errorMessage = [NSString stringWithFormat:@"Module：%@ doesn't exist, maybe it has not been registered", _moduleName];
        WX_MONITOR_FAIL(WXMTJSBridge, WX_ERR_INVOKE_NATIVE, errorMessage);
        return nil;
    }
    
    id<WXModuleProtocol> moduleInstance = [self.instance moduleForClass:moduleClass];
    WXAssert(moduleInstance, @"No instance found for module name:%@, class:%@", _moduleName, moduleClass);
    BOOL isSync = NO;
    SEL selector = [WXModuleFactory selectorWithModuleName:self.moduleName methodName:self.methodName isSync:&isSync];
   
    if (![moduleInstance respondsToSelector:selector]) {
        // if not implement the selector, then dispatch default module method
        if ([self.methodName isEqualToString:@"addEventListener"]) {
            [self.instance _addModuleEventObserversWithModuleMethod:self];
        } else if ([self.methodName isEqualToString:@"removeAllEventListeners"]) {
            [self.instance _removeModuleEventObserverWithModuleMethod:self];
        } else {
            NSString *errorMessage = [NSString stringWithFormat:@"method：%@ for module:%@ doesn't exist, maybe it has not been registered", self.methodName, _moduleName];
            WX_MONITOR_FAIL(WXMTJSBridge, WX_ERR_INVOKE_NATIVE, errorMessage);
        }
        return nil;
    }
    
    NSInvocation *invocation = [self invocationWithTarget:moduleInstance selector:selector];
    
    if (isSync) {
        [invocation invoke];
        return invocation;
    } else {
        [self _dispatchInvocation:invocation moduleInstance:moduleInstance];
        return nil;
    }
}

- (void)_dispatchInvocation:(NSInvocation *)invocation moduleInstance:(id<WXModuleProtocol>)moduleInstance
{
    // dispatch to user specified queue or thread, default is main thread
    dispatch_block_t dispatchBlock = ^ (){
        [invocation invoke];
    };
    
    NSThread *targetThread = nil;
    dispatch_queue_t targetQueue = nil;
    if([moduleInstance respondsToSelector:@selector(targetExecuteQueue)]){
        targetQueue = [moduleInstance targetExecuteQueue] ?: dispatch_get_main_queue();
    } else if([moduleInstance respondsToSelector:@selector(targetExecuteThread)]){
        targetThread = [moduleInstance targetExecuteThread] ?: [NSThread mainThread];
    } else {
        targetThread = [NSThread mainThread];
    }

    WXAssert(targetQueue || targetThread, @"No queue or thread found for module:%@", moduleInstance);
    
    if (targetQueue) {
        dispatch_async(targetQueue, dispatchBlock);
    } else {
        WXPerformBlockOnThread(^{
            dispatchBlock();
        }, targetThread);
    }
}

@end
