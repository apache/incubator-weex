//
//  WXModuleMethod.h
//  WeexSDK
//
//  Created by yinfeng on 2016/12/16.
//  Copyright © 2016年 taobao. All rights reserved.
//

#import "WXBridgeMethod.h"

typedef enum : NSUInteger {
    WXModuleMethodTypeSync,
    WXModuleMethodTypeAsync,
} WXModuleMethodType;

@interface WXModuleMethod : WXBridgeMethod

@property (nonatomic, assign) WXModuleMethodType methodType;
@property (nonatomic, strong, readonly) NSString *moduleName;

- (instancetype)initWithModuleName:(NSString *)moduleName
                        methodName:(NSString *)methodName
                         arguments:(NSArray *)arguments
                          instance:(WXSDKInstance *)instance;

- (NSInvocation *)invoke;

@end
