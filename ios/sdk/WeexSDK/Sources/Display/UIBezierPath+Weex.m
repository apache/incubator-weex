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

#import "UIBezierPath+Weex.h"
#import "WXUtility.h"

@implementation UIBezierPath (Weex)

// Approximation of control point positions on a bezier to simulate a quarter of a circle.
// This is 1-kappa, where kappa = 4 * (sqrt(2) - 1) / 3
static const float kCircleControlPoint = 0.447715;

+ (instancetype)wx_bezierPathWithRoundedRect:(CGRect)rect
                                     topLeft:(CGFloat)topLeftRadius
                                    topRight:(CGFloat)topRightRadius
                                  bottomLeft:(CGFloat)bottomLeftRadius
                                 bottomRight:(CGFloat)bottomRightRadius
{
    UIBezierPath *path = [UIBezierPath bezierPath];
    if(isnan(topLeftRadius) || isnan(topRightRadius) || isnan(bottomLeftRadius) || isnan(bottomRightRadius)) {
        return path;
    }
    if (![WXUtility isValidPoint:rect.origin] || isnan(rect.size.height) || isnan(rect.size.width)) {
        return path;
    }
    CGPoint topLeftPoint = CGPointMake(rect.origin.x + topLeftRadius, rect.origin.y);
    if (![WXUtility isValidPoint:topLeftPoint]) {
        return path;
    }
    [path moveToPoint:topLeftPoint];
    
    // +------------------+
    //  \\      top     //
    //   \\+----------+//
    CGPoint topRightPoint = CGPointMake(CGRectGetMaxX(rect) - topRightRadius, rect.origin.y);
    if (![WXUtility isValidPoint:topRightPoint]) {
        return path;
    }
    [path addLineToPoint:topRightPoint];
    if (topRightRadius > 0) {
        [path addCurveToPoint:CGPointMake(CGRectGetMaxX(rect), rect.origin.y + topRightRadius)
                controlPoint1:CGPointMake(CGRectGetMaxX(rect) - topRightRadius * kCircleControlPoint, rect.origin.y)
                controlPoint2:CGPointMake(CGRectGetMaxX(rect), rect.origin.y + topRightRadius * kCircleControlPoint)];
    }
    
    // +------------------+
    //  \\     top      //|
    //   \\+----------+// |
    //                |   |
    //                |rig|
    //                |ht |
    //                |   |
    //                 \\ |
    //                  \\|
    [path addLineToPoint:CGPointMake(CGRectGetMaxX(rect), CGRectGetMaxY(rect) - bottomRightRadius)];
    if (bottomRightRadius > 0) {
        [path addCurveToPoint:CGPointMake(CGRectGetMaxX(rect) - bottomRightRadius, CGRectGetMaxY(rect))
                controlPoint1:CGPointMake(CGRectGetMaxX(rect), CGRectGetMaxY(rect) - bottomRightRadius * kCircleControlPoint)
                controlPoint2:CGPointMake(CGRectGetMaxX(rect) - bottomRightRadius * kCircleControlPoint, CGRectGetMaxY(rect))];
    }
    
    // +------------------+
    //  \\     top      //|
    //   \\+----------+// |
    //                |   |
    //                |rig|
    //                |ht |
    //                |   |
    //   //+----------+\\ |
    //  //    bottom    \\|
    // +------------------+
    [path addLineToPoint:CGPointMake(rect.origin.x + bottomLeftRadius, CGRectGetMaxY(rect))];
    if (bottomLeftRadius > 0) {
        [path addCurveToPoint:CGPointMake(rect.origin.x, CGRectGetMaxY(rect) - bottomLeftRadius)
                controlPoint1:CGPointMake(rect.origin.x + bottomLeftRadius * kCircleControlPoint, CGRectGetMaxY(rect))
                controlPoint2:CGPointMake(rect.origin.x, CGRectGetMaxY(rect) - bottomLeftRadius * kCircleControlPoint)];
    }
    
    // +------------------+
    // |\\     top      //|
    // | \\+----------+// |
    // |   |          |   |
    // |lef|          |rig|
    // |t  |          |ht |
    // |   |          |   |
    // | //+----------+\\ |
    // |//    bottom    \\|
    // +------------------+
    [path addLineToPoint:CGPointMake(rect.origin.x, rect.origin.y + topLeftRadius)];
    if (topLeftRadius > 0) {
        [path addCurveToPoint:CGPointMake(rect.origin.x + topLeftRadius, rect.origin.y)
                controlPoint1:CGPointMake(rect.origin.x, rect.origin.y + topLeftRadius * kCircleControlPoint)
                controlPoint2:CGPointMake(rect.origin.x + topLeftRadius * kCircleControlPoint, rect.origin.y)];
    }
    
    
    return path;
}
@end
