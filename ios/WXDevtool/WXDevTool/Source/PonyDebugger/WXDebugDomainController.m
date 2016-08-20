/**
 * Created by Weex.
 * Copyright (c) 2016, Alibaba, Inc. All rights reserved.
 *
 * This source code is licensed under the Apache Licence 2.0.
 * For the full copyright and license information,please view the LICENSE file in the root directory of this source tree.
 */

#import "WXDebugDomainController.h"
#import "WXDevToolType.h"
#import <WeexSDK/WXLog.h>
#import <WeexSDK/WXSDKEngine.h>

@implementation WXDebugDomainController
@dynamic domain;

+ (WXDebugDomainController *)defaultInstance {
    static WXDebugDomainController *defaultInstance = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        defaultInstance = [[WXDebugDomainController alloc] init];
    });
    return defaultInstance;
}

+ (Class)domainClass {
    return [WXDebugDomain class];
}

+ (NSDictionary *)getLogLevelMap {
    NSDictionary *logLevelEnumToString =
    @{
      @"all":@(WXLogLevelDebug),
      @"error":@(WXLogLevelError),
      @"warn":@(WXLogLevelWarning),
      @"info":@(WXLogLevelInfo),
      @"log":@(WXLogLevelLog),
      @"debug":@(WXLogLevelDebug),
      @"off":@(WXLogLevelOff)
      };
    return logLevelEnumToString;
}

#pragma mark - PDCommandDelegate
- (void)domain:(PDDynamicDebuggerDomain *)domain enableWithCallback:(void (^)(id error))callback {
    [WXDevToolType setDebug:YES];
    [WXSDKEngine restart];
    [[NSNotificationCenter defaultCenter] postNotificationName:@"RefreshInstance" object:nil];
    callback(nil);
}

- (void)domain:(PDDynamicDebuggerDomain *)domain disableWithCallback:(void (^)(id error))callback {
    [WXDevToolType setDebug:NO];
    [WXSDKEngine restart];
    [[NSNotificationCenter defaultCenter] postNotificationName:@"RefreshInstance" object:nil];
    callback(nil);
}

- (void)domain:(WXDebugDomain *)domain sendLogLevel:(NSString *)level WithCallback:(void (^)(id error))callback {
    NSDictionary *logLevelMap = [WXDebugDomainController getLogLevelMap];
    WXLogLevel wxLogLevel = [[logLevelMap objectForKey:level] integerValue];
    [WXLog setLogLevel:wxLogLevel];
    callback(nil);
}


@end
