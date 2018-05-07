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
#import "WXSDKInstance_performance.h"
#import "WXMonitor.h"

@implementation WXPerformance

@end

@implementation WXSDKInstance (Performance)

- (void) updatePerDicAfterCreateFinish
{
    [WXMonitor performanceFinishWithState:DebugAfterFSFinish instance:self];
    self.performance.jsCreateFinishTime =  CACurrentMediaTime()*1000;
    self.isJSCreateFinish = TRUE;
    WX_MONITOR_PERF_SET(WXPTFsCallJsNum, self.performance.fsCallJsNum, self);
    WX_MONITOR_PERF_SET(WXPTFsCallJsTime, self.performance.fsCallJsTime, self);
    WX_MONITOR_PERF_SET(WXPTFsCallNativeNum, self.performance.fsCallNativeNum, self);
    WX_MONITOR_PERF_SET(WXPTFsCallNativeTime, self.performance.fsCallNativeTime, self);
    WX_MONITOR_PERF_SET(WXPTFsReqNetNum, self.performance.fsReqNetNum, self);
    WX_MONITOR_PERF_SET(WXPTTimerNum, self.performance.timerNum, self);
    WX_MONITOR_PERF_SET(WXPTFsComponentCount,self.performance.fsComponentCount,self);
    WX_MONITOR_PERF_SET(WXPTFsComponentCreateTime,self.performance.fsComponentCreateTime,self);
}


- (void)updatePerDicBeforExit
{
    WX_MONITOR_PERF_SET(WXPTCellExceedNum,self.performance.cellExceedNum,self);
    WX_MONITOR_PERF_SET(WXPTMaxDeepVDom,self.performance.maxVdomDeep,self);
    WX_MONITOR_PERF_SET(WXPTImgWrongSizeNum,self.performance.imgWrongSizeNum,self);
    WX_MONITOR_PERF_SET(WXPTInteractionTime,self.performance.interactionTime,self);
    WX_MONITOR_PERF_SET(WXPTComponentCount,self.performance.componentCount,self);
    WX_MONITOR_PERF_SET(WXPTComponentCreateTime,self.performance.componentCreateTime,self);
}

@end
