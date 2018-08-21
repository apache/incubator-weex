/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 * 
 *   http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
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

-(WXModuleValidateResult *)validateWithWXSDKInstance:(WXSDKInstance *)wxsdkInstance module:(NSString*) moduel method:(NSString *)method args:(NSArray *)args options:(NSDictionary *)options;

-(WXComponentValidateResult *)validateWithWXSDKInstance:(WXSDKInstance *)wxsdkInstance component:(NSString *)componentName supercomponent:(WXComponent *)supercomponent;

@end
