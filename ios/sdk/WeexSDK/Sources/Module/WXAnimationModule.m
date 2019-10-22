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
#import "WXTransition.h"
#import "WXComponent+Layout.h"

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
    [self applyTransform];
}

-(void)applyTransform
{
    if (!_animationInfo.target || ![_animationInfo.target isViewLoaded]) {
        return;
    }
    if ([_animationInfo.propertyName hasPrefix:@"transform"]) {
        WXTransform *transform = _animationInfo.target->_transform;
        [transform applyTransformForView:_animationInfo.target.view];
        [_animationInfo.target _adjustForRTL];
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
    [_animationInfo.target.layer removeAnimationForKey:_animationInfo.propertyName];
    
    if (_finishBlock) {
        _finishBlock(flag);
    }
}

@end

@interface WXAnimationModule ()

@property (nonatomic, assign) BOOL isAnimationedSuccess;

@end

@implementation WXAnimationModule

@synthesize weexInstance;

WX_EXPORT_METHOD(@selector(transition:args:callback:))

- (void)transition:(NSString *)nodeRef args:(NSDictionary *)args callback:(WXModuleKeepAliveCallback)callback
{
    _isAnimationedSuccess = YES;
    WXPerformBlockOnComponentThread(^{
        if (nodeRef == nil || ![nodeRef isKindOfClass:[NSString class]] ||
            ![args isKindOfClass:[NSDictionary class]]) {
            if (callback) {
                NSDictionary *message = @{@"result":@"Fail",
                                          @"message":@"Argument type error."};
                callback(message, NO);
            }
            return;
        }
        
        NSArray *stringArray = [nodeRef componentsSeparatedByString:@"@"];
        if ([stringArray count] == 0) {
            if (callback) {
                NSDictionary *message = @{@"result":@"Fail",
                                          @"message":@"Node ref format error."};
                callback(message, NO);
            }
            return;
        }
        
        WXComponent *targetComponent = [self.weexInstance componentForRef:stringArray[0]];
        if (!targetComponent) {
            if (callback) {
                NSDictionary *message = @{@"result":@"Fail",
                                          @"message":[NSString stringWithFormat:@"No component find for ref:%@", nodeRef]};
                callback(message, NO);
            }
            return;
        }
        WXPerformBlockOnMainThread(^{
            [self animation:targetComponent args:args callback:callback];
        });
    });
}

- (NSArray<WXAnimationInfo *> *)animationInfoArrayFromArgs:(NSDictionary *)args target:(WXComponent *)target
                                                needLayout:(BOOL* _Nonnull)needLayout
                                                transition:(WXTransition* _Nonnull *)transition
                                             transitionDic:(NSMutableDictionary* _Nonnull *)transitionDic
{
    UIView *view = target.view;
    CALayer *layer = target.layer;
    NSMutableArray<WXAnimationInfo *> *infos = [NSMutableArray new];

    double duration = [args[@"duration"] doubleValue] / 1000;
    double delay = [args[@"delay"] doubleValue] / 1000;
    if (args[@"needLayout"]) {
        *needLayout = [WXConvert BOOL:args[@"needLayout"]];
        if (*needLayout) {
            *transition = [WXTransition new];
            *transitionDic = [NSMutableDictionary new];
            (*transition).filterStyles = [NSMutableDictionary new];
            (*transition).oldFilterStyles = [NSMutableDictionary new];
        }
    }
    CAMediaTimingFunction *timingFunction = [WXConvert CAMediaTimingFunction:args[@"timingFunction"]];
    NSDictionary *styles = args[@"styles"];
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
            if (wxTransform.rotateX != oldTransform.rotateX)
            {
                WXAnimationInfo *newInfo = [info copy];
                newInfo.propertyName = @"transform.rotation.x";
                newInfo.fromValue = @(oldTransform.rotateX);
                newInfo.toValue = [NSNumber numberWithDouble:wxTransform.rotateX];
                 [infos addObject:newInfo];
            }
            if (wxTransform.rotateY != oldTransform.rotateY)
            {
                WXAnimationInfo *newInfo = [info copy];
                newInfo.propertyName = @"transform.rotation.y";
                newInfo.fromValue = @(oldTransform.rotateY);
                newInfo.toValue = [NSNumber numberWithDouble:wxTransform.rotateY];
                [infos addObject:newInfo];
            }
            if (wxTransform.rotateZ != oldTransform.rotateZ)
            {
                WXAnimationInfo *newInfo = [info copy];
                newInfo.propertyName = @"transform.rotation.z";
                newInfo.fromValue = @(oldTransform.rotateZ);
                newInfo.toValue = [NSNumber numberWithDouble:wxTransform.rotateZ];
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
                newInfo.toValue = @(wxTransform.scaleY);
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
            target.transform = wxTransform;
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
            if (*needLayout) {
                [self transitionWithArgs:args withProperty:property target:target transition:*transition transitionDic:*transitionDic];
            }
            else
            {
                info.propertyName = @"bounds.size.width";
                info.fromValue = @(layer.bounds.size.width);
                CGRect newBounds = layer.bounds;
                newBounds.size = CGSizeMake([WXConvert WXPixelType:value scaleFactor:self.weexInstance.pixelScaleFactor], newBounds.size.height);
                info.toValue = @(newBounds.size.width);
                [infos addObject:info];
            }
        } else if ([property isEqualToString:@"height"]) {
            if (*needLayout) {
                [self transitionWithArgs:args withProperty:property target:target transition:*transition transitionDic:*transitionDic];
            }
            else
            {
                info.propertyName = @"bounds.size.height";
                info.fromValue = @(layer.bounds.size.height);
                CGRect newBounds = layer.bounds;
                newBounds.size = CGSizeMake(newBounds.size.width, [WXConvert WXPixelType:value scaleFactor:self.weexInstance.pixelScaleFactor]);
                info.toValue = @(newBounds.size.height);
                [infos addObject:info];
            }
        }
    }
    return infos;
}

- (void)transitionWithArgs:(NSDictionary *)args withProperty:(NSString *)property target:(WXComponent *)target
                transition:(WXTransition*)transition
             transitionDic:(NSMutableDictionary*)transitionDic
{
    if (args[@"styles"][property] == nil) {
        return;
    }
    
    [transition.filterStyles setObject:args[@"styles"][property] forKey:property];
    
    id oldStyleValue = target.styles[property];
    if (oldStyleValue == nil) {
        oldStyleValue = [target convertLayoutValueToStyleValue:property];
    }
    if (oldStyleValue == nil) {
        oldStyleValue = @"0.0";
    }
    [transition.oldFilterStyles setObject:oldStyleValue ?:@0 forKey:property];
    
    [target _modifyStyles:@{property:args[@"styles"][property]}];
    [transitionDic setObject:@([args[@"duration"] doubleValue]) forKey:kWXTransitionDuration];
    [transitionDic setObject:@([args[@"delay"] doubleValue]) forKey:kWXTransitionDelay];
    [transitionDic setObject:args[@"timingFunction"] ?: @"linear" forKey:kWXTransitionTimingFunction];
}

- (void)animation:(WXComponent *)targetComponent args:(NSDictionary *)args callback:(WXModuleKeepAliveCallback)callback
{
    /* Check if view of targetComponent is created, if not, we do not do animation and
     simulate delay of 'duration' and callback.
     For a view in list, the view migth be recycled, and if view is not attached to a window,
     the CATransaction completion block will be called immediately, which may cause CPU overload
     problem if JS code do any logic in the completion callback.
     */
    
    BOOL shouldDoAnimation = NO;
    if ([targetComponent isViewLoaded]) {
        UIView* view = targetComponent.view;
        if ([view window] != nil) {
            shouldDoAnimation = YES;
        }
    }
    
    /**
       UIView-style animation functions support the standard timing functions,
       but they don’t allow you to specify your own cubic Bézier curve. 
       CATransaction can be used instead to force these animations to use the supplied CAMediaTimingFunction to pace animations.
     **/
    [CATransaction begin];
    [CATransaction setAnimationTimingFunction:[WXConvert CAMediaTimingFunction:args[@"timingFunction"]]];
    
    if (shouldDoAnimation) {
        [CATransaction setCompletionBlock:^{
            if (callback) {
                NSDictionary *message;
                if (_isAnimationedSuccess) {
                    message = @{@"result":@"Success",
                                @"message":@"Success"};
                }
                else
                {
                    message = @{@"result":@"Fail",
                                @"message":@"Animation did not complete"};
                }
                callback(message, NO);
            }
        }];
    }
    else if (callback) {
        double duration = [[args objectForKey:@"duration"] doubleValue] / 1000.f;
        dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(duration * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
            callback(@{@"result":@"Success",
                       @"message":@"Success"}, NO);
        });
    }
    
    BOOL needLayout = NO;
    WXTransition* transition = nil;
    NSMutableDictionary* transitionDic = nil;
    NSArray<WXAnimationInfo *> *infos = [self animationInfoArrayFromArgs:args target:targetComponent needLayout:&needLayout transition:&transition transitionDic:&transitionDic];
    for (WXAnimationInfo *info in infos) {
        [self _createCAAnimation:info];
    }
    
    [CATransaction commit];
    
    if (needLayout && transition) {
        WXPerformBlockOnComponentThread(^{
            [transition _handleTransitionWithStyles:transitionDic resetStyles:nil target:targetComponent];
        });
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
        if (!isFinish) {
            _isAnimationedSuccess = isFinish;
        }
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
    
    if(!WXFloatGreaterThan(animation.duration, 0)){
        if([delegate respondsToSelector:@selector(applyTransform)]) {
            [delegate applyTransform];
        }
    } else {
        CATransform3D transform = layer.transform;
        if (info.target->_transform.perspective && !isinf(info.target->_transform.perspective)) { //!OCLint
            transform.m34 = -1.0/info.target->_transform.perspective*[UIScreen mainScreen].scale;
            layer.transform = transform;
        }
        [layer addAnimation:animation forKey:info.propertyName];
    }
}

@end
