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

NS_ASSUME_NONNULL_BEGIN

@interface WXJSExceptionInfo : NSObject
/**
 * instance id
 */
@property(nonatomic,strong) NSString * instanceId;
/**
 * the URL where the exception occurred
 */
@property(nonatomic,strong) NSString * bundleUrl;
/**
 * error code
 */
@property(nonatomic,strong) NSString * errorCode;
/**
 * the function name of exception
 */
@property(nonatomic,strong) NSString * functionName;
/**
 * exception detail;
 */
@property(nonatomic,strong) NSString * exception;
/**
 * extend filed
 */
@property(nonatomic,strong) NSMutableDictionary * userInfo;
/**
 * weex sdk version
 */
@property(nonatomic,strong, readonly) NSString * sdkVersion;
/**
 * js framework verison
 */
@property(nonatomic,strong, readonly) NSString * jsfmVersion;


/**
 * @abstract Initializes a WXJSException instance
 * @param instanceId the id of instance
 * @param bundleUrl the page URL where the exception occurred
 * @param errorCode error Code
 * @param exception exception detail
 * @param userInfo  extend field
 */
- (instancetype)initWithInstanceId:(NSString *)instanceId
                         bundleUrl:(NSString *)bundleUrl
                         errorCode:(NSString *)errorCode
                      functionName:(NSString *)functionName
                         exception:(NSString * _Nullable)exception
                          userInfo:(NSMutableDictionary * _Nullable)userInfo;

@end

NS_ASSUME_NONNULL_END
