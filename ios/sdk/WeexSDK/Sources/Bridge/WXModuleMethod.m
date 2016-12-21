//
//  WXModuleMethod.m
//  WeexSDK
//
//  Created by yinfeng on 2016/12/16.
//  Copyright © 2016年 taobao. All rights reserved.
//

#import "WXModuleMethod.h"
#import "WXModuleFactory.h"
#import "WXMonitor.h"
#import "WXModuleProtocol.h"
#import "WXAssert.h"
#import "WXUtility.h"

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
    Class moduleClass =  [WXModuleFactory classWithModuleName:_moduleName];
    if (!moduleClass) {
        NSString *errorMessage = [NSString stringWithFormat:@"Module：%@ doesn't exist, maybe it has not been registered", _moduleName];
        WX_MONITOR_FAIL(WXMTJSBridge, WX_ERR_INVOKE_NATIVE, errorMessage);
        return nil;
    }
    
    id<WXModuleProtocol> moduleInstance = [self.instance moduleForClass:moduleClass];
    WXAssert(moduleInstance, @"No instance found for module name:%@, class:%@", _moduleName, moduleClass);
    BOOL isSync;
    SEL selector = [WXModuleFactory selectorWithModuleName:self.moduleName methodName:self.methodName isSync:&isSync];
    if (!selector) {
        NSString *errorMessage = [NSString stringWithFormat:@"method：%@ for module:%@ doesn't exist, maybe it has not been registered", self.methodName, _moduleName];
        WX_MONITOR_FAIL(WXMTJSBridge, WX_ERR_INVOKE_NATIVE, errorMessage);
        return nil;;
    }

    NSInvocation *invocation = [self invocationWithTarget:moduleInstance selector:selector];
    
    if (isSync) {
        [invocation invoke];
        return invocation;
    } else {
        [self _dispatchInvovation:invocation moduleInstance:moduleInstance];
        return nil;
    }
}

- (void)_dispatchInvovation:(NSInvocation *)invocation moduleInstance:(id<WXModuleProtocol>)moduleInstance
{
    // dispatch to user specified queue or thread, default is main thread
    dispatch_block_t dipatchBlock = ^ (){
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
        dispatch_async(targetQueue, dipatchBlock);
    } else {
        WXPerformBlockOnThread(^{
            dipatchBlock();
        }, targetThread);
    }
}

@end
