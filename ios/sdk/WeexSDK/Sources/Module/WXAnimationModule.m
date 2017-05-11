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

#import "WXAnimationModule.h"
#import "WXSDKInstance_private.h"
#import "WXComponent_internal.h"
#import "WXConvert.h"
#import "WXTransform.h"
#import "WXUtility.h"
#import "WXLength.h"
#import "NSTimer+Weex.h"

@interface WXAnimationInfo : NSObject<NSCopying>

@property (nonatomic, weak) WXComponent *target;
@property (nonatomic, strong) NSString *propertyName;
@property (nonatomic, strong) id fromValue;
@property (nonatomic, strong) id toValue;
@property (nonatomic, assign) double duration;
@property (nonatomic, assign) double delay;
@property (nonatomic, strong) CAMediaTimingFunction *timingFunction;
@property (nonatomic, assign) CGPoint originAnchorPoint;

@end

@implementation WXAnimationInfo

- (id)copyWithZone:(NSZone *)zone
{
    WXAnimationInfo *info = [[WXAnimationInfo allocWithZone:zone] init];
    
    info.target = self.target;
    info.propertyName = self.propertyName;
    info.fromValue = self.fromValue;
    info.toValue = self.toValue;
    info.duration = self.duration;
    info.delay = self.delay;
    info.timingFunction = self.timingFunction;
    
    return info;
}

@end

#if __IPHONE_OS_VERSION_MAX_ALLOWED < 100000
// CAAnimationDelegate is not available before iOS 10 SDK
@interface WXAnimationDelegate : NSObject 
#else
@interface WXAnimationDelegate : NSObject <CAAnimationDelegate>
#endif

@property (nonatomic, copy) void (^finishBlock)(BOOL);
@property (nonatomic, strong) WXAnimationInfo *animationInfo;

- (instancetype)initWithAnimationInfo:(WXAnimationInfo *)info finishBlock:(void(^)(BOOL))finishBlock;

@end

@implementation WXAnimationDelegate

- (instancetype)initWithAnimationInfo:(WXAnimationInfo *)info finishBlock:(void (^)(BOOL))finishBlock
{
    if (self = [super init]) {
        _animationInfo = info;
        _finishBlock = finishBlock;
    }
    
    return self;
}

- (void)animationDidStart:(CAAnimation *)anim
{
    if (!_animationInfo.target || ![_animationInfo.target isViewLoaded]) {
        return;
    }
    
    if ([_animationInfo.propertyName hasPrefix:@"transform"]) {
        WXTransform *transform = _animationInfo.target->_transform;
        [transform applyTransformForView:_animationInfo.target.view];
    } else if ([_animationInfo.propertyName isEqualToString:@"backgroundColor"]) {
        _animationInfo.target.view.layer.backgroundColor = (__bridge CGColorRef _Nullable)(_animationInfo.toValue);
    } else if ([_animationInfo.propertyName isEqualToString:@"opacity"]) {
        _animationInfo.target.view.layer.opacity = [_animationInfo.toValue floatValue];
    } else if ([_animationInfo.propertyName hasPrefix:@"bounds.size"]) {
        CGRect newBounds = _animationInfo.target.view.layer.bounds;
        if ([_animationInfo.propertyName isEqualToString:@"bounds.size.width"]) {
            newBounds.size = CGSizeMake([_animationInfo.toValue floatValue], newBounds.size.height);
        }else if ([_animationInfo.propertyName isEqualToString:@"bounds.size.height"]) {
            newBounds.size = CGSizeMake(newBounds.size.width,[_animationInfo.toValue floatValue]);
        }
        _animationInfo.target.view.layer.bounds = newBounds;
    }
    NSDate *startTime = [NSDate date];
    NSDateFormatter *dateFormatter = [[NSDateFormatter alloc] init];
    [dateFormatter setDateFormat:@"YYYY-MM-dd hh:mm:ss:SSS"];
    NSString *startTimeStr = [dateFormatter stringFromDate:startTime];
    NSLog(@"animationDidStart:%@",startTimeStr);
}

- (void)animationDidStop:(CAAnimation *)anim finished:(BOOL)flag
{
    if (!_animationInfo.target) {
        return;
    }
    
    if ([_animationInfo.propertyName hasPrefix:@"bounds.size"]) {
        /*
         * http://ronnqvi.st/about-the-anchorpoint/
         */
        //
        CGRect originFrame = _animationInfo.target.view.layer.frame;
        _animationInfo.target.view.layer.anchorPoint = _animationInfo.originAnchorPoint;
        _animationInfo.target.view.layer.frame = originFrame;
    }
    [_animationInfo.target.layer removeAllAnimations];
    
    if (_finishBlock) {
        _finishBlock(flag);
    }
    NSDate *startTime = [NSDate date];
    NSDateFormatter *dateFormatter = [[NSDateFormatter alloc] init];
    [dateFormatter setDateFormat:@"YYYY-MM-dd hh:mm:ss:SSS"];
    NSString *startTimeStr = [dateFormatter stringFromDate:startTime];
    NSLog(@"animationDidStop:%@",startTimeStr);
}

@end

@interface WXAnimationModule ()

@property (nonatomic,retain) NSTimer *updateStyleTimer;
@property (nonatomic,retain) WXComponent *targetComponent;
@property (nonatomic,retain) NSDate *animationStartDate;
@property (nonatomic,retain) WXAnimationInfo *widthInfo;
@property (nonatomic,retain) WXAnimationInfo *heightInfo;
@property (nonatomic,assign) double animationDuration;
@property (nonatomic,assign) double animationDelay;
@property (nonatomic,retain) NSDictionary *needUpdateStyles;
@end

@implementation WXAnimationModule

@synthesize weexInstance;

WX_EXPORT_METHOD(@selector(transition:args:callback:))

- (void)transition:(NSString *)nodeRef args:(NSDictionary *)args callback:(WXModuleCallback)callback
{
    WXPerformBlockOnComponentThread(^{
        WXComponent *targetComponent = [self.weexInstance componentForRef:nodeRef];
        if (!targetComponent) {
            if (callback) {
                callback([NSString stringWithFormat:@"No component find for ref:%@", nodeRef]);
            }
            return;
        }
        
        WXPerformBlockOnMainThread(^{
            [self animation:targetComponent args:args callback:callback];
        });
    });
}

- (NSArray<WXAnimationInfo *> *)animationInfoArrayFromArgs:(NSDictionary *)args target:(WXComponent *)target
{
    UIView *view = target.view;
    CALayer *layer = target.layer;
    NSMutableArray<WXAnimationInfo *> *infos = [NSMutableArray new];

    double duration = [args[@"duration"] doubleValue] / 1000;
    _animationDuration = duration * 1000;
    double delay = [args[@"delay"] doubleValue] / 1000;
    _animationDelay = delay;
    CAMediaTimingFunction *timingFunction = [WXConvert CAMediaTimingFunction:args[@"timingFunction"]];
    NSDictionary *styles = args[@"styles"];
    NSLog(@"styles:%@",[styles description]);
    for (NSString *property in styles) {
        WXAnimationInfo *info = [WXAnimationInfo new];
        info.duration = duration;
        info.delay = delay;
        info.timingFunction = timingFunction;
        info.target = target;
        
        id value = styles[property];
        if ([property isEqualToString:@"transform"]) {
            NSString *transformOrigin = styles[@"transformOrigin"];
            WXTransform *wxTransform = [[WXTransform alloc] initWithCSSValue:value origin:transformOrigin instance:self.weexInstance];
            WXTransform *oldTransform = target->_transform;
            if (wxTransform.rotateAngle != oldTransform.rotateAngle) {
                WXAnimationInfo *newInfo = [info copy];
                newInfo.propertyName = @"transform.rotation";
                /**
                 Rotate >= 180 degree not working on UIView block animation, have not found any more elegant solution than using CAAnimation
                 See http://stackoverflow.com/questions/9844925/uiview-infinite-360-degree-rotation-animation
                 **/
                newInfo.fromValue = @(oldTransform.rotateAngle);
                newInfo.toValue = [NSNumber numberWithDouble:wxTransform.rotateAngle];
                [infos addObject:newInfo];
            }
            
            if (wxTransform.scaleX != oldTransform.scaleX) {
                WXAnimationInfo *newInfo = [info copy];
                newInfo.propertyName = @"transform.scale.x";
                newInfo.fromValue = @(oldTransform.scaleX);
                newInfo.toValue = @(wxTransform.scaleX);
                [infos addObject:newInfo];
            }
            
            if (wxTransform.scaleY != oldTransform.scaleY) {
                WXAnimationInfo *newInfo = [info copy];
                newInfo.propertyName = @"transform.scale.y";
                newInfo.fromValue = @(oldTransform.scaleY);
                newInfo.toValue = @(wxTransform.scaleX);
                [infos addObject:newInfo];
            }
            
            if ((wxTransform.translateX && ![wxTransform.translateX isEqualToLength:oldTransform.translateX]) || (!wxTransform.translateX && oldTransform.translateX)) {
                WXAnimationInfo *newInfo = [info copy];
                newInfo.propertyName = @"transform.translation.x";
                newInfo.fromValue = @([oldTransform.translateX valueForMaximum:view.bounds.size.width]);
                newInfo.toValue = @([wxTransform.translateX valueForMaximum:view.bounds.size.width]);
                [infos addObject:newInfo];
            }
            
            if ((wxTransform.translateY && ![wxTransform.translateY isEqualToLength:oldTransform.translateY]) || (!wxTransform.translateY && oldTransform.translateY)) {
                WXAnimationInfo *newInfo = [info copy];
                newInfo.propertyName = @"transform.translation.y";
                newInfo.fromValue = @([oldTransform.translateY valueForMaximum:view.bounds.size.height]);
                newInfo.toValue = @([wxTransform.translateY valueForMaximum:view.bounds.size.height]);
                [infos addObject:newInfo];
            }
            
            target->_transform = wxTransform;
        } else if ([property isEqualToString:@"backgroundColor"]) {
            info.propertyName = @"backgroundColor";
            info.fromValue = (__bridge id)(layer.backgroundColor);
            info.toValue = (__bridge id)[WXConvert CGColor:value];
            [infos addObject:info];
        } else if ([property isEqualToString:@"opacity"]) {
            info.propertyName = @"opacity";
            info.fromValue = @(layer.opacity);
            info.toValue = @([value floatValue]);
            [infos addObject:info];
        } else if ([property isEqualToString:@"width"]) {
            info.propertyName = @"bounds.size.width";
            info.fromValue = @(layer.bounds.size.width);
            CGRect newBounds = layer.bounds;
            newBounds.size = CGSizeMake([WXConvert WXPixelType:value scaleFactor:self.weexInstance.pixelScaleFactor], newBounds.size.height);
            info.toValue = @(newBounds.size.width);
            [infos addObject:info];
            _widthInfo = info;
        } else if ([property isEqualToString:@"height"]) {
            info.propertyName = @"bounds.size.height";
            info.fromValue = @(layer.bounds.size.height);
            CGRect newBounds = layer.bounds;
            newBounds.size = CGSizeMake(newBounds.size.width, [WXConvert WXPixelType:value scaleFactor:self.weexInstance.pixelScaleFactor]);
            info.toValue = @(newBounds.size.height);
            [infos addObject:info];
            _heightInfo = info;
        }
    }

    return infos;
}


- (void)animation:(WXComponent *)targetComponent args:(NSDictionary *)args callback:(WXModuleCallback)callback
{
    /**
       UIView-style animation functions support the standard timing functions,
       but they don’t allow you to specify your own cubic Bézier curve. 
       CATransaction can be used instead to force these animations to use the supplied CAMediaTimingFunction to pace animations.
     **/
    [CATransaction begin];
    [CATransaction setAnimationTimingFunction:[WXConvert CAMediaTimingFunction:args[@"timingFunction"]]];
    [CATransaction setCompletionBlock:^{
        if (callback) {
            callback(@"SUCCESS");
//            [self updateHeight:targetComponent];
        }
    }];
    NSArray<WXAnimationInfo *> *infos = [self animationInfoArrayFromArgs:args target:targetComponent];
    for (WXAnimationInfo *info in infos) {
        [self _createCAAnimation:info];
    }
    
    [CATransaction commit];
    _targetComponent = targetComponent;
    _animationStartDate = [NSDate date];
    [self startUpdateStyleTimer];
    //log
    NSDateFormatter *dateFormatter = [[NSDateFormatter alloc] init];
    [dateFormatter setDateFormat:@"YYYY-MM-dd hh:mm:ss:SSS"];
    NSString *startTimeStr = [dateFormatter stringFromDate:_animationStartDate];
    NSLog(@"startUpdateStyleTimer:%@",startTimeStr);
}

#pragma mark Private Methods

- (void)startUpdateStyleTimer
{
    if (!self.updateStyleTimer || ![self.updateStyleTimer isValid]) {
        __weak __typeof__(self) weakSelf = self;
        self.updateStyleTimer = [NSTimer wx_scheduledTimerWithTimeInterval:16/1000.0f block:^() {
            [weakSelf updateStyleOnTimer];
        } repeats:YES];
        [[NSRunLoop currentRunLoop] addTimer:self.updateStyleTimer forMode:NSRunLoopCommonModes];
//        _targetComponent.isSkipLayout = YES;
    }
}

- (void)stopUpdateStyleTimer
{
    if (self.updateStyleTimer && [self.updateStyleTimer isValid]) {
        [self.updateStyleTimer invalidate];
        self.updateStyleTimer = nil;
//        _targetComponent.isSkipLayout = NO;
        //log
        NSDateFormatter *dateFormatter = [[NSDateFormatter alloc] init];
        [dateFormatter setDateFormat:@"YYYY-MM-dd hh:mm:ss:SSS"];
        NSString *nowStr = [dateFormatter stringFromDate:[NSDate date]];
        NSLog(@"stopUpdateStyleTimer time:%@",nowStr);
    }
}

- (void)updateStyleOnTimer
{
    NSTimeInterval startMsecond = [_animationStartDate timeIntervalSince1970]*1000;
    NSTimeInterval nowMsecond = [[NSDate date] timeIntervalSince1970]*1000;
    NSTimeInterval interval = nowMsecond - startMsecond;
    NSLog(@"interval:%f",interval);
    if (!(_widthInfo || _heightInfo)) {
        [self stopUpdateStyleTimer];
        return;
    }
    if (interval > _animationDuration) {
        [self stopUpdateStyleTimer];
        return;
    }
    CGFloat scaleFactor = self.weexInstance.pixelScaleFactor;
    _needUpdateStyles = [[NSMutableDictionary alloc] init];
    if (_widthInfo) {
        double currentValue = (([_widthInfo.toValue doubleValue] - [_widthInfo.fromValue doubleValue]) * (interval / _animationDuration) + [_widthInfo.fromValue doubleValue]) / scaleFactor;
        [_needUpdateStyles setValue:[NSNumber numberWithDouble:currentValue] forKey:@"width"];
    }
    if (_heightInfo) {
        double currentValue = (([_heightInfo.toValue doubleValue] - [_heightInfo.fromValue doubleValue]) * (interval / _animationDuration) + [_heightInfo.fromValue doubleValue]) / scaleFactor;
        [_needUpdateStyles setValue:[NSNumber numberWithDouble:currentValue] forKey:@"height"];
    }
    [self updateStyle:_needUpdateStyles];
    NSLog(@"_needUpdateStyles:%@",[_needUpdateStyles description]);
}

- (void)updateStyle:(NSDictionary *)styles
{
//    NSDictionary *styles = [NSDictionary dictionaryWithObjectsAndKeys:@"400",@"height", nil];
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
        //log
        NSDateFormatter *dateFormatter = [[NSDateFormatter alloc] init];
        [dateFormatter setDateFormat:@"YYYY-MM-dd hh:mm:ss:SSS"];
        NSString *nowStr = [dateFormatter stringFromDate:[NSDate date]];
        NSLog(@"updateStyles forComponent time:%@",nowStr);
    }

}



- (void)_createCAAnimation:(WXAnimationInfo *)info
{
    CABasicAnimation* animation = [CABasicAnimation animationWithKeyPath:info.propertyName];
    animation.fromValue = info.fromValue;
    animation.toValue = info.toValue;
    animation.duration = info.duration;
    animation.beginTime = CACurrentMediaTime() + info.delay;
    animation.timingFunction = info.timingFunction;
    animation.removedOnCompletion = NO;
    animation.fillMode = kCAFillModeForwards;
    
    WXAnimationDelegate *delegate = [[WXAnimationDelegate alloc] initWithAnimationInfo:info finishBlock:^(BOOL isFinish) {
        
    }];
    animation.delegate = delegate;
    
    CALayer *layer = info.target.layer;
    if ([info.propertyName hasPrefix:@"bounds"]) {
        info.originAnchorPoint = layer.anchorPoint;
        CGRect originFrame = layer.frame;
         /*
          * if anchorPoint changed, the origin of layer's frame will change
          * http://ronnqvi.st/about-the-anchorpoint/
         */
        layer.anchorPoint = CGPointZero;
        layer.frame = originFrame;
    }
    [layer addAnimation:animation forKey:info.propertyName];
}

- (void)dealloc
{
    NSLog(@"dealloc");
    [_updateStyleTimer invalidate];
}
@end
