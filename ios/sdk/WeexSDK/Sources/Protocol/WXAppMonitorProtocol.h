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

#import "WXModuleProtocol.h"

/**  dimenValue  */
#define BIZTYPE             @"bizType"
#define PAGENAME            @"pageName"
#define WXSDKVERSION        @"WXSDKVersion"
#define JSLIBVERSION        @"JSLibVersion"
#define JSLIBSIZE           @"JSLibSize"
#define WXREQUESTTYPE       @"requestType"
#define WXCONNECTIONTYPE    @"connectionType"
#define NETWORKTYPE         @"networkType"
#define CACHETYPE           @"cacheType"
#define WXCUSTOMMONITORINFO @"customMonitorInfo"

/**  measureValue  */

#define SDKINITTIME         @"SDKInitTime"
#define SDKINITINVOKETIME   @"SDKInitInvokeTime"
#define JSLIBINITTIME       @"JSLibInitTime"
#define JSTEMPLATESIZE      @"JSTemplateSize"
#define NETWORKTIME         @"networkTime"
#define COMMUNICATETIME     @"communicateTime"
#define SCREENRENDERTIME    @"screenRenderTime"
#define TOTALTIME           @"totalTime"
#define FIRSETSCREENJSFEXECUTETIME  @"firstScreenJSFExecuteTime"

#define CALLCREATEINSTANCETIME  @"callCreateInstanceTime"
#define COMMUNICATETOTALTIME    @"communicateTotalTime"
#define FSRENDERTIME    @"fsRenderTime"
#define COMPONENTCOUNT      @"componentCount"
#define M_COMPONENT_TIME    @"componentCreateTime"

#define CACHEPROCESSTIME    @"cacheProcessTime"
#define CACHERATIO          @"cacheRatio"

//todo new point
#define M_FS_CALL_JS_TIME       @"fsCallJsTotalTime"
#define M_FS_CALL_JS_NUM        @"fsCallJsTotalNum"
#define M_FS_CALL_NATIVE_TIME   @"fsCallNativeTotalTime"
#define M_FS_CALL_NATIVE_NUM    @"fsCallNativeTotalNum"
#define M_FS_CALL_EVENT_NUM     @"fsCallEventTotalNum"
#define M_FS_REQUEST_NET_NUM    @"fsRequestNum"
#define M_CELL_EXCEED_NUM       @"cellExceedNum"
#define M_MAX_DEEP_VDOM         @"maxDeepVDomLayer"
#define M_IMG_WRONG_SIZE_NUM    @"imgSizeCount"
#define M_TIMER_NUM             @"timerCount"
#define M_WRONG_IMG_NUM         @"imgSizeCount"
#define M_INTERACTION_TIME      @"interactionTime"
#define M_COMPONENT_FS_COUNT    @"fsComponentCount"
#define M_COMPONENT_FS_TIME     @"fsComponentCreateTime"



@protocol WXAppMonitorProtocol <WXModuleProtocol>

- (void)commitAppMonitorArgs:(NSDictionary *)args;

- (void)commitAppMonitorAlarm:(NSString *)pageName monitorPoint:(NSString *)monitorPoint success:(BOOL)success errorCode:(NSString *)errorCode errorMsg:(NSString *)errorMsg arg:(NSString *)arg;

@optional

- (void)commitMonitorWithPage:(NSString *)pageName monitorPoint:(NSString *)monitorPoint args:(NSDictionary *)args;

@end
