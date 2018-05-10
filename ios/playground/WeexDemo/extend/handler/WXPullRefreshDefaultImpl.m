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
#import <Masonry/Masonry.h>
#import <WeexSDK/WXUtility.h>

#import "QNWaveLayer.h"
#import "QNRotateImageView.h"

@interface WXPullRefreshDefaultImpl ()
@property (nonatomic, weak) UIView *pullContentView;
@property (nonatomic, weak) UIView *refreshContentView;
@property (nonatomic, weak) UIView *finalContentView;
@property (nonatomic, weak) NSTimer *refreshTimer;
@property (nonatomic, weak) NSTimer *finalTimer;

@property (nonatomic, strong) UILabel *timeLabel;
@property (nonatomic, strong) QNWaveLayer *waveLayer;
@property (nonatomic, strong) QNRotateImageView *imageView;
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
    [aView mas_makeConstraints:^(MASConstraintMaker *make) {
        make.top.left.mas_equalTo(inSuperView);
        make.bottom.right.mas_equalTo(inSuperView);
    }];
    _pullContentView = aView;

    UIImageView *imageView = [[UIImageView alloc] init];
    imageView.contentMode = UIViewContentModeScaleToFill;
    imageView.image = [UIImage imageNamed:@"weex"];
    [aView addSubview:imageView];
    [imageView mas_makeConstraints:^(MASConstraintMaker *make) {
        make.center.mas_equalTo(aView);
        make.width.mas_equalTo(@80);
        make.height.mas_equalTo(aView);
    }];
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
    [aView mas_makeConstraints:^(MASConstraintMaker *make) {
        make.top.left.mas_equalTo(inSuperView);
        make.bottom.right.mas_equalTo(inSuperView);
    }];
    _refreshContentView = aView;

    _imageView = [[QNRotateImageView alloc] init];
    _imageView.contentMode = UIViewContentModeScaleAspectFit;
    _imageView.image = [UIImage imageNamed:@"weex-icon"];
    [aView addSubview:_imageView];

    [_imageView mas_makeConstraints:^(MASConstraintMaker *make) {
        make.center.mas_equalTo(aView);
        make.width.mas_equalTo(aView.mas_width).multipliedBy(0.7);
        make.height.mas_equalTo(aView.mas_height).multipliedBy(0.7);
    }];

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
    [aView mas_makeConstraints:^(MASConstraintMaker *make) {
        make.top.left.mas_equalTo(inSuperView);
        make.bottom.right.mas_equalTo(inSuperView);
    }];
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

- (QNWaveLayer *)waveLayer {

    if (nil == _waveLayer) {
        _waveLayer = [[QNWaveLayer alloc] init];
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
