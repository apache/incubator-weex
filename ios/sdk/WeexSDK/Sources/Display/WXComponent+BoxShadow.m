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

- (void)resetViewLayer:(UIView *_Nullable)view lastBoxShadow:(WXBoxShadow *_Nullable)lastBoxShadow  boxShadow:(WXBoxShadow *_Nullable)originalBoxShadow
{
    UIBezierPath *shadowPath = [UIBezierPath bezierPathWithRect:view.bounds];
    view.layer.masksToBounds = NO;
    view.layer.shadowColor = originalBoxShadow.shadowColor;
    view.layer.shadowOffset = originalBoxShadow.shadowOffset;
    view.layer.shadowRadius = originalBoxShadow.shadowRadius;
    view.layer.shadowOpacity = originalBoxShadow.shadowOpacity;
    view.layer.shadowPath = shadowPath.CGPath;
    if (lastBoxShadow.innerLayer) {
        if (lastBoxShadow.innerLayer) {
            [lastBoxShadow.innerLayer removeFromSuperlayer];
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

- (void)configViewLayer:(UIView *_Nullable)view boxShadow:(WXBoxShadow *_Nullable)boxShadow
{
    if (!_originalBoxShadow) {
        _originalBoxShadow = [self getViewBoxShadow:view];
    }
    if (!boxShadow && !_lastBoxShadow) {
        return;
    }
    [self resetViewLayer:view lastBoxShadow:_lastBoxShadow boxShadow:_originalBoxShadow];
    if (!boxShadow) {
        return;
    }
    if (boxShadow.isInset) {
        if (boxShadow.innerLayer) {
            [boxShadow.innerLayer removeFromSuperlayer];
            boxShadow.innerLayer.frame = view.bounds;
            [view.layer addSublayer:boxShadow.innerLayer];
        }
    } else {
        UIBezierPath *shadowPath = [UIBezierPath bezierPathWithRect:view.bounds];
        view.layer.masksToBounds = NO;
        view.layer.shadowColor = boxShadow.shadowColor;
        view.layer.shadowOffset = boxShadow.shadowOffset;
        view.layer.shadowRadius = boxShadow.shadowRadius;
        view.layer.shadowOpacity = boxShadow.shadowOpacity;
        view.layer.shadowPath = shadowPath.CGPath;
    }
}

@end
