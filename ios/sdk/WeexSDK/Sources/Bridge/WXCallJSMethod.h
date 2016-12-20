//
//  WXCallJSMethod.h
//  WeexSDK
//
//  Created by yinfeng on 2016/12/20.
//  Copyright © 2016年 taobao. All rights reserved.
//

#import "WXBridgeMethod.h"

@interface WXCallJSMethod : WXBridgeMethod

- (instancetype)initWithModuleName:(NSString *)moduleName
                        methodName:(NSString *)methodName
                         arguments:(NSArray *)arguments
                          instance:(WXSDKInstance *)instance;

- (NSDictionary *)callJSTask;

@end
