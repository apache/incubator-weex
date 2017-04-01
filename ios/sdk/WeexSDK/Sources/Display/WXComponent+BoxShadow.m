/**
 * Created by Weex.
 * Copyright (c) 2016, Alibaba, Inc. All rights reserved.
 *
 * This source code is licensed under the Apache Licence 2.0.
 * For the full copyright and license information,please view the LICENSE file in the root directory of this source tree.
 */

#import "WXComponent+BoxShadow.h"
#import "WXBoxShadow.h"
#import "WXConvert.h"
#import "WXUtility.h"
#import "WXComponent_internal.h"

@implementation WXComponent (BoxShadow)


- (WXBoxShadow *_Nullable)getViewBoxShadow:(UIView *_Nullable)view
{
    WXBoxShadow *boxShadow = [WXBoxShadow new];
    boxShadow.shadowColor = view.layer.shadowColor;
    boxShadow.shadowOffset = view.layer.shadowOffset;
    boxShadow.shadowRadius = view.layer.shadowRadius;
    boxShadow.shadowOpacity =  view.layer.shadowOpacity;
    return boxShadow;
}

- (void)resetViewLayer
{
    UIBezierPath *shadowPath = [UIBezierPath bezierPathWithRect:self.view.bounds];
    self.view.layer.masksToBounds = NO;
    self.view.layer.shadowColor = _originalBoxShadow.shadowColor;
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
- (BOOL)EqualBoxShadow:(WXBoxShadow *_Nullable)boxShadow withBoxShadow:(WXBoxShadow *_Nullable)compareBoxShadow
{
    if(!compareBoxShadow && !boxShadow) {
        return YES;
    } else if (CGColorEqualToColor(boxShadow.shadowColor,compareBoxShadow.shadowColor) &&
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
    if (boxShadow.isInset) {
        if (boxShadow.innerLayer) {
            boxShadow.innerLayer.frame = self.view.bounds;
            if (![boxShadow.innerLayer superlayer] ){
                [self.view.layer addSublayer:boxShadow.innerLayer];
            }
        }
    } else {
        UIBezierPath *shadowPath = [UIBezierPath bezierPathWithRect:self.view.bounds];
        self.view.layer.masksToBounds = NO;
        self.view.layer.shadowColor = boxShadow.shadowColor;
        self.view.layer.shadowOffset = boxShadow.shadowOffset;
        self.view.layer.shadowRadius = boxShadow.shadowRadius;
        self.view.layer.shadowOpacity = boxShadow.shadowOpacity;
        self.view.layer.shadowPath = shadowPath.CGPath;
    }
}

@end
