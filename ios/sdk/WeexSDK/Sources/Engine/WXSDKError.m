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
#define ERROR_GROUP   @"groupe"

@implementation WXSDKErrCodeUtil

+(WXSDKErrorType)getErrorTypeByCode:(WXSDKErrCode)code
{
    NSDictionary* codeMap = [[self getMap] objectForKey:@(code)];
    if (!codeMap) {
        return NATIVE_ERROR;
    }
    return [[codeMap objectForKey:ERROR_TYPE] intValue];
}


+(WXSDKErrorGroup) getErrorGroupByCode:(WXSDKErrCode)code
{
    NSDictionary* codeMap = [[self getMap] objectForKey:@(code)];
    if (!codeMap) {
        return NATIVE;
    }
    return [[codeMap objectForKey:ERROR_GROUP] intValue];
}

+(NSDictionary *) getMap
{
    static NSDictionary *codeMap;
    static dispatch_once_t onceToken;
    
    dispatch_once(&onceToken, ^{
        
        codeMap=@{
                @(WX_ERR_JSFRAMEWORK_START):@{ERROR_TYPE:@(NATIVE_ERROR),ERROR_GROUP:@(NATIVE)},
                @(WX_ERR_JSFRAMEWORK_LOAD):@{ERROR_TYPE:@(NATIVE_ERROR),ERROR_GROUP:@(NATIVE)},
                @(WX_ERR_JSFRAMEWORK_EXECUTE):@{ERROR_TYPE:@(NATIVE_ERROR),ERROR_GROUP:@(NATIVE)},
                @(WX_ERR_JSFRAMEWORK_END):@{ERROR_TYPE:@(NATIVE_ERROR),ERROR_GROUP:@(NATIVE)},
                
                @(WX_ERR_JSBRIDGE_START):@{ERROR_TYPE:@(NATIVE_ERROR),ERROR_GROUP:@(NATIVE)},
                @(WX_ERR_RENDER_CREATEBODY):@{ERROR_TYPE:@(NATIVE_ERROR),ERROR_GROUP:@(NATIVE)},
                @(WX_ERR_INVOKE_NATIVE):@{ERROR_TYPE:@(NATIVE_ERROR),ERROR_GROUP:@(NATIVE)},
                @(WX_ERR_JS_EXECUTE):@{ERROR_TYPE:@(JS_ERROR),ERROR_GROUP:@(JS)},
                @(WX_ERR_JSBRIDGE_END):@{ERROR_TYPE:@(JS_ERROR),ERROR_GROUP:@(JS)},
                
                @(WX_ERR_RENDER_START):@{ERROR_TYPE:@(NATIVE_ERROR),ERROR_GROUP:@(NATIVE)},
                @(WX_ERR_JSFRAMEWORK_START):@{ERROR_TYPE:@(NATIVE_ERROR),ERROR_GROUP:@(NATIVE)},
                @(WX_ERR_RENDER_UPDATTR):@{ERROR_TYPE:@(NATIVE_ERROR),ERROR_GROUP:@(NATIVE)},
                @(WX_ERR_RENDER_UPDSTYLE):@{ERROR_TYPE:@(NATIVE_ERROR),ERROR_GROUP:@(NATIVE)},
                @(WX_ERR_RENDER_ADDELEMENT):@{ERROR_TYPE:@(NATIVE_ERROR),ERROR_GROUP:@(NATIVE)},
                @(WX_ERR_RENDER_REMOVEELEMENT):@{ERROR_TYPE:@(NATIVE_ERROR),ERROR_GROUP:@(NATIVE)},
                @(WX_ERR_RENDER_MOVEELEMENT):@{ERROR_TYPE:@(NATIVE_ERROR),ERROR_GROUP:@(NATIVE)},
                @(WX_ERR_RENDER_ADDEVENT):@{ERROR_TYPE:@(NATIVE_ERROR),ERROR_GROUP:@(NATIVE)},
                @(WX_ERR_RENDER_REMOVEEVENT):@{ERROR_TYPE:@(NATIVE_ERROR),ERROR_GROUP:@(NATIVE)},
                @(WX_ERR_RENDER_SCROLLTOELEMENT):@{ERROR_TYPE:@(NATIVE_ERROR),ERROR_GROUP:@(NATIVE)},
                @(WX_ERR_RENDER_END):@{ERROR_TYPE:@(NATIVE_ERROR),ERROR_GROUP:@(NATIVE)},
                
                @(WX_ERR_DOWNLOAD_START):@{ERROR_TYPE:@(NATIVE_ERROR),ERROR_GROUP:@(NATIVE)},
                @(WX_ERR_JSBUNDLE_DOWNLOAD):@{ERROR_TYPE:@(NATIVE_ERROR),ERROR_GROUP:@(NATIVE)},
                @(WX_ERR_JSBUNDLE_STRING_CONVERT):@{ERROR_TYPE:@(NATIVE_ERROR),ERROR_GROUP:@(NATIVE)},
                @(WX_ERR_NOT_CONNECTED_TO_INTERNET):@{ERROR_TYPE:@(NATIVE_ERROR),ERROR_GROUP:@(NATIVE)},
                @(WX_ERR_CANCEL):@{ERROR_TYPE:@(NATIVE_ERROR),ERROR_GROUP:@(NATIVE)},
                @(WX_ERR_DOWNLOAD_END):@{ERROR_TYPE:@(NATIVE_ERROR),ERROR_GROUP:@(NATIVE)},
                
                @(WX_KEY_EXCEPTION_SDK_INIT):@{ERROR_TYPE:@(NATIVE_ERROR),ERROR_GROUP:@(NATIVE)},
                @(WX_KEY_EXCEPTION_INVOKE):@{ERROR_TYPE:@(NATIVE_ERROR),ERROR_GROUP:@(NATIVE)},
                @(WX_KEY_EXCEPTION_JS_DOWNLOAD):@{ERROR_TYPE:@(NATIVE_ERROR),ERROR_GROUP:@(NATIVE)},
                @(WX_KEY_EXCEPTION_DOM):@{ERROR_TYPE:@(NATIVE_ERROR),ERROR_GROUP:@(NATIVE)},
                @(WX_KEY_EXCEPTION_WXBRIDGE):@{ERROR_TYPE:@(JS_ERROR),ERROR_GROUP:@(JS)},
                
                @(WX_KEY_EXCEPTION_DEGRADE):@{ERROR_TYPE:@(DEGRAD_ERROR),ERROR_GROUP:@(NATIVE)},
                @(WX_KEY_EXCEPTION_DEGRADE_CHECK_CONTENT_LENGTH_FAILED):@{ERROR_TYPE:@(DEGRAD_ERROR),ERROR_GROUP:@(NATIVE)},
                @(WX_KEY_EXCEPTION_DEGRADE_BUNDLE_CONTENTTYPE_ERROR):@{ERROR_TYPE:@(DEGRAD_ERROR),ERROR_GROUP:@(NATIVE)},
                @(WX_KEY_EXCEPTION_DEGRADE_OTHER_CAUSE):@{ERROR_TYPE:@(DEGRAD_ERROR),ERROR_GROUP:@(NATIVE)},
                
                @(WX_KEY_EXCEPTION_ABILITY_DOWN):@{ERROR_TYPE:@(NATIVE_ERROR),ERROR_GROUP:@(NATIVE)},
                @(WX_KEY_EXCEPTION_ABILITY_DOWN_IMAGE):@{ERROR_TYPE:@(NATIVE_ERROR),ERROR_GROUP:@(NATIVE)},
                @(WX_KEY_EXCEPTION_ABILITY_DOWN_TOH5):@{ERROR_TYPE:@(NATIVE_ERROR),ERROR_GROUP:@(NATIVE)},
                @(WX_ERR_JSFRAMEWORK_START):@{ERROR_TYPE:@(NATIVE_ERROR),ERROR_GROUP:@(NATIVE)},
                @(WX_KEY_EXCEPTION_ABILITY_DOWN_):@{ERROR_TYPE:@(NATIVE_ERROR),ERROR_GROUP:@(NATIVE)},
                @(WX_KEY_EXCEPTION_EMPTY_SCREEN_JS):@{ERROR_TYPE:@(RENDER_ERROR),ERROR_GROUP:@(JS)}
                };
    });
    return codeMap;
}

@end
