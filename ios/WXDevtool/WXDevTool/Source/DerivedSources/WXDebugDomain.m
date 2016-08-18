/**
 * Created by Weex.
 * Copyright (c) 2016, Alibaba, Inc. All rights reserved.
 *
 * This source code is licensed under the Apache Licence 2.0.
 * For the full copyright and license information,please view the LICENSE file in the root directory of this source tree.
 */

#import "WXDebugDomain.h"

@implementation WXDebugDomain

@dynamic delegate;

+ (NSString *)domainName {
    return @"WxDebug";
}

#pragma mark - Public Method
- (void)handleMethodWithName:(NSString *)methodName parameters:(NSDictionary *)params responseCallback:(PDResponseCallback)responseCallback {
    if ([methodName isEqualToString:@"enable"] && [self.delegate respondsToSelector:@selector(domain:enableWithCallback:)]) {
        [self.delegate domain:self enableWithCallback:^(id error) {
            responseCallback(nil, error);
        }];
    } else if ([methodName isEqualToString:@"setLogLevel"] && [self.delegate respondsToSelector:@selector(domain:sendLogLevel:WithCallback:)]) {
        [self.delegate domain:self sendLogLevel:[params objectForKey:@"logLevel"] WithCallback:^(id error) {
            responseCallback(nil,error);
        }];
    } else if ([methodName isEqualToString:@"disable"] && [self.delegate respondsToSelector:@selector(domain:disableWithCallback:)]) {
        [self.delegate domain:self disableWithCallback:^(id error) {
            responseCallback(nil,error);
        }];
    }
}

@end
