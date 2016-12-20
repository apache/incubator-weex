//
//  WXComponentMethod.h
//  WeexSDK
//
//  Created by yinfeng on 2016/12/16.
//  Copyright © 2016年 taobao. All rights reserved.
//

#import "WXBridgeMethod.h"
@class WXComponent;

@interface WXComponentMethod : WXBridgeMethod

- (instancetype)initWithComponentName:(NSString *)componentName
                         componentRef:(NSString *)ref
                           methodName:(NSString *)methodName
                            arguments:(NSArray *)arguments
                             instance:(WXSDKInstance *)instance;

- (void)invoke;

@end
