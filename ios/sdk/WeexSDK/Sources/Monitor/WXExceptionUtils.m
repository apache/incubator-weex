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
#import "WXAnalyzerCenter+Transfer.h"



@implementation WXExceptionUtils

+ (void)commitCriticalExceptionRT:(NSString *)instanceId errCode:(NSString *)errCode function:(NSString *)function exception:(NSString *)exception extParams:(NSDictionary *)extParams{
    NSString *bundleUrlCommit = @"BundleUrlDefault";
    NSString *instanceIdCommit = @"InstanceIdDefalut";
    
    WXSDKInstance * instance ;
    if(![WXUtility isBlankString:instanceId]){
        instanceIdCommit = instanceId;
        instance = [WXSDKManager instanceForID:instanceId];
        if(instance){
            bundleUrlCommit = instance.pageName?:([instance.scriptURL absoluteString]?:bundleUrlCommit);
            NSMutableDictionary* extInfo = [[NSMutableDictionary alloc] initWithDictionary:extParams];
            if (instance.containerInfo && instance.containerInfo.count >0) {
                [extInfo addEntriesFromDictionary:instance.containerInfo];
            }
            [extInfo setObject:[self _convertInstanceStageToStr:instance] forKey:@"wxStageList"];
            [extInfo setObject:instance.pageName?:@"unKnowPageNameCaseUnSet" forKey:@"wxPageName"];
            [extInfo setObject:instance.bundleTemplate?:@"has recycle" forKey:@"wxTemplateOfBundle"];
            extParams = extInfo;
        }else if([instanceIdCommit hasPrefix:@"WX_KEY_EXCEPTION"]){
            bundleUrlCommit = instanceId;
        }
    }
  
    WXJSExceptionInfo * jsExceptionInfo = [[WXJSExceptionInfo alloc] initWithInstanceId:instanceIdCommit bundleUrl:bundleUrlCommit errorCode:errCode functionName:function exception:exception userInfo: [extParams mutableCopy]];
    
    //record top5 erromsg ,if errorType is not WX_RENDER_ERROR
    NSNumberFormatter *formater = [[NSNumberFormatter alloc] init];
    formater.numberStyle = NSNumberFormatterDecimalStyle;
    NSNumber *codeNumber = [formater numberFromString:errCode];
    if (codeNumber) {
       WXSDKErrorType type = [WXSDKErrCodeUtil getErrorTypeByCode:codeNumber.intValue];
        if (type != WX_RENDER_ERROR && nil != instance) {
            [instance.apmInstance recordErrorMsg:jsExceptionInfo];
        }
    }
  
    [WXExceptionUtils commitCriticalExceptionRT:jsExceptionInfo];
}

+ (void)commitCriticalExceptionRT:(WXJSExceptionInfo *)jsExceptionInfo{
    id<WXJSExceptionProtocol> jsExceptionHandler = [WXHandlerFactory handlerForProtocol:@protocol(WXJSExceptionProtocol)];
    if ([jsExceptionHandler respondsToSelector:@selector(onJSException:)]) {
        [jsExceptionHandler onJSException:jsExceptionInfo];
    }
    if ([WXAnalyzerCenter isOpen]) {
        [WXAnalyzerCenter transErrorInfo:jsExceptionInfo];
    }
}

+ (NSString*) _convertInstanceStageToStr:(WXSDKInstance *)instance
{
    if (nil == instance || !instance.apmInstance.isOpenApm) {
        return @"";
    }
    NSDictionary<NSString*,NSNumber*>* stageDic = instance.apmInstance.stageDic;
    if (!stageDic || stageDic.count <=0 ) {
        return @"emptyStageRecord";
    }
    NSString* stageStr = @"";
    for (NSString* key in stageDic) {
        NSNumber* time = [stageDic objectForKey:key];
        stageStr = [stageStr stringByAppendingFormat:@"%@ -> %@:%@",stageStr,key,time];
    }
    return stageStr;
}



@end
