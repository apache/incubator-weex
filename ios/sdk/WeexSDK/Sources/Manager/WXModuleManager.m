/**
 * Created by Weex.
 * Copyright (c) 2016, Alibaba, Inc. All rights reserved.
 *
 * This source code is licensed under the Apache Licence 2.0.
 * For the full copyright and license information,please view the LICENSE file in the root directory of this source tree.
 */

#import "WXModuleManager.h"
#import "WXModuleProtocol.h"
#import "WXUtility.h"
#import "WXModuleFactory.h"
#import "WXSDKError.h"
#import "WXMonitor.h"
#import "WXSDKManager.h"
#import "WXThreadSafeMutableDictionary.h"
#import "WXRuntimeConfig.h"

#import <objc/message.h>

@interface WXModuleManager ()

@property (nonatomic, strong) WXThreadSafeMutableDictionary *indexDict;

@end

@implementation WXModuleManager

- (instancetype)init
{
    self = [super init];
    if(self){
        _indexDict = [[WXThreadSafeMutableDictionary alloc]init];
    }
    return self;
}

#pragma mark Private Methods

- (void)_executeModuleThead:(NSThread *)thread withBlock:(dispatch_block_t)block
{
    if (!thread || !block) return;
    
    if ([NSThread currentThread] == thread) {
        block();
    } else {
        [self performSelector:@selector(_executeModuleBlock:)
                     onThread:thread
                   withObject:[block copy]
                waitUntilDone:NO];
    }
}

- (void)_executeModuleBlock:(dispatch_block_t)block
{
    if (block) block();
}

- (void)_executeModuleMethod:(id)module withMethod:(WXBridgeMethod *)method
{
    NSInvocation *invocation = [[WXRuntimeConfig sharedInstance] invocationWithTargetMethod:module method:method];
    [invocation invoke];
}

#pragma mark Public Methods

- (void)dispatchMethod:(WXBridgeMethod *)method
{
    if (!method) return;
    
    Class module =  [WXModuleFactory classWithModuleName:method.module];
    if (!module) {
        NSString *errorMessage = [NSString stringWithFormat:@"Module：%@ doesn't exist！", method.module];
        WX_MONITOR_FAIL(WXMTJSBridge, WX_ERR_INVOKE_NATIVE, errorMessage);
        return;
    }
    
//    WXLogDebug(@"execute：[module %@], [method %@], [parameter %@]", method.module, method.method, method.arguments);
    
    WXSDKInstance *weexInstance = [WXSDKManager instanceForID:method.instance];
    id<WXModuleProtocol> moduleInstance = [weexInstance moduleForClass:module];
    
    // attach target thread
    NSThread *targetThread = nil;
    if([moduleInstance respondsToSelector:@selector(targetExecuteThread)]){
        targetThread = [moduleInstance targetExecuteThread];
    } else {
        targetThread = [NSThread mainThread];
    }
    
    //execute module method
    __weak typeof(self) weakSelf = self;
    dispatch_block_t dipatchMethodBlock = ^ (){
        [weakSelf _executeModuleMethod:moduleInstance withMethod:method];
    };
    [self _executeModuleThead:targetThread withBlock:dipatchMethodBlock];
}

@end
