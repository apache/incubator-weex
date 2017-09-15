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

#import "WXJSExceptionInfo.h"
#import "WXAppConfiguration.h"
#import "WXDefine.h"

@implementation WXJSExceptionInfo

- (instancetype)initWithInstanceId:(NSString *)instanceId
                         bundleUrl:(NSString *)bundleUrl
                         errorCode:(NSString *)errorCode
                      functionName:(NSString *)functionName
                         exception:(NSString *)exception
                          userInfo:(NSMutableDictionary *)userInfo
{
    if (self = [super init]) {
        self.instanceId = instanceId;
        self.bundleUrl = bundleUrl;
        self.errorCode = errorCode;
        self.exception = exception;
        self.userInfo = userInfo;
        self.functionName = functionName;
        _jsfmVersion = [WXAppConfiguration JSFrameworkVersion];
        _sdkVersion = WX_SDK_VERSION;
    }
    return self;
}

- (NSString *)description {
    return [NSString stringWithFormat:@"instanceId:%@ bundleUrl:%@ errorCode:%@ functionName:%@ exception:%@ userInfo:%@ jsfmVersion:%@ sdkVersion:%@", _instanceId?:@"", _bundleUrl?:@"", _errorCode?:@"", _functionName?:@"", _exception?:@"", _userInfo?:@"", _jsfmVersion, _sdkVersion];
}
@end
