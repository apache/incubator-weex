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

#import "WXPullRefreshDefaultImpl.h"
#import <UIKit/UIKit.h>
#import <WeexSDK/WXUtility.h>

#pragma mark WXRotateImageView
@interface WXRotateImageView : UIImageView
@property (assign) CGFloat currentAngle;
- (void)startRotate;
@end

#pragma mark WXWaveLayer
typedef void (^WXWaveLayerOverBlock)();
@interface WXWaveLayer : CALayer
@property (nonatomic, assign) CGFloat radius;
@property (nonatomic, assign) NSTimeInterval animationDuration;
- (void)startAnimation:(WXWaveLayerOverBlock)block;
@end

#pragma mark implementation WXPullRefreshDefaultImpl
@interface WXPullRefreshDefaultImpl ()
@property (nonatomic, weak) UIView *pullContentView;
@property (nonatomic, weak) UIView *refreshContentView;
@property (nonatomic, weak) UIView *finalContentView;
@property (nonatomic, weak) NSTimer *refreshTimer;
@property (nonatomic, weak) NSTimer *finalTimer;

@property (nonatomic, strong) UILabel *timeLabel;
@property (nonatomic, strong) WXWaveLayer *waveLayer;
@property (nonatomic, strong) WXRotateImageView *imageView;
@property (nonatomic, strong) UIImageView *pullingIV;

@property (nonatomic, assign) CGRect oldFrame;

@end

@implementation WXPullRefreshDefaultImpl
- (void)startPulling:(UIView *)inSuperView
{
    if (inSuperView == nil) {
        return;
    }
    _oldFrame = inSuperView.frame;
    UIView *aView = [[UIView alloc] init];
    aView.backgroundColor = [UIColor greenColor];
    [inSuperView addSubview:aView];
    aView.autoresizingMask = UIViewAutoresizingFlexibleTopMargin|UIViewAutoresizingFlexibleBottomMargin|UIViewAutoresizingFlexibleHeight;
    [aView setFrame:inSuperView.bounds];
    _pullContentView = aView;

    UIImageView *imageView = [[UIImageView alloc] init];
    imageView.contentMode = UIViewContentModeScaleToFill;
    imageView.image = [UIImage imageNamed:@"weex"];
    [aView addSubview:imageView];
    imageView.autoresizingMask = UIViewAutoresizingFlexibleTopMargin|UIViewAutoresizingFlexibleBottomMargin|UIViewAutoresizingFlexibleHeight;
    CGRect bounds = inSuperView.bounds;
    bounds.origin.x = (bounds.size.width - 80)/2;
    bounds.size.width = 80;
    imageView.frame = bounds;
    _pullingIV = imageView;
}

- (void)pulling:(NSDictionary *)inParams
{
    CGFloat newHeight = ((NSNumber *)inParams[@"pullingDistance"]).floatValue;
    CGFloat scale = [WXUtility defaultPixelScaleFactor];
    CGRect tempRect = CGRectMake(0, 0, [UIScreen mainScreen].bounds.size.width, newHeight*scale);
    UIView *superView = [_pullContentView superview];
    [superView setFrame:tempRect];
}

- (void)endPulling
{
    if (_pullingIV) {
        [_pullingIV removeFromSuperview];
        _pullingIV = nil;
    }
    if (_pullContentView) {
        [_pullContentView removeFromSuperview];
        _pullContentView = nil;
    }
}

- (void)startRefreshing:(UIView *)inSuperView
{
    if (inSuperView == nil) {
        return;
    }
    [inSuperView setFrame:_oldFrame];

    UIView *aView = [[UIView alloc] init];
    aView.backgroundColor = [UIColor grayColor];
    [inSuperView addSubview:aView];
    aView.autoresizingMask = UIViewAutoresizingFlexibleTopMargin|UIViewAutoresizingFlexibleBottomMargin|UIViewAutoresizingFlexibleHeight;
    [aView setFrame:inSuperView.bounds];
    _refreshContentView = aView;

    _imageView = [[WXRotateImageView alloc] init];
    _imageView.contentMode = UIViewContentModeScaleAspectFit;
    _imageView.image = [UIImage imageNamed:@"weex-icon"];
    [aView addSubview:_imageView];
    CGRect bounds = inSuperView.bounds;
    bounds.origin.x = (bounds.size.width - 80)/2;
    bounds.size.width = 80;
    _imageView.frame = bounds;

    [_imageView startRotate];
}

- (void)endRefreshing
{
    if (_imageView) {
        [_imageView stopAnimating];
        [_imageView removeFromSuperview];
        _imageView = nil;
    }
    if (_refreshContentView) {
        [_refreshContentView removeFromSuperview];
        _refreshContentView = nil;
    }
}

- (void)startFinalAnimation:(UIView *)inSuperView duration:(CGFloat)inDuration
{
    if (inSuperView == nil) {
        return;
    }
    UIView *aView = [[UIView alloc] init];
    aView.backgroundColor = [UIColor blueColor];
    aView.clipsToBounds = YES;
    [inSuperView addSubview:aView];
    aView.autoresizingMask = UIViewAutoresizingFlexibleTopMargin|UIViewAutoresizingFlexibleBottomMargin|UIViewAutoresizingFlexibleHeight;
    [aView setFrame:inSuperView.bounds];
    _finalContentView = aView;
    UIView *bView = (UIView *)inSuperView;
    CGFloat duration = inDuration/1000;

    [self.waveLayer setRadius:[UIScreen mainScreen].bounds.size.width / 2 + 10];
    self.waveLayer.position = CGPointMake(bView.frame.size.width / 2, bView.frame.size.height / 2);
    self.waveLayer.animationDuration = duration/4;

    self.timeLabel.frame = CGRectMake(0, (bView.bounds.size.height - 16) / 2, bView.bounds.size.width, 16);
    [self updateTimeLabelShow:nil];
    self.timeLabel.alpha = 0.0;

    __weak typeof(self) weakself = self;
    [self.waveLayer startAnimation:^{
        [UIView animateWithDuration:duration/2 animations:^{
            weakself.timeLabel.alpha = 1.0f;
        }];
    }];
}

- (void)endFinalAnimation
{
    if (_timeLabel) {
        [_timeLabel removeFromSuperview];
        _timeLabel = nil;
    }
    if (_waveLayer) {
        [_waveLayer removeFromSuperlayer];
        _waveLayer = nil;
    }
    if (_finalContentView) {
        [_finalContentView removeFromSuperview];
        _finalContentView = nil;
    }
}

- (UILabel *)timeLabel {

    if (nil == _timeLabel) {

        _timeLabel = [[UILabel alloc] init];
        _timeLabel.font = [UIFont systemFontOfSize:14];
        _timeLabel.textColor = [UIColor whiteColor];
        _timeLabel.textAlignment = NSTextAlignmentCenter;
        [_finalContentView addSubview:_timeLabel];
    }

    return _timeLabel;
}

- (WXWaveLayer *)waveLayer {

    if (nil == _waveLayer) {
        _waveLayer = [[WXWaveLayer alloc] init];
        [_finalContentView.layer addSublayer:_waveLayer];
    }

    return _waveLayer;
}

- (void)updateTimeLabelShow:(NSString*)text {
    if (text) {
        self.timeLabel.text = text;
    }
    else{
        NSDate *lastUpdatedTime = [NSDate date];
        NSDateFormatter *formatter = [[NSDateFormatter alloc] init];
        formatter.dateFormat = @"HH:mm";

        NSString *time = [formatter stringFromDate:lastUpdatedTime];
        NSString *updateFormat = @"Last updated: %@";
        self.timeLabel.text = [NSString stringWithFormat:updateFormat, time];
    }
}

@end

#pragma mark implementation WXRotateImageView

@interface WXRotateImageView()

@property (assign, nonatomic) CATransform3D identityTransform;

@property (strong) NSTimer *timer;

@end

@implementation WXRotateImageView

- (id)init {

    self = [super init];
    if (nil == self) {

        return self;
    }

    [self _init];
    return self;
}

- (id)initWithFrame:(CGRect)frame {

    self = [super initWithFrame:frame];
    if (nil == self) {

        return self;
    }

    [self _init];
    return self;
}

- (void)awakeFromNib {

    [super awakeFromNib];
    [self _init];
}

- (void)_init {

    CATransform3D transform = CATransform3DIdentity;
    transform.m34 = 1.0 / -500;
    self.identityTransform = CATransform3DRotate(transform, 0, 0, 0, 1.0);
}

- (void)dealloc {

    [self finishRotate];
}

- (void)startRotate {

    [self finishRotate];
    self.timer = [NSTimer scheduledTimerWithTimeInterval:0.02 target:self selector:@selector(onTimer:) userInfo:nil repeats:YES];
}

- (void)finishRotate {

    [self.timer invalidate];
    self.timer = nil;
}

- (void)onTimer:(NSTimer *)timer {

    self.currentAngle += 0.04 * M_PI;
    [self applyEffect];
}

- (void)applyEffect {

    CATransform3D transform = CATransform3DRotate(self.identityTransform, self.currentAngle, 0, 0, 1.0);
    self.layer.transform = transform;
}


@end

#pragma mark implementation WXWaveLayer
@interface WXWaveLayer()<CAAnimationDelegate>

@property (nonatomic, strong) CAAnimationGroup *animationGroup;

@property (strong) CAKeyframeAnimation *keyFrameAnimation;

@property (copy) WXWaveLayerOverBlock block;

@end

@implementation WXWaveLayer

- (id)init {
    self = [super init];
    if (self) {

        self.contentsScale = [UIScreen mainScreen].scale;

        self.animationDuration = 0.25f;
        self.backgroundColor = [UIColor clearColor].CGColor;
    }
    return self;
}

- (void)startAnimation:(WXWaveLayerOverBlock)block {

    self.block = block;
    self.backgroundColor = [UIColor colorWithRed:0xFF/255.0f green:0x00/255.0f blue:0x00/255.0f alpha:1].CGColor;
    [self setupAnimationGroup];
    [self addAnimation:self.keyFrameAnimation forKey:@"pulse"];
}

- (void)stopAnimation {

    [self removeAllAnimations];
    self.animationGroup = nil;
}

- (void)reset {

    self.backgroundColor = [UIColor clearColor].CGColor;
}

- (void)setRadius:(CGFloat)radius {

    _radius = radius;

    CGPoint tempPos = self.position;

    CGFloat diameter = self.radius * 2;

    self.bounds = CGRectMake(0, 0, diameter, diameter);
    self.cornerRadius = self.radius;
    self.position = tempPos;
}

- (void)setupAnimationGroup {

    CAKeyframeAnimation *scaleAnimation = [CAKeyframeAnimation animationWithKeyPath:@"transform.scale.xy"];
    scaleAnimation.values = @[@0, @0.3, @0.8, @1];
    scaleAnimation.keyTimes = @[@0, @0.5, @0.9, @1];
    scaleAnimation.duration = self.animationDuration;
    scaleAnimation.delegate = self;

    self.keyFrameAnimation = scaleAnimation;
}

#pragma mark CAAnimationDelegate
- (void)animationDidStop:(CAAnimation *)anim finished:(BOOL)flag {

    [self stopAnimation];
    if (self.block) {

        self.block();
    }
}

@end
