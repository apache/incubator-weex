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
#import "WXRootView.h"
#import "WXLog.h"
#import "WXComponent_performance.h"
#import "WXTextComponent.h"
#import "WXCellComponent.h"
#import "WXImageComponent.h"
#import "WXUtility.h"
#import "WXAnalyzerCenter+Transfer.h"
#import "WXCoreBridge.h"

@interface WXPerformance()
@property (nonatomic, assign) bool hasRecordFsRenderTimeByPosition;
@property (nonatomic, assign) double interactionAddCountRecord;
@end

@implementation WXPerformance

- (void) recordComponentCreatePerformance:(double) diffTime forComponent:(WXComponent *)targetComponent
{
    self.componentCount++;
    self.componentCreateTime+=diffTime;
}

- (void)recordViewCreatePerformance:(double) diffTime {
    self.viewCreateTime += diffTime;
}

/** on UI thread **/
- (void)onViewLoad:(WXComponent *)targetComponent
{
    if (targetComponent.hasAdd || targetComponent.ignoreInteraction) {
        return;
    }
    targetComponent.hasAdd = true;
    double modifyTime =  CACurrentMediaTime()*1000;
    
    __weak WXComponent* weakComponent = targetComponent;
    __weak WXPerformance* weakSelf = self;
    
    dispatch_async(dispatch_get_main_queue(), ^{
        __strong WXComponent* strongComponent = weakComponent;
        __strong WXPerformance* strongSelf = weakSelf;
        
        if (nil == strongComponent || nil == strongSelf) {
            return;
        }
        
        if(![strongSelf _verifyComponent:strongComponent]){
            return;
        }
        strongSelf.interactionAddCountRecord++;
        [strongSelf _handleRenderTime:strongComponent withModifyTime:modifyTime];
    });
}

- (void) _handleRenderTime:(WXComponent*)targetComponent withModifyTime:(double)modifyTime
{
    if (nil == targetComponent) {
        return;
    }
    double diff = modifyTime - self.renderTimeOrigin;
    if (diff > 8000) {
        return;
    }
    if ([targetComponent.type isEqualToString:@"_root"]) {
        return;
    }
    
    if (diff < self.interactionTime) {
        //useless calulcate ,for imporove performance
        return;
    }
    
    CGRect absoluteFrame = [targetComponent.view.superview convertRect:targetComponent.view.frame toView:targetComponent.weexInstance.rootView];
    CGRect rootFrame = targetComponent.weexInstance.rootView.frame;
    
    if (!self.hasRecordFsRenderTimeByPosition && absoluteFrame.origin.y+absoluteFrame.size.height > rootFrame.size.height +1 && ![self _isViewGroup:targetComponent] ) {
        self.newFsRenderTime = diff;
        self.hasRecordFsRenderTimeByPosition = true;
        [targetComponent.weexInstance.apmInstance onStage:KEY_PAGE_STAGES_NEW_FSRENDER];
    }
    
    UIView *root = targetComponent.weexInstance.rootView;
    if (root && [root isKindOfClass:[WXRootView class]]) {
        WXRootView* wxRootView = (WXRootView *)root;
        if ([wxRootView isHasEvent]) {
            return;
        }
    }
    
    if ([targetComponent.type isEqualToString:@"videoplus"]) {
        return;
    }

    bool inScreen = CGRectIntersectsRect(rootFrame, absoluteFrame);
    if (!inScreen) {
        return;
    }
    
    if (!targetComponent.weexInstance.apmInstance.hasRecordFirstInterationView) {
        targetComponent.weexInstance.apmInstance.hasRecordFirstInterationView = YES;
        [targetComponent.weexInstance.apmInstance onStage:KEY_PAGE_STAGES_FIRST_INTERACTION_VIEW];
    }
    [WXAnalyzerCenter transferInteractionInfo:targetComponent];
    [targetComponent.weexInstance.apmInstance onStage:KEY_PAGE_STAGES_INTERACTION];
    self.interactionLimitAddOpCount++;
    self.interactionAddCount = self.interactionAddCountRecord;
    [targetComponent.weexInstance.apmInstance updateMaxStats:KEY_PAGE_STATS_I_SCREEN_VIEW_COUNT curMaxValue:self.interactionLimitAddOpCount];
    [targetComponent.weexInstance.apmInstance updateMaxStats:KEY_PAGE_STATS_I_ALL_VIEW_COUNT curMaxValue:self.interactionAddCount];
    [targetComponent.weexInstance.apmInstance updateMaxStats:KEY_PAGE_STATS_EXECUTE_JS_TIME curMaxValue:self.callJsTime];
    [targetComponent.weexInstance.apmInstance updateMaxStats:KEY_PAGE_STATS_CREATE_COMPONENT_TIME curMaxValue:self.componentCreateTime];
    [targetComponent.weexInstance.apmInstance updateMaxStats:KEY_PAGE_STATS_CREATE_VIEW_TIME curMaxValue:self.viewCreateTime];
    __weak WXComponent* weakComponent = targetComponent;
    WXPerformBlockOnComponentThread(^{
         __strong WXComponent* strongComponent = weakComponent;
        if (!strongComponent) {
            return;
        }
        [strongComponent.weexInstance.apmInstance updateMaxStats:KEY_PAGE_STATS_LAYOUT_TIME curMaxValue:[WXCoreBridge getLayoutTime:strongComponent.weexInstance.instanceId]];
    });

    self.interactionTime = self.interactionTime < diff ? diff :self.interactionTime;
}

- (bool) _verifyComponent:(WXComponent*)targetComponent
{
    bool isViewloaded = targetComponent && targetComponent.weexInstance && [targetComponent isViewLoaded];
    
    if (!isViewloaded) {
        return false;
    }
    
    UIView* curView = targetComponent.view;
    while (curView != nil) {
        if (curView == targetComponent.weexInstance.rootView) {
            return true;
        }
        curView = curView.superview;
    }
    return false;
}

- (bool) _isViewGroup:(WXComponent *)targetComponent
{
    UIView* targetView = targetComponent.view;
    
    bool isViewGroup = true;
    if ([targetComponent isKindOfClass:WXTextComponent.class]
        || [targetView isKindOfClass:UILabel.class]
        || [targetView isKindOfClass:UITextView.class]
        || [targetView isKindOfClass:UIPickerView.class]
        || [targetView isKindOfClass:UIProgressView.class]
        || [targetView isKindOfClass:UIPickerView.class]
        || [targetView isKindOfClass:UIImageView.class]
        || [targetView isKindOfClass:UIButton.class]
        || [targetView isKindOfClass:UIDatePicker.class]
        || [targetView isKindOfClass:UITextField.class]
        || [targetView isKindOfClass:UISwitch.class]
        || [targetView isKindOfClass:UIActivityIndicatorView.class]) {
        
        isViewGroup =false;
    }
    return isViewGroup;
}

- (void)onInstanceRenderSuccess:(WXSDKInstance*) instance;
{
    if (self.hasRecordFsRenderTimeByPosition) {
        return;
    }
    self.newFsRenderTime = CACurrentMediaTime()*1000 - self.renderTimeOrigin;
    [instance.apmInstance onStage:KEY_PAGE_STAGES_NEW_FSRENDER];
}

@end

@implementation WXSDKInstance (Performance)

- (void) updatePerDicAfterCreateFinish
{
    [WXMonitor performanceFinishWithState:DebugAfterFSFinish instance:self];
    self.performance.jsCreateFinishTime =  CACurrentMediaTime()*1000;
    self.isJSCreateFinish = TRUE;
    WX_MONITOR_PERF_SET(WXPTFsCallJsNum, self.performance.callJsNum, self);
    WX_MONITOR_PERF_SET(WXPTFsCallJsTime, self.performance.callJsTime, self);
    WX_MONITOR_PERF_SET(WXPTFsCallNativeNum, self.performance.fsCallNativeNum, self);
    WX_MONITOR_PERF_SET(WXPTFsCallNativeTime, self.performance.fsCallNativeTime, self);
    WX_MONITOR_PERF_SET(WXPTFsReqNetNum, self.performance.fsReqNetNum, self);
    WX_MONITOR_PERF_SET(WXPTTimerNum, self.performance.timerNum, self);
}


- (void)updatePerDicBeforExit
{
    WX_MONITOR_PERF_SET(WXPTCellExceedNum,self.performance.cellExceedNum,self);
    WX_MONITOR_PERF_SET(WXPTMaxDeepVDom,self.performance.maxVdomDeep,self);
    WX_MONITOR_PERF_SET(WXPTImgWrongSizeNum,self.performance.imgWrongSizeNum,self);
    WX_MONITOR_PERF_SET(WXPTInteractionTime,self.performance.interactionTime,self);
    WX_MONITOR_PERF_SET(WXPTComponentCount,self.performance.componentCount,self);
    WX_MONITOR_PERF_SET(WXPTComponentCreateTime,self.performance.componentCreateTime,self);
    WX_MONITOR_PERF_SET(WXPTInteractionAddCount,self.performance.interactionAddCount,self);
    WX_MONITOR_PERF_SET(WXPTInteractionLimitAddCount,self.performance.interactionLimitAddOpCount,self);
    WX_MONITOR_PERF_SET(WXPNewFSRenderTime,self.performance.newFsRenderTime,self);
}

@end
