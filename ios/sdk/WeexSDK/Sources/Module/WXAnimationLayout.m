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

#import "WXAnimationLayout.h"
#import "WXSDKInstance_private.h"

@implementation WXAnimationLayoutInfo

@end

@implementation WXAnimationLayout

- (instancetype)init
{
    if (self = [super init]) {
    
    }
    return self;
}

- (void)layoutForAnimation
{
    self.animationStartDate = [NSDate date];
    if (_animationDelay > 0) {
        [self performSelector:@selector(startUpdateStyleTimer) withObject:nil afterDelay:_animationDelay/1000];
    } else {
        [self startUpdateStyleTimer];
    }
}

#pragma mark UpdateStyle Methods

- (void)startUpdateStyleTimer
{
    if (!self.updateStyleTimer || ![self.updateStyleTimer isValid]) {
        __weak __typeof__(self) weakSelf = self;
        self.updateStyleTimer = [NSTimer wx_scheduledTimerWithTimeInterval:16/1000.0f block:^() {
            [weakSelf updateStyleOnTimer];
        } repeats:YES];
        [[NSRunLoop currentRunLoop] addTimer:self.updateStyleTimer forMode:NSRunLoopCommonModes];
    }
}

- (void)stopUpdateStyleTimer
{
    if (self.updateStyleTimer && [self.updateStyleTimer isValid]) {
        [self.updateStyleTimer invalidate];
        self.updateStyleTimer = nil;
    }
}

- (void)updateStyleOnTimer
{
    NSTimeInterval startMsecond = [_animationStartDate timeIntervalSince1970]*1000;
    NSTimeInterval nowMsecond = [[NSDate date] timeIntervalSince1970]*1000;
    NSTimeInterval interval = nowMsecond - startMsecond;
    if (!(_widthInfo || _heightInfo)) {
        [self stopUpdateStyleTimer];
        return;
    }
    if (interval > _animationDuration + _animationDelay) {
        [self stopUpdateStyleTimer];
        return;
    }
    CGFloat scaleFactor = self.weexInstance.pixelScaleFactor;
    _needUpdateStyles = [[NSMutableDictionary alloc] init];
    if (_widthInfo) {
        double currentValue = (([_widthInfo.toValue doubleValue] - [_widthInfo.fromValue doubleValue]) * ((interval - _animationDelay) / _animationDuration) + [_widthInfo.fromValue doubleValue]) / scaleFactor;
        [_needUpdateStyles setValue:[NSNumber numberWithDouble:currentValue] forKey:@"width"];
    }
    if (_heightInfo) {
        double currentValue = (([_heightInfo.toValue doubleValue] - [_heightInfo.fromValue doubleValue]) * ((interval - _animationDelay) / _animationDuration) + [_heightInfo.fromValue doubleValue]) / scaleFactor;
        [_needUpdateStyles setValue:[NSNumber numberWithDouble:currentValue] forKey:@"height"];
    }
    [self updateStyle:_needUpdateStyles];
}

- (void)updateStyle:(NSDictionary *)styles
{
    if ([styles count]>0) {
        __weak typeof(self) weakSelf = self;
        WXPerformBlockOnComponentThread(^{
            WXComponentManager *manager = weakSelf.weexInstance.componentManager;
            if (!manager.isValid) {
                return;
            }
            [manager updateStyles:styles forComponent:_targetComponent.ref];
            [manager startComponentTasks];
        });
    }
}

- (void)dealloc
{
    [self stopUpdateStyleTimer];
    [NSObject cancelPreviousPerformRequestsWithTarget:self];
}

@end
