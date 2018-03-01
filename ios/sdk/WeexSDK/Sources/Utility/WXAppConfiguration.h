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

@interface WXAppConfiguration : NSObject

/**
 * @abstract Group or organization of your app, default value is nil.
 */
+ (NSString *)appGroup;
+ (void)setAppGroup:(NSString *) appGroup;

/**
 * @abstract Name of your app, default is value for CFBundleDisplayName in main bundle.
 */
+ (NSString *)appName;
+ (void)setAppName:(NSString *)appName;

/**
 * @abstract Version of your app, default is value for CFBundleShortVersionString in main bundle.
 */
+ (NSString *)appVersion;
+ (void)setAppVersion:(NSString *)appVersion;

/**
 * @abstract External user agent of your app, all requests sent by weex will set the user agent on header,  default value is nil.
 */
+ (NSString *)externalUserAgent;
+ (void)setExternalUserAgent:(NSString *)userAgent;

/**
 * @abstract JSFrameworkVersion
 */
+ (NSString *)JSFrameworkVersion;
+ (void)setJSFrameworkVersion:(NSString *)JSFrameworkVersion;

/**
 + * @abstract JSFrameworkLibSize
 + */
+ (NSUInteger)JSFrameworkLibSize;
+ (void)setJSFrameworkLibSize:(NSUInteger)JSFrameworkLibSize;

/*
 *  @abstract customizeProtocolClasses
 */
+ (NSArray*)customizeProtocolClasses;
+ (void)setCustomizeProtocolClasses:(NSArray*)customizeProtocolClasses;

@end
