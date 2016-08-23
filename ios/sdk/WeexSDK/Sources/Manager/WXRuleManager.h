//
//  WXRuleManager.h
//  WeexSDK
//
//  Created by zifan.zx on 8/18/16.
//  Copyright © 2016 taobao. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "WXThreadSafeMutableDictionary.h"

@interface WXRuleManager : NSObject

+ (WXRuleManager *)sharedInstance;
- (void)addRule:(NSString *)type rule:(NSDictionary *)rule;
- (WXThreadSafeMutableDictionary *)getRule:(NSString *)type;
@end
