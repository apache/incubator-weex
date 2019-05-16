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

#import "WXVersion.h"
#import "WXDefine.h"

static const char* WeexSDKBuildTime = "2019-04-23 06:34:41 UTC";
static const unsigned long WeexSDKBuildTimestamp = 1556001281;

NSString* GetWeexSDKVersion(void)
{
    return WX_SDK_VERSION;
}

NSString* GetWeexSDKBuildTime(void)
{
    return [NSString stringWithUTF8String:WeexSDKBuildTime];
}

unsigned long GetWeexSDKBuildTimestamp(void)
{
    return WeexSDKBuildTimestamp;
}

