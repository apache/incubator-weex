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
#import "WXAnimationModule.h"
#import "WXComponentManager.h"
#import "WXSDKInstance.h"
#import "WXComponent+Layout.h"
#import "WXComponent_internal.h"
#import "WXTransition.h"
#import "WXUtility.h"
#import "WXAssert.h"
#import "WXSDKInstance_private.h"

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

- (void)_handleTransitionWithStyles:(NSDictionary *)styles withTarget:(WXComponent *)targetComponent
{
    [self _suspendLayoutAnimationDisplayLink];
    _targetComponent = targetComponent;
    if (!_addStyles) {
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

- (void)_resloveTransitionProperty:(NSString *)propertyNames withStyles:(NSDictionary *)styles
{
    NSArray *array = @[@"width",@"height",@"top",@"bottom",@"right",@"left"];
    for (NSString *propertyName in array) {
        if ([propertyNames containsString:propertyName]) {
            [self _judgeProperty:propertyName ];
        }
    }
    NSArray *animationModuleArray = @[@"transform",@"backgroundColor",@"opacity"];
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
    if ([singleProperty isEqualToString:singleProperty]&&styles[singleProperty])
    {
        NSDictionary *args = @{@"delay":@(_layoutAnimationDelay),@"duration":@(_layoutAnimationDuration),@"styles":styles,@"timingFunction":_fromStyles[kWXTransitionTimingFunction]};
        [self _animationModuleHandleTransition:args];
    }
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
        double currentValue = [info.fromValue doubleValue] + [info.perValue doubleValue] * per;
        [_fromStyles setObject:@(currentValue) forKey:info.propertyName];
    }
    [_targetComponent _updateCSSNodeStyles:_fromStyles];
    [_targetComponent.weexInstance.componentManager startComponentTasks];
}

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

- (void)_suspendLayoutAnimationDisplayLink
{
    WXAssertComponentThread();
    if(_layoutAnimationDisplayLink && !_layoutAnimationDisplayLink.paused)
    {
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

}

- (void)_animationModuleHandleTransition:(NSDictionary *)args
{
    WXAnimationModule *animation = [WXAnimationModule new];
    WXPerformBlockOnMainThread(^{
        [animation animationModuleProcessAnimationWithArgs:args withWeexInstance:_targetComponent.weexInstance targetComponent:_targetComponent];
    });
}

- (NSMutableDictionary *)_addStyles
{
    return self.addStyles;
}

- (NSMutableDictionary *)_fromStyles
{
    return self.fromStyles;
}

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
