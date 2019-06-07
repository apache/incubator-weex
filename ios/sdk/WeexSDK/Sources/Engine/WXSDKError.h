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

#define WX_ERROR_GROUP_NATIVE   @"NATIVE"
#define WX_ERROR_GROUP_JS       @"JS"
#define WX_ERROR_GROUP_NET      @"NET"

#define WX_ERROR_TYPE_NATIVE    @"NATIVE_ERROR"
#define WX_ERROR_TYPE_JS        @"JS_ERROR"
#define WX_ERROR_TYPE_DEGRADE   @"DEGRAD_ERROR"
#define WX_ERROR_TYPE_RENDER    @"RENDER_ERROR"
#define WX_ERROR_TYPE_DOWNLOAD  @"DOWN_LOAD_ERROR"


typedef NS_ENUM(int, WXSDKErrCode)
{
    WX_KEY_EXCEPTION_DEGRADE = -1000,
    WX_KEY_EXCEPTION_DEGRADE_CHECK_CONTENT_LENGTH_FAILED = -1003,
    WX_KEY_EXCEPTION_DEGRADE_BUNDLE_CONTENTTYPE_ERROR = -1004,
    WX_KEY_EXCEPTION_DEGRADE_OTHER_CAUSE = -1005,
    WX_KEY_EXCEPTION_DEGRADE_EAGLE_RENDER_ERROR = -1007,
    WX_KEY_EXCEPTION_DEGRADE_NET_CODE_CAUSE = -1008,
    
    WX_ERR_JSBRIDGE_START = -2001,
    WX_ERR_JSFUNC_PARAM = -2009,
    WX_ERR_INVOKE_NATIVE = -2012,
    WX_ERR_JS_EXECUTE = -2013,
    WX_ERR_JSBRIDGE_END = -2099,
    
    WX_ERR_RENDER_START = -2100,
    WX_ERR_RENDER_CREATEBODY = -2100,
    WX_ERR_RENDER_UPDATTR = -2101,
    WX_ERR_RENDER_UPDSTYLE = -2102,
    WX_ERR_RENDER_ADDELEMENT = -2103,
    WX_ERR_RENDER_REMOVEELEMENT = -2104,
    WX_ERR_RENDER_MOVEELEMENT = -2105,
    WX_ERR_RENDER_ADDEVENT = -2106,
    WX_ERR_RENDER_REMOVEEVENT = -2107,
    WX_ERR_RENDER_SCROLLTOELEMENT = -2110,
    WX_ERR_RENDER_TWICE = -2111,
    WX_ERR_RENDER_END = -2199,
    
    WX_ERR_DOWNLOAD_START = -2201,
    WX_ERR_JSBUNDLE_DOWNLOAD = -2202,
    WX_ERR_JSBUNDLE_STRING_CONVERT = -2203,
    WX_ERR_NOT_CONNECTED_TO_INTERNET = -2205,
    WX_ERR_CANCEL = -2204,
    WX_ERR_DOWNLOAD_END = -2299,
    
    WX_KEY_EXCEPTION_SDK_INIT = -9000,
    WX_KEY_EXCEPTION_INVOKE = -9100,
    WX_KEY_EXCEPTION_JS_DOWNLOAD =-9200,
    WX_KEY_EXCEPTION_DOM = -9300,
    WX_KEY_EXCEPTION_WXBRIDGE=-9400,
    
    WX_KEY_EXCEPTION_ABILITY_DOWN = -9600,
    WX_KEY_EXCEPTION_ABILITY_DOWN_IMAGE = -9601,
    WX_KEY_EXCEPTION_ABILITY_DOWN_TOH5 = -9602,
    WX_KEY_EXCEPTION_ABILITY_DOWN_ = -9603,
    
    WX_KEY_EXCEPTION_EMPTY_SCREEN_JS = -9700,
    WX_KEY_EXCEPTION_EMPTY_SCREEN_NATIVE = -9701,
    
    WX_KEY_EXCEPTION_TOO_MANY_TIMERS = -9800,
    WX_KEY_EXCEPTION_NO_BUNDLE_TYPE = -9801,
};

typedef NS_ENUM (NSInteger,WXSDKErrorType)
{
    WX_JS_ERROR,
    WX_NATIVE_ERROR,
    WX_RENDER_ERROR,
    WX_DEGRADE_ERROR,
    WX_DOWN_LOAD_ERROR
};

typedef NS_ENUM (NSInteger,WXSDKErrorGroup){
    WX_JS,
    WX_NATIVE,
    WX_NET
};

@interface WXSDKErrCodeUtil :NSObject

+ (WXSDKErrorType) getErrorTypeByCode:(WXSDKErrCode) code;
+ (WXSDKErrorGroup) getErrorGroupByCode:(WXSDKErrCode) code;
+ (NSString *) convertGroupToStringName:(WXSDKErrorGroup) group;
+ (NSString *) convertTypeToStringName:(WXSDKErrorType)type;

@end

