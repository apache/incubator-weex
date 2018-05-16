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
#import "WXAnalyzerCenter.h"
#import "WXAnalyzerProtocol.h"
#import "WXAppMonitorProtocol.h"
#import "WXSDKManager.h"
#import "WXLog.h"
#import "WXTracingManager.h"
#import "WXAnalyzerCenter.h"
#import "WXAnalyzerCenter+Transfer.h"
#import "WXUtility.h"

@interface WXAnalyzerCenter ()
@property (nonatomic, strong) NSMutableArray<WXAnalyzerProtocol> *analyzerList;
@property (nonatomic, assign) BOOL anzlyzerSwitch;
@end

@implementation WXAnalyzerCenter

+ (instancetype) sharedInstance{

    static WXAnalyzerCenter *instance = nil;
    static dispatch_once_t once;

    dispatch_once(&once, ^{
        instance = [[WXAnalyzerCenter alloc] init];
        instance.analyzerList= [NSMutableArray<WXAnalyzerProtocol> new];
        instance.anzlyzerSwitch = FALSE;
    });

    return instance;
}

+ (void) transDataOnState:(CommitState) timeState withInstaneId:(NSString *)instanceId data:(NSDictionary *)data
{
    if (![self isOpen]) {
        return;
    }
    if (!instanceId) {
        return;
    }
    WXSDKInstance * instance = [WXSDKManager instanceForID:instanceId];
    if (!instance) {
        return;
    }

    NSDictionary *commitDimenKeys = [self getKeys:TRUE];
    NSDictionary *commitMeasureKeys = [self getKeys:FALSE];
    for(id key in data){
       if([self checkDataWithSate:timeState checkKey:key limitDic:commitMeasureKeys]){
           [self _transMeasureValue:instance key:key withVal:[data valueForKey:key]];
       }else if([self checkDataWithSate:timeState checkKey:key limitDic:commitDimenKeys]){
           [self _transDimenValue:instance key:key withVal:[data valueForKey:key]];
       }else{
           WXLogDebug(@"WXAnalyzerDataTransfer -> unKnowPerformanceKey :%@",key);
       }
    }
}

+(BOOL) checkDataWithSate:(CommitState)timeState checkKey:(id)key limitDic:(NSDictionary *)limitDic
{
    if (!key || ![key isKindOfClass:[NSString class]]) {
        return FALSE;
    }
   
    if (![limitDic objectForKey:key]) {
        return FALSE;
    }
    CommitState limitSate = [[limitDic objectForKey:key] intValue];
    return timeState == limitSate;
}

+ (NSDictionary *) getKeys:(BOOL) measureOrDimen
{
    static NSDictionary *commitDimenKeys;
    static NSDictionary *commitMeasureKeys;
    static dispatch_once_t onceToken;
    
    dispatch_once(&onceToken, ^{
        // non-standard perf commit names, remove this hopefully.
        
        commitDimenKeys =@{
                           BIZTYPE:             [NSNumber numberWithInt:DebugAfterFSFinish],
                           PAGENAME:            [NSNumber numberWithInt:DebugAfterRequest],
                           WXSDKVERSION:        [NSNumber numberWithInt:DebugAfterRequest],
                           JSLIBVERSION:        [NSNumber numberWithInt:DebugAfterRequest],
                           JSLIBSIZE:           [NSNumber numberWithInt:DebugAfterRequest],
                           WXREQUESTTYPE:       [NSNumber numberWithInt:DebugAfterRequest],
                           WXCONNECTIONTYPE:    [NSNumber numberWithInt:DebugAfterRequest],
                           NETWORKTYPE:         [NSNumber numberWithInt:DebugAfterRequest],
                           CACHETYPE:           [NSNumber numberWithInt:DebugAfterRequest],
                           WXCUSTOMMONITORINFO: [NSNumber numberWithInt:DebugAfterRequest]
        };
        commitMeasureKeys =@{
                             SDKINITTIME:                   [NSNumber numberWithInt:DebugAfterFSFinish],
                             SDKINITINVOKETIME:             [NSNumber numberWithInt:DebugAfterFSFinish],
                             JSLIBINITTIME:                 [NSNumber numberWithInt:DebugAfterFSFinish],
                             JSTEMPLATESIZE:                [NSNumber numberWithInt:DebugAfterRequest],
                             NETWORKTIME:                   [NSNumber numberWithInt:DebugAfterRequest],
                             COMMUNICATETIME:               [NSNumber numberWithInt:DebugAfterExist],
                             SCREENRENDERTIME:              [NSNumber numberWithInt:DebugAfterExist],
                             TOTALTIME:                     [NSNumber numberWithInt:DebugAfterExist],
                             FIRSETSCREENJSFEXECUTETIME:    [NSNumber numberWithInt:DebugAfterFSFinish],
                             CALLCREATEINSTANCETIME:        [NSNumber numberWithInt:DebugAfterFSFinish],
                             COMMUNICATETOTALTIME:          [NSNumber numberWithInt:DebugAfterExist],
                             FSRENDERTIME:                  [NSNumber numberWithInt:DebugAfterExist],
                             CACHEPROCESSTIME:              [NSNumber numberWithInt:DebugAfterRequest],
                             CACHERATIO:                    [NSNumber numberWithInt:DebugAfterRequest],
                             M_FS_CALL_JS_TIME:             [NSNumber numberWithInt:DebugAfterFSFinish],
                             M_FS_CALL_JS_NUM:              [NSNumber numberWithInt:DebugAfterFSFinish],
                             M_FS_CALL_NATIVE_TIME:         [NSNumber numberWithInt:DebugAfterFSFinish],
                             M_FS_CALL_NATIVE_NUM:          [NSNumber numberWithInt:DebugAfterFSFinish],
                             M_FS_CALL_EVENT_NUM:           [NSNumber numberWithInt:DebugAfterFSFinish],
                             M_CELL_EXCEED_NUM:             [NSNumber numberWithInt:DebugAfterFSFinish],
                             M_MAX_DEEP_VDOM:               [NSNumber numberWithInt:DebugAfterExist],
                             M_IMG_WRONG_SIZE_NUM:          [NSNumber numberWithInt:DebugAfterExist],
                             M_TIMER_NUM:                   [NSNumber numberWithInt:DebugAfterFSFinish],
                             M_INTERACTION_TIME:            [NSNumber numberWithInt:DebugAfterExist],
                             M_COMPONENT_FS_TIME:           @(DebugAfterFSFinish),
                             M_COMPONENT_FS_COUNT:          @(DebugAfterFSFinish),
                             COMPONENTCOUNT:                @(DebugAfterExist),
                             M_COMPONENT_TIME:              @(DebugAfterExist)
                             };
        
    });
    return measureOrDimen?commitMeasureKeys:commitDimenKeys;
}

+ (void) _transMeasureValue:(WXSDKInstance *)instance key:(NSString *)commitKey withVal:(id)commitVal
{
    [self _transDataToAnaylzer:instance
                        withModule:MODULE_PERFORMANCE
                        withType:TYPE_MEASURE_REAL
                        withData:@{commitKey:commitVal}
     ];
}

+ (void) _transDimenValue:(WXSDKInstance *)instance key:(NSString *)commitKey withVal:(id)commitVal
{
    [self _transDataToAnaylzer:instance
                        withModule:MODULE_PERFORMANCE
                        withType:TYPE_DIMEN_REAL
                        withData:@{commitKey:commitVal}
     ];
}

+(void) _transDataToAnaylzer:(WXSDKInstance *)instance withModule:(NSString *)module  withType:(NSString *)type withData:(NSDictionary *)data
{
    NSMutableArray* analyzerList = [self getAnalyzerList];
    if (nil == analyzerList) {
        return;
    }
    
    NSMutableDictionary *wrapDic = [data mutableCopy];
    [wrapDic setObject:instance.instanceId forKey:@"instanceId"];
    [wrapDic setObject:[instance.scriptURL absoluteString]?:@"unSetscriptURL" forKey:@"url"];
    [wrapDic setValue:GROUP_ANALYZER forKey:@"group"];
    [wrapDic setValue:module forKey:@"module"];
    [wrapDic setValue:type forKey:@"type"];

    
    if ([self needTransfer]) {
        for (id analyzer in analyzerList) {
            if ( [analyzer respondsToSelector:(@selector(transfer:))])
            {
                [analyzer performSelector:@selector(transfer:) withObject:wrapDic];
            }
        }
    }
    
    
}

+(void)transErrorInfo:(WXJSExceptionInfo *)errorInfo
{
    if (!errorInfo) {
        return;
    }
    WXSDKInstance *instance = [WXSDKManager instanceForID:errorInfo.instanceId];
    if (!instance) {
        return;
    }
    NSDictionary *dic= @{
                         @"errorCode":errorInfo.errorCode,
                         @"errorGroup":@"",
                         @"errorMsg":errorInfo.exception
                         };
    
    [self _transDataToAnaylzer:instance
                    withModule:MODULE_ERROR
                      withType:TYPE_JS_ERROR
                      withData:dic
     ];
}

+ (void) addWxAnalyzer:(id<WXAnalyzerProtocol>)handler
{
    if (!handler) {
        return;
    }
    [[WXAnalyzerCenter sharedInstance].analyzerList addObject:handler];
}

+ (void) rmWxAnalyzer:(id<WXAnalyzerProtocol>)handler
{
    if (!handler) {
        return;
    }
    [[WXAnalyzerCenter sharedInstance].analyzerList removeObject:handler];
}

+ (NSMutableArray<WXAnalyzerProtocol> *)getAnalyzerList
{
    return [WXAnalyzerCenter sharedInstance].analyzerList;
}

+(BOOL) needTransfer
{
    if(![self isOpen]){
        return FALSE;
    }
    NSMutableArray* analyzerList = [self getAnalyzerList];
    if (nil == analyzerList || analyzerList.count <= 0) {
        return FALSE;
    }
    return TRUE;
}

+ (void)setOpen:(BOOL)isOpen
{
    [WXAnalyzerCenter sharedInstance].anzlyzerSwitch = isOpen;
}

+ (BOOL)isOpen
{
     return [WXAnalyzerCenter sharedInstance].anzlyzerSwitch;
}

@end
