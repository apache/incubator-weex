//
//  WXAnimationLayout.m
//  WeexSDK
//
//  Created by LiuHongfeng on 2017/5/11.
//  Copyright © 2017年 taobao. All rights reserved.
//

#import "WXAnimationLayout.h"
<<<<<<< Updated upstream

@implementation WXAnimationLayout

=======
#import "WXSDKInstance_private.h"

@implementation WXAnimationLayoutInfo

@end

@implementation WXAnimationLayout

- (instancetype)init
{
    if (self = [super init]) {
        _widthInfo = [[WXAnimationLayoutInfo alloc] init];
        _heightInfo = [[WXAnimationLayoutInfo alloc] init];
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
        NSLog(@"currentValue:%f",currentValue);

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

>>>>>>> Stashed changes
@end
