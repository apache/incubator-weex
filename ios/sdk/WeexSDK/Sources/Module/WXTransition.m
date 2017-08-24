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

#define SOLVE_EPS(dur) (1. / (1000. * (dur)))

#import <QuartzCore/CATransaction.h>
#import <QuartzCore/CADisplayLink.h>
#import "WXComponentManager.h"
#import "WXSDKInstance.h"
#import "WXComponent+Layout.h"
#import "WXComponent_internal.h"
#import "WXTransition.h"
#import "WXUtility.h"
#import "WXAssert.h"
#import "WXSDKInstance_private.h"
#import "WXLength.h"

@implementation WXLayoutAnimationInfo

@end

@interface WXTransition()
{
    WXComponent *_targetComponent;
    
    double ax;
    double bx;
    double cx;
    
    double ay;
    double by;
    double cy;
    
    float _layoutAnimationDuration;
    float _layoutAnimationDelay;
    NSUInteger _layoutAnimationCount;
    
    CAMediaTimingFunction *_layoutAnimationTimingFunction;
    CADisplayLink *_layoutAnimationDisplayLink;

    NSMutableDictionary *_toStyles;
    NSMutableDictionary *_fromStyles;
    NSMutableDictionary *_addStyles;
}

@end

@implementation WXTransition

#pragma mark - HandleStyle
- (void)_handleTransitionWithStyles:(NSDictionary *)styles withTarget:(WXComponent *)targetComponent
{
    if ([self _isLayoutAnimationRunning]) {
        [self _rollBackTransitionWithStyles:styles];
    }
    else
    {
        [self _suspendLayoutAnimationDisplayLink];
    }
    
    _targetComponent = targetComponent;
    if (!_fromStyles) {
        _fromStyles = [NSMutableDictionary dictionaryWithDictionary:targetComponent.styles];
        _addStyles = [NSMutableDictionary dictionaryWithDictionary:styles];
    }
    else
    {
        [_addStyles addEntriesFromDictionary:styles];
    }
    _toStyles = [NSMutableDictionary dictionaryWithDictionary:_fromStyles];
    [_toStyles addEntriesFromDictionary:_addStyles];
    
    
    _layoutAnimationDuration = _fromStyles[kWXTransitionDuration] ? [WXConvert CGFloat:_fromStyles[kWXTransitionDuration]] : 0;
    _layoutAnimationDelay = _fromStyles[kWXTransitionDelay] ? [WXConvert CGFloat:_fromStyles[kWXTransitionDelay]] : 0;
    _layoutAnimationTimingFunction = [WXConvert CAMediaTimingFunction:_fromStyles[kWXTransitionTimingFunction]];
    
    if (_layoutAnimationDuration == 0 ) {
        [targetComponent _updateCSSNodeStyles:styles];
        WXPerformBlockOnMainThread(^{
            [targetComponent _updateViewStyles:styles];
        });
        return;
    }
    
    if (![[NSString stringWithFormat:@"%@",_layoutAnimationTimingFunction] isEqualToString: kCAMediaTimingFunctionLinear]) {
        float vec[4] = {0.};
        [_layoutAnimationTimingFunction getControlPointAtIndex:1 values:&vec[0]];
        [_layoutAnimationTimingFunction getControlPointAtIndex:2 values:&vec[2]];
        [self unitBezierp1x:vec[0] p1y:vec[1] p2x:vec[2] p2y:vec[3]];
    }
    
    NSString *layoutAnimationProperty = _fromStyles[kWXTransitionProperty];
    [self _resloveTransitionProperty:layoutAnimationProperty withStyles:styles];
    [self performSelector:@selector(_startLayoutAnimationDisplayLink) withObject:self afterDelay:_layoutAnimationDelay/1000];
}

- (void)_rollBackTransitionWithStyles:(NSDictionary *)styles
{
    _layoutAnimationDuration = _layoutAnimationCount * 1000 / 60;
    _layoutAnimationCount = 0;
    _propertyArray = nil;
}

- (void)_resloveTransitionProperty:(NSString *)propertyNames withStyles:(NSDictionary *)styles
{
    NSArray *array = @[@"width",@"height",@"top",@"bottom",@"right",@"left",@"opacity"];
    for (NSString *propertyName in array) {
        if ([propertyNames containsString:propertyName]) {
            [self _judgeProperty:propertyName ];
        }
    }
    NSArray *animationModuleArray = @[@"transform",@"backgroundColor"];
    for (NSString *propertyName in animationModuleArray) {
        if ([propertyNames containsString:propertyName]) {
            [self _dealWithAnimationModuleProperty:propertyName styles:styles];
        }
    }
}

- (void)_judgeProperty:(NSString *)singleProperty
{
    WXLayoutAnimationInfo *info = [WXLayoutAnimationInfo new];
    info.fromValue = @(_fromStyles[singleProperty] ? [WXConvert CGFloat:_fromStyles[singleProperty]] : 0);
    info.toValue = @(_toStyles[singleProperty] ? [WXConvert CGFloat:_toStyles[singleProperty]] : 0 );
    info.perValue = @([info.toValue doubleValue] - [info.fromValue doubleValue]);
    info.propertyName = singleProperty;
    if (!_propertyArray) {
        _propertyArray = [NSMutableArray new];
    }
    [_propertyArray addObject:info];
}

- (void)_dealWithAnimationModuleProperty:(NSString *)singleProperty styles:(NSDictionary *)styles
{
    if (styles[singleProperty])
    {
        if (!_propertyArray) {
            _propertyArray = [NSMutableArray new];
        }
        
        if ([singleProperty isEqualToString:@"backgroundColor"]) {
            WXLayoutAnimationInfo *info = [WXLayoutAnimationInfo new];
            info.fromValue = [self _dealWithColor:[WXConvert UIColor:_fromStyles[singleProperty]]];
            info.toValue = [self _dealWithColor:[WXConvert UIColor:_toStyles[singleProperty]]];
            info.perValue = [self _calculatePerColorRGB1:info.toValue RGB2:info.fromValue];
            info.propertyName = singleProperty;
            [_propertyArray addObject:info];
        }
        if ([singleProperty isEqualToString:@"transform"]) {
            NSString *transformOrigin = styles[@"transformOrigin"];
            WXTransform *wxTransform = [[WXTransform alloc] initWithCSSValue:styles[singleProperty] origin:transformOrigin instance:_targetComponent.weexInstance];
            WXTransform *oldTransform = _targetComponent->_transform;
            if (wxTransform.rotateAngle != oldTransform.rotateAngle) {
                WXLayoutAnimationInfo *info = [WXLayoutAnimationInfo new];
                info.propertyName = @"transform.rotation";
                info.fromValue = @(oldTransform.rotateAngle);
                info.toValue = [NSNumber numberWithDouble:wxTransform.rotateAngle];
                info.perValue = @([info.toValue doubleValue] - [info.fromValue doubleValue]);
                [_propertyArray addObject:info];
            }
            if (wxTransform.rotateX != oldTransform.rotateX)
            {
                WXLayoutAnimationInfo *info = [WXLayoutAnimationInfo new];
                info.propertyName = @"transform.rotation.x";
                info.fromValue = @(oldTransform.rotateX);
                info.toValue = [NSNumber numberWithDouble:wxTransform.rotateX];
                info.perValue = @([info.toValue doubleValue] - [info.fromValue doubleValue]);
                [_propertyArray addObject:info];
            }
            if (wxTransform.rotateY != oldTransform.rotateY)
            {
                WXLayoutAnimationInfo *info = [WXLayoutAnimationInfo new];
                info.propertyName = @"transform.rotation.y";
                info.fromValue = @(oldTransform.rotateY);
                info.toValue = [NSNumber numberWithDouble:wxTransform.rotateY];
                info.perValue = @([info.toValue doubleValue] - [info.fromValue doubleValue]);
                [_propertyArray addObject:info];
            }
            if (wxTransform.rotateZ != oldTransform.rotateZ)
            {
                WXLayoutAnimationInfo *info = [WXLayoutAnimationInfo new];
                info.propertyName = @"transform.rotation.z";
                info.fromValue = @(oldTransform.rotateZ);
                info.toValue = [NSNumber numberWithDouble:wxTransform.rotateZ];
                info.perValue = @([info.toValue doubleValue] - [info.fromValue doubleValue]);
                [_propertyArray addObject:info];
            }
            
            if (wxTransform.scaleX != oldTransform.scaleX) {
                WXLayoutAnimationInfo *info = [WXLayoutAnimationInfo new];
                info.propertyName = @"transform.scale.x";
                info.fromValue = @(oldTransform.scaleX);
                info.toValue = @(wxTransform.scaleX);
                info.perValue = @([info.toValue doubleValue] - [info.fromValue doubleValue]);
                [_propertyArray addObject:info];
            }
            
            if (wxTransform.scaleY != oldTransform.scaleY) {
                WXLayoutAnimationInfo *info = [WXLayoutAnimationInfo new];
                info.propertyName = @"transform.scale.y";
                info.fromValue = @(oldTransform.scaleY);
                info.toValue = @(wxTransform.scaleX);
                info.perValue = @([info.toValue doubleValue] - [info.fromValue doubleValue]);
                [_propertyArray addObject:info];
            }
            
            if ((wxTransform.translateX && ![wxTransform.translateX isEqualToLength:oldTransform.translateX]) || (!wxTransform.translateX && oldTransform.translateX)) {
                WXLayoutAnimationInfo *info = [WXLayoutAnimationInfo new];
                info.propertyName = @"transform.translation.x";
                info.fromValue = @([oldTransform.translateX valueForMaximum:_targetComponent.view.bounds.size.width]);
                info.toValue = @([wxTransform.translateX valueForMaximum:_targetComponent.view.bounds.size.width]);
                info.perValue = @([info.toValue doubleValue] - [info.fromValue doubleValue]);
                [_propertyArray addObject:info];
            }
            
            if ((wxTransform.translateY && ![wxTransform.translateY isEqualToLength:oldTransform.translateY]) || (!wxTransform.translateY && oldTransform.translateY)) {
                WXLayoutAnimationInfo *info = [WXLayoutAnimationInfo new];
                info.propertyName = @"transform.translation.y";
                info.fromValue = @([oldTransform.translateY valueForMaximum:_targetComponent.view.bounds.size.height]);
                info.toValue = @([wxTransform.translateY valueForMaximum:_targetComponent.view.bounds.size.height]);
                info.perValue = @([info.toValue doubleValue] - [info.fromValue doubleValue]);
                [_propertyArray addObject:info];
            }
            _targetComponent->_transform = wxTransform;
        }
    }
}

- (NSArray *)_dealWithColor:(UIColor *)color
{
    CGFloat R, G, B, A;
    [color getRed:&R green:&G blue:&B alpha:&A];
    return @[@(R),@(G),@(B),@(A)];
}

- (NSArray *)_calculatePerColorRGB1:(NSArray *)RGB1 RGB2:(NSArray *)RGB2
{
    CGFloat R = [RGB1[0] doubleValue] - [RGB2[0] doubleValue];
    CGFloat G = [RGB1[1] doubleValue] - [RGB2[1] doubleValue];
    CGFloat B = [RGB1[2] doubleValue] - [RGB2[2] doubleValue];
    CGFloat A = [RGB1[3] doubleValue] - [RGB2[3] doubleValue];
    return @[@(R),@(G),@(B),@(A)];
}

- (void)_calculateLayoutAnimationProcessingStyle
{
    if (_propertyArray.count == 0) {
        return;
    }
    double per = 1000 * (_layoutAnimationCount + 1 ) / (60 * _layoutAnimationDuration);//linear
    if (![[NSString stringWithFormat:@"%@",_layoutAnimationTimingFunction] isEqualToString: kCAMediaTimingFunctionLinear]) {
        per = [self solveWithx:((_layoutAnimationCount+2)*16)/_layoutAnimationDuration epsilon:SOLVE_EPS(_layoutAnimationDuration)];
    }
    for (WXLayoutAnimationInfo *info in _propertyArray) {
        if ([info.propertyName isEqualToString:@"backgroundColor"]) {
            NSArray *array = @[
                               @([info.fromValue[0] floatValue] + [info.perValue[0] floatValue] * per),
                               @([info.fromValue[1] floatValue] + [info.perValue[1] floatValue] * per),
                               @([info.fromValue[2] floatValue] + [info.perValue[2] floatValue] * per),
                               @([info.fromValue[3] floatValue] + [info.perValue[3] floatValue] * per)];
            UIColor *color = [UIColor colorWithRed:[array[0] floatValue] green:[array[1] floatValue] blue:[array[2] floatValue] alpha:[array[3] floatValue]];
            WXPerformBlockOnMainThread(^{
                _targetComponent.view.backgroundColor = color;
                [_targetComponent.view setNeedsDisplay];
            });
            NSString *colorString = [WXConvert HexWithColor:color];
            [_fromStyles setObject:colorString forKey:info.propertyName];
        }
        else if ([info.propertyName hasPrefix:@"transform"])
        {
            double currentValue = [info.fromValue doubleValue] + [info.perValue doubleValue] * per;
            NSString *transformString = [NSString string];
            if ([info.propertyName isEqualToString:@"transform.rotation"]) {
                transformString = [NSString stringWithFormat:@"rotate(%lfdeg)",currentValue * 180.0 / M_PI];
            }
            if ([info.propertyName isEqualToString:@"transform.rotation.x"]) {
                transformString = [NSString stringWithFormat:@"rotateX(%lfdeg)",currentValue * 180.0 / M_PI];
            }
            if ([info.propertyName isEqualToString:@"transform.rotation.y"]) {
                transformString = [NSString stringWithFormat:@"rotateY(%lfdeg)",currentValue * 180.0 / M_PI];
            }
            if ([info.propertyName isEqualToString:@"transform.rotation.z"]) {
                transformString = [NSString stringWithFormat:@"rotateZ(%lfdeg)",currentValue * 180.0 / M_PI];
            }
            if ([info.propertyName isEqualToString:@"transform.scale.x"]) {
                transformString = [NSString stringWithFormat:@"scaleX(%lf)",currentValue];
            }
            if ([info.propertyName isEqualToString:@"transform.scale.y"]) {
                transformString = [NSString stringWithFormat:@"scaleY(%lf)",currentValue];
            }
            if ([info.propertyName isEqualToString:@"transform.translation.x"]) {
                transformString = [NSString stringWithFormat:@"translateX(%lfpx)",currentValue / _targetComponent.weexInstance.pixelScaleFactor];
            }
            if ([info.propertyName isEqualToString:@"transform.translation.y"]) {
                transformString = [NSString stringWithFormat:@"translateY(%lfpx)",currentValue / _targetComponent.weexInstance.pixelScaleFactor];
            }
            [_fromStyles setObject:transformString forKey:@"transform"];
        }
        else
        {
            double currentValue = [info.fromValue doubleValue] + [info.perValue doubleValue] * per;
            [_fromStyles setObject:@(currentValue) forKey:info.propertyName];
        }
    }
    WXPerformBlockOnMainThread(^{
        [_targetComponent _updateViewStyles:_fromStyles];
    });

    [_targetComponent _updateCSSNodeStyles:_fromStyles];
    [_targetComponent.weexInstance.componentManager startComponentTasks];
}

#pragma mark CADisplayLink
- (void)_startLayoutAnimationDisplayLink
{
    WXAssertComponentThread();
    if (!_layoutAnimationDisplayLink) {
        _layoutAnimationDisplayLink = [CADisplayLink displayLinkWithTarget:self selector:@selector(_handleLayoutAnimationDisplayLink)];
        [_layoutAnimationDisplayLink addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
    }
    else{
        [self _awakeLayoutAnimationDisplayLink];
    }
}

- (void)_stopLayoutAnimationDisplayLink
{
    WXAssertComponentThread();
    if (_layoutAnimationDisplayLink) {
        [_layoutAnimationDisplayLink invalidate];
        _layoutAnimationDisplayLink = nil;
    }
}

- (BOOL)_isLayoutAnimationRunning
{
    WXAssertComponentThread();
    BOOL yesOrNo = NO;
    if (!_layoutAnimationDisplayLink.paused && _layoutAnimationCount >= 5) {
        yesOrNo = YES;
    }
    return yesOrNo;
}

- (void)_suspendLayoutAnimationDisplayLink
{
    WXAssertComponentThread();
    if(_layoutAnimationDisplayLink && !_layoutAnimationDisplayLink.paused){
        _layoutAnimationDisplayLink.paused = YES;
    }
}

- (void)_awakeLayoutAnimationDisplayLink
{
    WXAssertComponentThread();
    if (_layoutAnimationDisplayLink && _layoutAnimationDisplayLink.paused) {
        _layoutAnimationDisplayLink.paused = NO;
    }
}

- (void)_handleLayoutAnimationDisplayLink
{
    WXAssertComponentThread();
    int count = _layoutAnimationDuration * 60 / 1000;
    if (_layoutAnimationCount >= count) {
        [self _suspendLayoutAnimationDisplayLink];
        [self _resetProcessAnimationParameter];
        return;
    }
    else
    {
        [self _calculateLayoutAnimationProcessingStyle];
    }
    _layoutAnimationCount ++;
}

- (void)_resetProcessAnimationParameter
{
    _layoutAnimationCount = 0;
    _layoutAnimationDuration = 0;
    _propertyArray = nil;

    _addStyles = nil;
    _fromStyles = nil;
    _toStyles = nil;
}

- (NSMutableDictionary *)_addStyles
{
    return self.addStyles;
}

- (NSMutableDictionary *)_fromStyles
{
    return self.fromStyles;
}

#pragma mark UnitBezierp
- (void)unitBezierp1x:(double)p1x p1y:(double)p1y p2x:(double)p2x p2y:(double)p2y
{
    cx = 3.0 * p1x;
    bx = 3.0 * (p2x - p1x) - cx;
    ax = 1.0 - cx -bx;
    
    cy = 3.0 * p1y;
    by = 3.0 * (p2y - p1y) - cy;
    ay = 1.0 - cy - by;
}

- (double)sampleCurveX:(double)t
{
    return ((ax * t + bx) * t + cx) * t;
}

- (double)sampleCurveY:(double)t
{
    return ((ay * t + by) * t + cy) * t;
}

- (double)sampleCurveDerivativeX:(double)t
{
    return (3.0 * ax * t + 2.0 * bx) * t + cx;
}

- (double)solveCurveX:(double)x epsilon:(double)epsilon
{
    double t0;
    double t1;
    double t2;
    double x2;
    double d2;
    int i;
    
    for (t2 = x, i = 0; i < 8; i++) {
        x2 = [self sampleCurveX:t2] - x;
        if (fabs (x2) < epsilon)
            return t2;
        d2 = [self sampleCurveDerivativeX:t2];
        if (fabs(d2) < 1e-6)
            break;
        t2 = t2 - x2 / d2;
    }
    t0 = 0.0;
    t1 = 1.0;
    t2 = x;
    
    if (t2 < t0)
        return t0;
    if (t2 > t1)
        return t1;
    
    while (t0 < t1) {
        x2 = [self sampleCurveX:t2];
        if (fabs(x2 - x) < epsilon)
            return t2;
        if (x > x2)
            t0 = t2;
        else
            t1 = t2;
        t2 = (t1 - t0) * .5 + t0;
    }
    return t2;
}

- (double)solveWithx:(double)x epsilon:(double)epsilon
{
    return [self sampleCurveY:([self solveCurveX:x epsilon:epsilon])];
}

@end
