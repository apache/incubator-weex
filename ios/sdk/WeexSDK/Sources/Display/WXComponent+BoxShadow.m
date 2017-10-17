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

#import "WXComponent+BoxShadow.h"
#import "WXBoxShadow.h"
#import "WXConvert.h"
#import "WXUtility.h"
#import "WXComponent_internal.h"
#import "UIBezierPath+Weex.h"
#import "WXRoundedRect.h"

@implementation WXComponent (BoxShadow)


- (WXBoxShadow *_Nullable)getViewBoxShadow:(UIView *_Nullable)view
{
    WXBoxShadow *boxShadow = [WXBoxShadow new];
    boxShadow.shadowColor = [UIColor colorWithCGColor:view.layer.shadowColor];
    boxShadow.shadowOffset = view.layer.shadowOffset;
    boxShadow.shadowRadius = view.layer.shadowRadius;
    boxShadow.shadowOpacity =  view.layer.shadowOpacity;
    return boxShadow;
}

- (void)resetViewLayer
{
    UIBezierPath *shadowPath = [UIBezierPath bezierPathWithRect:self.view.bounds];
    self.view.layer.masksToBounds = NO;
    self.view.layer.shadowColor = _originalBoxShadow.shadowColor.CGColor;
    self.view.layer.shadowOffset = _originalBoxShadow.shadowOffset;
    self.view.layer.shadowRadius = _originalBoxShadow.shadowRadius;
    self.view.layer.shadowOpacity = _originalBoxShadow.shadowOpacity;
    self.view.layer.shadowPath = shadowPath.CGPath;
    
    
    if (_lastBoxShadow.isInset) {
        if (_lastBoxShadow.innerLayer) {
            [_lastBoxShadow.innerLayer removeFromSuperlayer];
        }
    }
}

// if not equal return NO, if equal return YES
- (BOOL)equalBoxShadow:(WXBoxShadow *_Nullable)boxShadow withBoxShadow:(WXBoxShadow *_Nullable)compareBoxShadow
{
    if(!compareBoxShadow && !boxShadow) {
        return YES;
    } else if (CGColorEqualToColor(boxShadow.shadowColor.CGColor,compareBoxShadow.shadowColor.CGColor) &&
               CGSizeEqualToSize(boxShadow.shadowOffset,compareBoxShadow.shadowOffset) &&
               WXFloatEqual(boxShadow.shadowRadius,compareBoxShadow.shadowRadius)&& (boxShadow.isInset == compareBoxShadow.isInset))
    {
        return YES;
    }
    return NO;
    
}

- (void)configBoxShadow:(WXBoxShadow *_Nullable)boxShadow
{
    if (!_originalBoxShadow) {
        _originalBoxShadow = [self getViewBoxShadow:self.view];
    }
    if (!boxShadow && !_lastBoxShadow) {
        return;
    }
    [self resetViewLayer];
    if (!boxShadow) {
        return;
    }
    WXRoundedRect *borderRect = [[WXRoundedRect alloc] initWithRect:self.view.bounds topLeft:_borderTopLeftRadius topRight:_borderTopRightRadius bottomLeft:_borderBottomLeftRadius bottomRight:_borderBottomRightRadius];
    // here is computed radii, do not use original style
    WXRadii *radii = borderRect.radii;
    CGFloat topLeft = radii.topLeft, topRight = radii.topRight, bottomLeft = radii.bottomLeft, bottomRight = radii.bottomRight;
    UIBezierPath *shadowPath = [UIBezierPath wx_bezierPathWithRoundedRect:self.view.bounds topLeft:topLeft topRight:topRight bottomLeft:bottomLeft bottomRight:bottomRight];
    if (boxShadow.isInset) {
        if (boxShadow.innerLayer) {
            boxShadow.innerLayer.frame = self.view.bounds;
            if (![boxShadow.innerLayer superlayer] ){
                self.view.layer.masksToBounds = YES;
                [self.view.layer addSublayer:boxShadow.innerLayer];
            }
        }
    } else {
        self.view.layer.masksToBounds = NO;
        self.view.layer.shadowColor = boxShadow.shadowColor.CGColor;
        self.view.layer.shadowOffset = boxShadow.shadowOffset;
        self.view.layer.shadowRadius = boxShadow.shadowRadius;
        self.view.layer.shadowOpacity = boxShadow.shadowOpacity;
        self.view.layer.shadowPath = shadowPath.CGPath;
    }
}

@end
