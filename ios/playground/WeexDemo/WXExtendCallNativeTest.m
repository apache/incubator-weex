//
//  WXExtendCallNativeTest.m
//  WeexDemo
//
//  Created by 齐山 on 2017/7/27.
//  Copyright © 2017年 taobao. All rights reserved.
//

#import "WXExtendCallNativeTest.h"

@implementation WXExtendCallNativeTest

#pragma mark -
#pragma WXExtendCallNativeProtocol

+ (BOOL)checkParameters:(NSDictionary *)parameters
{
    if(!parameters || ![parameters isKindOfClass:[NSDictionary class]]){
        return NO;
    }
    
    if(!parameters[@"className"]){
        return NO;
    }
    
    return YES;
}


+ (id)excuteCallNative:(NSDictionary *)parameters
{
    NSLog(@"weex test");
    return @{@"value":@"test"};
}

@end
