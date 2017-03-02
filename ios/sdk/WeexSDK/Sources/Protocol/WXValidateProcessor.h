/**
 * Created by Weex.
 * Copyright (c) 2016, Alibaba, Inc. All rights reserved.
 *
 * This source code is licensed under the Apache Licence 2.0.
 * For the full copyright and license information,please view the LICENSE file in the root directory of this source tree.
 */

#import <Foundation/Foundation.h>
#import "WXSDKInstance.h"

@interface WXComponentValidateResult : NSObject

@property(nonatomic,assign)BOOL            isSuccess;
@property(nonatomic,copy)NSString*         replacedComponent;

@end


@protocol WXValidateProcessor <NSObject>

-(BOOL)needValidate:(NSURL*) bundleUrl;

-(BOOL)validateWithWXSDKInstance:(WXSDKInstance *)wxsdkInstance module:(NSString*) moduel method:(NSString *)method;

-(WXComponentValidateResult *)validateWithWXSDKInstance:(WXSDKInstance *)wxsdkInstance component:(NSString *)componentName;

@end
