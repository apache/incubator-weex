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

#import "WXAppConfiguration.h"

@interface WXAppConfiguration ()

@property (nonatomic, strong) NSString * appGroup;
@property (nonatomic, strong) NSString * appName;
@property (nonatomic, strong) NSString * appVersion;
@property (nonatomic, strong) NSString * externalUA;
@property (nonatomic, strong) NSString * JSFrameworkVersion;
@property (nonatomic, assign) NSUInteger JSFrameworkLibSize;
@property (nonatomic, strong) NSArray  * customizeProtocolClasses;
@end

@implementation WXAppConfiguration

+ (instancetype)sharedConfiguration
{
    static id _sharedInstance = nil;
    static dispatch_once_t oncePredicate;
    dispatch_once(&oncePredicate, ^{
        _sharedInstance = [[self alloc] init];
    });
    return _sharedInstance;
}

+ (NSString *)appGroup
{
    return [WXAppConfiguration sharedConfiguration].appGroup;
}

+ (void)setAppGroup:(NSString *)appGroup
{
    [WXAppConfiguration sharedConfiguration].appGroup = appGroup;
}

+ (NSString *)appName
{
    return [WXAppConfiguration sharedConfiguration].appName ?: [[NSBundle mainBundle] objectForInfoDictionaryKey:@"CFBundleDisplayName"];
}

+ (void)setAppName:(NSString *)appName
{
    [WXAppConfiguration sharedConfiguration].appName = appName;
}

+ (NSString *)appVersion
{
    return [WXAppConfiguration sharedConfiguration].appVersion ?: [[NSBundle mainBundle] objectForInfoDictionaryKey:@"CFBundleShortVersionString"];
}

+ (void)setAppVersion:(NSString *)appVersion
{
    [WXAppConfiguration sharedConfiguration].appVersion = appVersion;
}

+ (NSString *)externalUserAgent
{
    return [WXAppConfiguration sharedConfiguration].externalUA;
}

+ (void)setExternalUserAgent:(NSString *)userAgent
{
    [WXAppConfiguration sharedConfiguration].externalUA = userAgent;
}

+ (NSString *)JSFrameworkVersion
{
    return [WXAppConfiguration sharedConfiguration].JSFrameworkVersion ?: @"";
}

+ (void)setJSFrameworkVersion:(NSString *)JSFrameworkVersion
{
    [WXAppConfiguration sharedConfiguration].JSFrameworkVersion = JSFrameworkVersion;
}

+ (NSUInteger)JSFrameworkLibSize
{
    return [WXAppConfiguration sharedConfiguration].JSFrameworkLibSize;
}

+ (void)setJSFrameworkLibSize:(NSUInteger)JSFrameworkLibSize
{
    [WXAppConfiguration sharedConfiguration].JSFrameworkLibSize = JSFrameworkLibSize;
}

+ (NSArray*)customizeProtocolClasses{
    return [WXAppConfiguration sharedConfiguration].customizeProtocolClasses;
}

+ (void)setCustomizeProtocolClasses:(NSArray *)customizeProtocolClasses{
    [WXAppConfiguration sharedConfiguration].customizeProtocolClasses = customizeProtocolClasses;
}


@end
