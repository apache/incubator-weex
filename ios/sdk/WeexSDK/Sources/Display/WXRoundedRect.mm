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

#import "WXRoundedRect.h"
#import "WXAssert.h"

@implementation WXRadii

- (instancetype)initWithTopLeft:(CGFloat)topLeft
                       topRight:(CGFloat)topRight
                     bottomLeft:(CGFloat)bottomLeft
                    bottomRight:(CGFloat)bottomRight
{
    if (self = [super init]) {
        _topLeft = topLeft;
        _topRight = topRight;
        _bottomLeft = bottomLeft;
        _bottomRight = bottomRight;
    }
    
    return self;
}

- (BOOL)hasBorderRadius
{
    return _topLeft > 0.001 || _topRight > 0.001 || _bottomLeft > 0.001 || _bottomRight > 0.001;
}

- (BOOL)radiusesAreEqual
{
    return _topLeft == _topRight && _topRight == _bottomRight && _bottomRight == _bottomLeft;
}

- (void)scale:(float)factor
{
    if (factor == 1) {
        return;
    }
    
    _topLeft *= factor;
    _topRight *= factor;
    _bottomLeft *= factor;
    _bottomRight *= factor;
}

@end

@interface WXRoundedRect ()

@end

@implementation WXRoundedRect

- (instancetype)initWithRect:(CGRect)rect
                     topLeft:(CGFloat)topLeft
                    topRight:(CGFloat)topRight
                  bottomLeft:(CGFloat)bottomLeft
                 bottomRight:(CGFloat)bottomRight
{
    if (self = [super init]) {
        _rect = rect;
        _radii = [[WXRadii alloc] initWithTopLeft:topLeft topRight:topRight bottomLeft:bottomLeft bottomRight:bottomRight];
        [_radii scale:[self radiiConstraintScaleFactor]];
    }
    
    return self;
}

- (float)radiiConstraintScaleFactor
{
    // Constrain corner radii using CSS3 rules:
    // http://www.w3.org/TR/css3-background/#the-border-radius
    float factor = 1;
    CGFloat radiiSum;
    
    // top
    radiiSum = _radii.topLeft + _radii.topRight;
    if (radiiSum > _rect.size.width) {
        factor = MIN(_rect.size.width / radiiSum, factor);
    }
    
    // bottom
    radiiSum = _radii.bottomLeft + _radii.bottomRight;
    if (radiiSum > _rect.size.width) {
        factor = MIN(_rect.size.width / radiiSum, factor);
    }
    
    // left
    radiiSum = _radii.topLeft + _radii.bottomLeft;
    if (radiiSum > _rect.size.height) {
        factor = MIN(_rect.size.height / radiiSum, factor);
    }
    
    // right
    radiiSum = _radii.topRight + _radii.bottomRight;
    if (radiiSum > _rect.size.height) {
        factor = MIN(_rect.size.height / radiiSum, factor);
    }
    
    WXAssert(factor <= 1, @"Wrong factor for radii constraint scale:%f", factor);
    return factor;
}

@end
