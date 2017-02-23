/**
 * Created by Weex.
 * Copyright (c) 2016, Alibaba, Inc. All rights reserved.
 *
 * This source code is licensed under the Apache Licence 2.0.
 * For the full copyright and license information,please view the LICENSE file in the root directory of this source tree.
 */

#import "WXInnerLayer.h"

@implementation WXInnerLayer

@dynamic boxShadowColor, boxShadowOffset, boxShadowRadius, boxShadowOpacity;

+ (BOOL)needsDisplayForKey:(NSString *)key {
    if ([key isEqualToString:@"boxShadowRadius"]  || [key isEqualToString:@"boxShadowOffset"] || [key isEqualToString:@"boxShadowColor"] || [key isEqualToString:@"boxShadowOpacity"]) {
        return YES;
    }
    return [super needsDisplayForKey:key];
}

- (id)actionForKey:(NSString *) key {
    if ([key isEqualToString:@"boxShadowRadius"] || [key isEqualToString:@"boxShadowOffset"] ||[key isEqualToString:@"boxShadowColor"] ||  [key isEqualToString:@"boxShadowOpacity"] ) {
        CABasicAnimation *theAnimation = [CABasicAnimation animationWithKeyPath:key];
        theAnimation.fromValue = [self.presentationLayer valueForKey:key];
        return theAnimation;
    }
    return [super actionForKey:key];
}

- (void)drawInContext:(CGContextRef)context {
    CGFloat radius = self.cornerRadius;
    
    CGRect rect = self.bounds;
    if (self.borderWidth != 0) {
        rect = CGRectInset(rect, self.borderWidth, self.borderWidth);
        radius -= self.borderWidth;
        radius = MAX(radius, 0);
    }
    
    CGContextSetAllowsAntialiasing(context, YES);
    CGContextSetShouldAntialias(context, YES);
    CGContextSetInterpolationQuality(context, kCGInterpolationHigh);
    CGColorSpaceRef colorspace = CGColorSpaceCreateDeviceRGB();
    
    UIBezierPath *bezierPath = [UIBezierPath bezierPathWithRoundedRect:rect cornerRadius:radius];
    CGContextAddPath(context, bezierPath.CGPath);
    CGContextClip(context);
    CGMutablePathRef outer = CGPathCreateMutable();
    CGPathAddRect(outer, NULL, CGRectInset(rect, -1*rect.size.width, -1*rect.size.height));
    CGPathAddPath(outer, NULL, bezierPath.CGPath);
    CGPathCloseSubpath(outer);
    CGFloat *oldComponents = (CGFloat *)CGColorGetComponents(self.boxShadowColor);
    CGFloat newComponents[4];
    NSInteger numberOfComponents = CGColorGetNumberOfComponents(self.boxShadowColor);
    switch (numberOfComponents)
    {
        case 2:
        {
            //grayscale
            newComponents[0] = oldComponents[0];
            newComponents[1] = oldComponents[0];
            newComponents[2] = oldComponents[0];
            newComponents[3] = oldComponents[1] * self.boxShadowOpacity;
            break;
        }
        case 4:
        {
            //RGBA
            newComponents[0] = oldComponents[0];
            newComponents[1] = oldComponents[1];
            newComponents[2] = oldComponents[2];
            newComponents[3] = oldComponents[3] * self.boxShadowOpacity;
            break;
        }
    }
    CGColorRef shadowColorWithMultipliedAlpha = CGColorCreate(colorspace, newComponents);
    CGColorSpaceRelease(colorspace);
    CGContextSetFillColorWithColor(context, shadowColorWithMultipliedAlpha);
    CGContextSetShadowWithColor(context, self.boxShadowOffset, self.boxShadowRadius, shadowColorWithMultipliedAlpha);
    CGContextAddPath(context, outer);
    CGContextEOFillPath(context);
    CGPathRelease(outer);
    CGColorRelease(shadowColorWithMultipliedAlpha);
}

@end
