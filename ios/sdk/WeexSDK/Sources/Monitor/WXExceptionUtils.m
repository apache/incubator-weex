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

#import "WXExceptionUtils.h"
#import "WXJSExceptionProtocol.h"
#import "WXHandlerFactory.h"
#import "WXSDKEngine.h"
#import "WXSDKError.h"
#import "WXJSExceptionInfo.h"
#import "WXUtility.h"
#import "WXSDKManager.h"


@implementation WXExceptionUtils

+ (void)commitCriticalExceptionRT:(NSString *)instanceId errCode:(NSString *)errCode function:(NSString *)function exception:(NSString *)exception extParams:(NSDictionary *)extParams{
    NSString *bundleUrlCommit = @"BundleUrlDefault";
    NSString *instanceIdCommit = @"InstanceIdDefalut";
    
    if(![WXUtility isBlankString:instanceId]){
        instanceIdCommit = instanceId;
        WXSDKInstance * instance = [WXSDKManager instanceForID:instanceId];
        if(instance){
            bundleUrlCommit = instance.pageName?:([instance.scriptURL absoluteString]?:bundleUrlCommit);
        }else if([instanceIdCommit hasPrefix:@"WX_KEY_EXCEPTION"]){
            bundleUrlCommit = instanceId;
        }
    }
    
    WXJSExceptionInfo * jsExceptionInfo = [[WXJSExceptionInfo alloc] initWithInstanceId:instanceIdCommit bundleUrl:bundleUrlCommit errorCode:errCode functionName:function exception:exception userInfo: [extParams mutableCopy]];
    [WXExceptionUtils commitCriticalExceptionRT:jsExceptionInfo];
}

+ (void)commitCriticalExceptionRT:(WXJSExceptionInfo *)jsExceptionInfo{
    id<WXJSExceptionProtocol> jsExceptionHandler = [WXHandlerFactory handlerForProtocol:@protocol(WXJSExceptionProtocol)];
    if ([jsExceptionHandler respondsToSelector:@selector(onJSException:)]) {
        [jsExceptionHandler onJSException:jsExceptionInfo];
    }
}

@end
