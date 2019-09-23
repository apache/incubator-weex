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
#import "WXSDKError.h"

#define ERROR_TYPE    @"type"
#define ERROR_GROUP   @"group"
#define ERROR_ALIAS   @"alias"

@implementation WXSDKErrCodeUtil

+(WXSDKErrorType)getErrorTypeByCode:(WXSDKErrCode)code
{
    NSDictionary* codeMap = [[self getMap] objectForKey:@(code)];
    if (!codeMap) {
        return WX_NATIVE_ERROR;
    }
    return [[codeMap objectForKey:ERROR_TYPE] intValue];
}


+(WXSDKErrorGroup) getErrorGroupByCode:(WXSDKErrCode)code
{
    NSDictionary* codeMap = [[self getMap] objectForKey:@(code)];
    if (!codeMap) {
        return WX_NATIVE;
    }
    return [[codeMap objectForKey:ERROR_GROUP] intValue];
}

+(NSDictionary *) getMap
{
    static NSDictionary *codeMap;
    static dispatch_once_t onceToken;
    
    dispatch_once(&onceToken, ^{
        
        codeMap=@{
                @(WX_ERR_JSFRAMEWORK_START):@{ERROR_TYPE:@(WX_NATIVE_ERROR),ERROR_GROUP:@(WX_NATIVE)},
                @(WX_ERR_JSFRAMEWORK_LOAD):@{ERROR_TYPE:@(WX_NATIVE_ERROR),ERROR_GROUP:@(WX_NATIVE)},
                @(WX_ERR_JSFRAMEWORK_EXECUTE):@{ERROR_TYPE:@(WX_NATIVE_ERROR),ERROR_GROUP:@(WX_NATIVE)},
                @(WX_ERR_JSFRAMEWORK_END):@{ERROR_TYPE:@(WX_NATIVE_ERROR),ERROR_GROUP:@(WX_NATIVE)},
                
                @(WX_ERR_JSBRIDGE_START):@{ERROR_TYPE:@(WX_NATIVE_ERROR),ERROR_GROUP:@(WX_NATIVE)},
                @(WX_ERR_RENDER_CREATEBODY):@{ERROR_TYPE:@(WX_NATIVE_ERROR),ERROR_GROUP:@(WX_NATIVE)},
                @(WX_ERR_INVOKE_NATIVE):@{ERROR_TYPE:@(WX_NATIVE_ERROR),ERROR_GROUP:@(WX_NATIVE)},
                @(WX_ERR_JS_EXECUTE):@{ERROR_TYPE:@(WX_JS_ERROR),ERROR_GROUP:@(WX_JS)},
                @(WX_ERR_JSBRIDGE_END):@{ERROR_TYPE:@(WX_JS_ERROR),ERROR_GROUP:@(WX_JS)},
                
                @(WX_ERR_RENDER_START):@{ERROR_TYPE:@(WX_NATIVE_ERROR),ERROR_GROUP:@(WX_NATIVE)},
                @(WX_ERR_RENDER_UPDATTR):@{ERROR_TYPE:@(WX_NATIVE_ERROR),ERROR_GROUP:@(WX_NATIVE)},
                @(WX_ERR_RENDER_UPDSTYLE):@{ERROR_TYPE:@(WX_NATIVE_ERROR),ERROR_GROUP:@(WX_NATIVE)},
                @(WX_ERR_RENDER_ADDELEMENT):@{ERROR_TYPE:@(WX_NATIVE_ERROR),ERROR_GROUP:@(WX_NATIVE)},
                @(WX_ERR_RENDER_REMOVEELEMENT):@{ERROR_TYPE:@(WX_NATIVE_ERROR),ERROR_GROUP:@(WX_NATIVE)},
                @(WX_ERR_RENDER_MOVEELEMENT):@{ERROR_TYPE:@(WX_NATIVE_ERROR),ERROR_GROUP:@(WX_NATIVE)},
                @(WX_ERR_RENDER_ADDEVENT):@{ERROR_TYPE:@(WX_NATIVE_ERROR),ERROR_GROUP:@(WX_NATIVE)},
                @(WX_ERR_RENDER_REMOVEEVENT):@{ERROR_TYPE:@(WX_NATIVE_ERROR),ERROR_GROUP:@(WX_NATIVE)},
                @(WX_ERR_RENDER_SCROLLTOELEMENT):@{ERROR_TYPE:@(WX_NATIVE_ERROR),ERROR_GROUP:@(WX_NATIVE)},
                @(WX_ERR_RENDER_TWICE):@{ERROR_TYPE:@(WX_NATIVE_ERROR),ERROR_GROUP:@(WX_NATIVE)},
                @(WX_ERR_RENDER_END):@{ERROR_TYPE:@(WX_NATIVE_ERROR),ERROR_GROUP:@(WX_NATIVE)},
                
                @(WX_ERR_DOWNLOAD_START):@{ERROR_TYPE:@(WX_NATIVE_ERROR),ERROR_GROUP:@(WX_NATIVE)},
                @(WX_ERR_JSBUNDLE_DOWNLOAD):@{ERROR_TYPE:@(WX_NATIVE_ERROR),ERROR_GROUP:@(WX_NATIVE)},
                @(WX_ERR_JSBUNDLE_STRING_CONVERT):@{ERROR_TYPE:@(WX_NATIVE_ERROR),ERROR_GROUP:@(WX_NATIVE)},
                @(WX_ERR_NOT_CONNECTED_TO_INTERNET):@{ERROR_TYPE:@(WX_NATIVE_ERROR),ERROR_GROUP:@(WX_NATIVE)},
                @(WX_ERR_CANCEL):@{ERROR_TYPE:@(WX_NATIVE_ERROR),ERROR_GROUP:@(WX_NATIVE)},
                @(WX_ERR_DOWNLOAD_END):@{ERROR_TYPE:@(WX_NATIVE_ERROR),ERROR_GROUP:@(WX_NATIVE)},
                
                @(WX_KEY_EXCEPTION_SDK_INIT):@{ERROR_TYPE:@(WX_NATIVE_ERROR),ERROR_GROUP:@(WX_NATIVE)},
                @(WX_KEY_EXCEPTION_INVOKE):@{ERROR_TYPE:@(WX_NATIVE_ERROR),ERROR_GROUP:@(WX_NATIVE)},
                @(WX_KEY_EXCEPTION_JS_DOWNLOAD):@{ERROR_TYPE:@(WX_DOWN_LOAD_ERROR),ERROR_GROUP:@(WX_NATIVE)},
                @(WX_KEY_EXCEPTION_DOM):@{ERROR_TYPE:@(WX_NATIVE_ERROR),ERROR_GROUP:@(WX_NATIVE)},
                @(WX_KEY_EXCEPTION_WXBRIDGE):@{ERROR_TYPE:@(WX_JS_ERROR),ERROR_GROUP:@(WX_JS)},
                
                @(WX_KEY_EXCEPTION_DEGRADE):@{ERROR_TYPE:@(WX_DEGRADE_ERROR),ERROR_GROUP:@(WX_NATIVE),ERROR_ALIAS:@(WX_UNI_KEY_EXCEPTION_DEGRADE)},
                @(WX_KEY_EXCEPTION_DEGRADE_CHECK_CONTENT_LENGTH_FAILED):@{ERROR_TYPE:@(WX_DEGRADE_ERROR),ERROR_GROUP:@(WX_NET),ERROR_ALIAS:@(WX_UNI_KEY_EXCEPTION_DEGRADE_CHECK_CONTENT_LENGTH_FAILED)},
                @(WX_KEY_EXCEPTION_DEGRADE_BUNDLE_CONTENTTYPE_ERROR):@{ERROR_TYPE:@(WX_DEGRADE_ERROR),ERROR_GROUP:@(WX_NET), ERROR_ALIAS:@(WX_UNI_KEY_EXCEPTION_DEGRADE_BUNDLE_CONTENTTYPE_ERROR)},
                @(WX_KEY_EXCEPTION_DEGRADE_EAGLE_RENDER_ERROR):@{ERROR_TYPE:@(WX_DEGRADE_ERROR),ERROR_GROUP:@(WX_NATIVE),ERROR_ALIAS:@(WX_UNI_KEY_EXCEPTION_DEGRADE_EAGLE_RENDER_ERROR)},
                @(WX_KEY_EXCEPTION_DEGRADE_OTHER_CAUSE):@{ERROR_TYPE:@(WX_DEGRADE_ERROR),ERROR_GROUP:@(WX_NATIVE),ERROR_ALIAS:@(WX_UNI_KEY_EXCEPTION_DEGRADE_OTHER_CAUSE)},
                @(WX_KEY_EXCEPTION_DEGRADE_NET_CODE_CAUSE):@{ERROR_TYPE:@(WX_DEGRADE_ERROR),ERROR_GROUP:@(WX_NET),ERROR_ALIAS:@(WX_UNI_KEY_EXCEPTION_DEGRADE_NET_CODE_CAUSE)},
                
                @(WX_KEY_EXCEPTION_ABILITY_DOWN):@{ERROR_TYPE:@(WX_NATIVE_ERROR),ERROR_GROUP:@(WX_NATIVE)},
                @(WX_KEY_EXCEPTION_ABILITY_DOWN_IMAGE):@{ERROR_TYPE:@(WX_NATIVE_ERROR),ERROR_GROUP:@(WX_NATIVE)},
                @(WX_KEY_EXCEPTION_ABILITY_DOWN_TOH5):@{ERROR_TYPE:@(WX_NATIVE_ERROR),ERROR_GROUP:@(WX_NATIVE)},
                @(WX_KEY_EXCEPTION_ABILITY_DOWN_):@{ERROR_TYPE:@(WX_NATIVE_ERROR),ERROR_GROUP:@(WX_NATIVE)},
                @(WX_KEY_EXCEPTION_EMPTY_SCREEN_JS):@{ERROR_TYPE:@(WX_RENDER_ERROR),ERROR_GROUP:@(WX_JS)},
                @(WX_KEY_EXCEPTION_EMPTY_SCREEN_NATIVE):@{ERROR_TYPE:@(WX_RENDER_ERROR),ERROR_GROUP:@(WX_NATIVE)},
                
                @(WX_KEY_EXCEPTION_NO_BUNDLE_TYPE):@{ERROR_TYPE:@(WX_JS_ERROR),ERROR_GROUP:@(WX_JS)},
                
                @(WX_KEY_EXCEPTION_INVALID_JSON_OBJECT):@{ERROR_TYPE:@(WX_JS_ERROR),ERROR_GROUP:@(WX_JS)},
                
                @(WX_KEY_EXCEPTION_HERON_ERROR):@{ERROR_TYPE:@(WX_NATIVE_ERROR),ERROR_GROUP:@(WX_NATIVE)},
                @(WX_KEY_EXCEPTION_HERON_RENDER_ERROR):@{ERROR_TYPE:@(WX_RENDER_ERROR),ERROR_GROUP:@(WX_NATIVE)},
                };
    });
    return codeMap;
}

+ (NSString *) convertTypeToStringName:(WXSDKErrorType)type
{
    switch (type) {
        case WX_JS_ERROR:
            return WX_ERROR_TYPE_JS;
        case WX_RENDER_ERROR:
            return WX_ERROR_TYPE_RENDER;
        case WX_DEGRADE_ERROR:
            return WX_ERROR_TYPE_DEGRADE;
        case WX_DOWN_LOAD_ERROR:
            return WX_ERROR_TYPE_DOWNLOAD;
        case WX_NATIVE_ERROR:
        default:
            return WX_ERROR_TYPE_NATIVE;
    }
}

+ (NSString *) convertGroupToStringName:(WXSDKErrorGroup)group
{
    switch (group) {
        case WX_JS:
            return WX_ERROR_GROUP_JS;
        case WX_NET:
            return WX_ERROR_GROUP_NET;
        case WX_NATIVE:
        default:
            return WX_ERROR_GROUP_NATIVE;
    }
}

+ (NSString *) convertToUniversalCode:(WXSDKErrCode)code
{
    id aliasCode = [[[self getMap] objectForKey:@(code)] objectForKey:ERROR_ALIAS];
    if (aliasCode) {
        return [aliasCode description];
    }
    else {
        return [@(code) description];
    }
}

@end
