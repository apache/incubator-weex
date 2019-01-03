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
#import "WXAnalyzerCenter.h"
#import "WXAnalyzerCenter+Transfer.h"
#import "WXUtility.h"
#import "WXSDKInstance_performance.h"
#import "WXCoreBridge.h"
#import "WXBridgeManager.h"
#import "WXBridgeContext.h"


#define MODULE_WX_APM  @"wxapm"

@interface WXAnalyzerCenter ()
{
    BOOL _interactionLogSwitch;
}
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
        instance.anzlyzerSwitch = NO;
    });

    return instance;
}

+ (void) transferPerformance:(NSString*)instanceId withType:(NSString*) type andKey:(NSString*) key andValue:(id)value
{
    if (![self isOpen] || !instanceId) {
        return;
    }

    if ([self isInteractionLogOpen] && [type isEqualToString:@"stage"]) {
        WXLogDebug(@"wxInteractionAnalyzer : [client][stage],%@,%@,%@",instanceId,key,value);
    }

    NSMutableArray* analyzerList = [self getAnalyzerList];
    if (nil == analyzerList) {
        return;
    }
    
    NSDictionary* dic =  @{
                            @"group":MODULE_WX_APM,
                            @"module":instanceId,
                            @"type":type,
                            @"data":@{key:value}
                            };
    for (id analyzer in analyzerList) {
        if ([analyzer respondsToSelector:(@selector(transfer:))]){
            [analyzer performSelector:@selector(transfer:) withObject:dic];
        }
    }
}

+ (void) transferInteractionInfo:(WXComponent*)targetComponent
{
    if (![self isOpen]) {
        return;
    }
    NSMutableArray* analyzerList = [self getAnalyzerList];
    if (nil == analyzerList) {
        return;
    }
    
    long renderOriginDiffTime = [WXUtility getUnixFixTimeMillis] - targetComponent.weexInstance.performance.renderUnixTimeOrigin;
    
    NSDictionary* dic =  @{
                           @"group":MODULE_WX_APM,
                           @"module":targetComponent.weexInstance.instanceId,
                           @"type":@"wxinteraction",
                           @"data":@{
                                   @"renderOriginDiffTime":@(renderOriginDiffTime),
                                   @"type":targetComponent.type,
                                   @"ref":targetComponent.ref,
                                   @"style":targetComponent.styles,
                                   @"attrs":targetComponent.attributes
                                   }
                           };
    for (id analyzer in analyzerList) {
        if ([analyzer respondsToSelector:(@selector(transfer:))]){
            [analyzer performSelector:@selector(transfer:) withObject:dic];
        }
    }
    if ([self isInteractionLogOpen]) {
        WXLogDebug(@"wxInteractionAnalyzer : [client][wxinteraction]%@",dic);
    }
}

+(void)transErrorInfo:(WXJSExceptionInfo *)errorInfo
{
    if (![self isOpen] || !errorInfo) {
        return;
    }

    WXSDKInstance *instance = [WXSDKManager instanceForID:errorInfo.instanceId];
    if (!instance) {
        return;
    }
    NSMutableArray* analyzerList = [self getAnalyzerList];
    if (nil == analyzerList) {
        return;
    }

    NSDictionary *dic= @{
                         @"errorCode":errorInfo.errorCode?:@"-1",
                         @"errorGroup":@"",
                         @"errorMsg":errorInfo.exception?:@"unSetException",
                         @"instanceId":instance.instanceId,
                         @"url":[instance.scriptURL absoluteString]?:@"unSetscriptURL",
                         @"group":GROUP_ANALYZER,
                         @"module":MODULE_ERROR,
                         @"type":TYPE_JS_ERROR
                        };
    
    for (id analyzer in analyzerList) {
        if ([analyzer respondsToSelector:(@selector(transfer:))]){
            [analyzer performSelector:@selector(transfer:) withObject:dic];
        }
    }
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


+ (void)setOpen:(BOOL)isOpen
{
    [WXAnalyzerCenter sharedInstance].anzlyzerSwitch = isOpen;
}

+ (BOOL)isOpen
{
     return [WXAnalyzerCenter sharedInstance].anzlyzerSwitch;
}

+(void) switchInteractionLog:(BOOL) isOpen
{
    if ([WXAnalyzerCenter sharedInstance]->_interactionLogSwitch == isOpen) {
        return;
    }
    [WXAnalyzerCenter sharedInstance]->_interactionLogSwitch = isOpen;
    [WXCoreBridge registerCoreEnv:@"switchInteractionLog" withValue:isOpen?@"true":@"false"];
#ifdef DEBUG
    [WXLog setLogLevel: isOpen?WXLogLevelDebug:WXLogLevelLog];
#else
     [WXLog setLogLevel: isOpen?WXLogLevelDebug:WXLogLevelWarning];
#endif

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wundeclared-selector"
    if ([WXSDKManager.bridgeMgr respondsToSelector:@selector(bridgeCtx)]) {
        id bridgeCtx = [WXSDKManager.bridgeMgr performSelector:@selector(bridgeCtx) withObject:nil];
        if (nil != bridgeCtx && [bridgeCtx respondsToSelector:@selector(callJSMethod:args:)]) {
            WXPerformBlockOnBridgeThread(^(){
                NSArray* args = @[isOpen?@(1):@(0)];
                [bridgeCtx performSelector:@selector(callJSMethod:args:) withObject:@"switchInteractionLog" withObject:args];
            });
        }
    }
#pragma clang diagnostic pop
}

+(BOOL) isInteractionLogOpen
{
    return [WXAnalyzerCenter sharedInstance]->_interactionLogSwitch;
}

@end
