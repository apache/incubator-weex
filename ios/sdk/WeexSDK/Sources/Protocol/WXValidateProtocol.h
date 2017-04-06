/**
 * Created by Weex.
 * Copyright (c) 2016, Alibaba, Inc. All rights reserved.
 *
 * This source code is licensed under the Apache Licence 2.0.
 * For the full copyright and license information,please view the LICENSE file in the root directory of this source tree.
 */

#import <Foundation/Foundation.h>
#import "WXSDKInstance.h"


@interface WXValidateResult : NSObject

@property(nonatomic,assign)BOOL            isSuccess;
@property(nonatomic,strong)NSError*        error;

@end

@interface WXModuleValidateResult : WXValidateResult

@end

@interface WXComponentValidateResult :WXValidateResult

@property(nonatomic,copy)NSString* replacedComponent;

@end


@protocol WXValidateProtocol <NSObject>

-(BOOL)needValidate:(NSURL*) bundleUrl;

-(WXModuleValidateResult *)validateWithWXSDKInstance:(WXSDKInstance *)wxsdkInstance module:(NSString*) moduel method:(NSString *)method args:(NSArray *)args;

-(WXComponentValidateResult *)validateWithWXSDKInstance:(WXSDKInstance *)wxsdkInstance component:(NSString *)componentName;

@end
