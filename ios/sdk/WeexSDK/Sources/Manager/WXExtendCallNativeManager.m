//
//  WXExtendCallNativeManager.m
//  WeexSDK
//
//  Created by 齐山 on 2017/7/27.
//  Copyright © 2017年 taobao. All rights reserved.
//

#import "WXExtendCallNativeManager.h"
#import "WXExtendCallNativeProtocol.h"
#import <objc/runtime.h>

@implementation WXExtendCallNativeManager

+(id)sendExtendCallNativeEvent:(NSDictionary *)parameters
{
    if(parameters[@"className"]){
        Class<WXExtendCallNativeProtocol> receiveClass = NSClassFromString(parameters[@"className"]);
        if(class_conformsToProtocol(receiveClass,@protocol(WXExtendCallNativeProtocol))){
            BOOL receivedItem = [receiveClass checkParameters:parameters];
            if(receivedItem){
                return [receiveClass excuteCallNative:parameters];
            }
        }
    }
    return @(-1);
}
@end
