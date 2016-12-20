//
//  WXCallJSMethod.m
//  WeexSDK
//
//  Created by yinfeng on 2016/12/20.
//  Copyright © 2016年 taobao. All rights reserved.
//

#import "WXCallJSMethod.h"

@implementation WXCallJSMethod
{
    NSString *_moduleName;
}

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

- (NSDictionary *)callJSTask
{
    return @{@"module":_moduleName ?: @"",
             @"method":self.methodName ?: @"",
             @"args":self.arguments ?: @[]};
}

@end
