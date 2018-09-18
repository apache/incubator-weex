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

//Deprecated, use @WXApmForInstance

#import "WXSDKInstance.h"
#import "WXImageComponent.h"

@interface WXPerformance : NSObject


@property (nonatomic, assign) double renderUnixTimeOrigin;
@property (nonatomic, assign) double renderTimeOrigin;
@property (nonatomic, assign) double jsCreateFinishTime;

//firsrScreen call jsMethod totalTime
@property (nonatomic, assign) double fsCallJsTime;
//firsrScreen call jsMethod totalNum
@property (nonatomic, assign) double fsCallJsNum;
//firsrScreen call nativeMethod totalTime
@property (nonatomic, assign) double fsCallNativeTime;
//firsrScreen call nativeMethod totalNum
@property (nonatomic, assign) double fsCallNativeNum;
//firsrScreen call event totalNum
@property (nonatomic, assign) double fsCallEventNum;
//firsrScreen request netWrork totalNum
@property (nonatomic, assign) double fsReqNetNum;
//
@property (nonatomic, assign) double cellExceedNum;
//max dom deep
@property (nonatomic, assign) double maxVdomDeep;
//unMatch img in imgView num
@property (nonatomic, assign) double imgWrongSizeNum;
//call setTimeOut Num
@property (nonatomic, assign) double timerNum;
//time of user could interace : firsrScreen view or async load veiew(net lazyload) are show)
@property (nonatomic, assign) double interactionTime;

//in interactionTime ,add component count
@property (nonatomic,assign) double interactionAddCount;
@property (nonatomic,assign) double interactionLimitAddOpCount;

//all componentCreate count in instance life
@property (nonatomic,assign) double componentCount;
//all componentCreate time in instance life
@property (nonatomic,assign) double componentCreateTime;

@property (nonatomic,assign) double newFsRenderTime;
@property (nonatomic,assign) long lastRealInteractionTime;
//for performance record

- (void)onViewLoad:(WXComponent *)targetComponent;
- (void)recordComponentCreatePerformance:(double) diffTime forComponent:(WXComponent *)targetComponent;
- (void)onInstanceRenderSuccess:(WXSDKInstance*) instance;

@end


@interface WXSDKInstance ()

@property (nonatomic, assign) BOOL isJSCreateFinish;
//Deprecated, use @WXApmForInstance
@property (nonatomic,strong) WXPerformance* performance;

@end

@interface WXSDKInstance (Performance)

- (void) updatePerDicAfterCreateFinish;
- (void) updatePerDicBeforExit;
@end


