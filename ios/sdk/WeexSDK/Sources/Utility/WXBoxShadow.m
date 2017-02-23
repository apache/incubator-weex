/**
 * Created by Weex.
 * Copyright (c) 2016, Alibaba, Inc. All rights reserved.
 *
 * This source code is licensed under the Apache Licence 2.0.
 * For the full copyright and license information,please view the LICENSE file in the root directory of this source tree.
 */

#import "WXBoxShadow.h"
#import "WXConvert.h"
#import "WXUtility.h"

@implementation WXBoxShadow

+(WXBoxShadow *_Nullable)getViewBoxShadow:(UIView *_Nullable)view
{
    WXBoxShadow *boxShadow = [WXBoxShadow new];
    boxShadow.shadowColor = view.layer.shadowColor;
    boxShadow.shadowOffset = view.layer.shadowOffset;
    boxShadow.shadowRadius = view.layer.shadowRadius;
    boxShadow.shadowOpacity =  view.layer.shadowOpacity;
    return boxShadow;
}

+(void)resetViewLayer:(UIView *_Nullable)view lastBoxShadow:(WXBoxShadow *_Nullable)lastBoxShadow  boxShadow:(WXBoxShadow *_Nullable)originalBoxShadow
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
+(BOOL)EqualBoxShadow:(WXBoxShadow *_Nullable)boxShadow withBoxShadow:(WXBoxShadow *_Nullable)compareBoxShadow
{
    if(!compareBoxShadow && !boxShadow) {
        return YES;
    } else if (CGColorEqualToColor(boxShadow.shadowColor,compareBoxShadow.shadowColor) &&
        CGSizeEqualToSize(boxShadow.shadowOffset,compareBoxShadow.shadowOffset) &&
        WXFloatEqual(boxShadow.shadowRadius,compareBoxShadow.shadowRadius)&& (boxShadow.isInset == boxShadow.isInset))
    {
        return YES;
    }
    return NO;
    
}

+(void)configViewLayer:(UIView *_Nullable)view boxShadow:(WXBoxShadow *_Nullable)boxShadow lastBoxShadow:(WXBoxShadow *_Nullable)lastBoxShadow originalBoxShadow:(WXBoxShadow *_Nullable)originalBoxShadow
{
    if (!boxShadow && !lastBoxShadow) {
        return;
    }
    [self resetViewLayer:view lastBoxShadow:lastBoxShadow boxShadow:originalBoxShadow];
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

+ (NSArray *)getBoxShadowElementsByBlank:(NSString *)string
{
    string = [string stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceCharacterSet]];
    NSError *error = nil;
    NSRegularExpression *regex = [NSRegularExpression regularExpressionWithPattern:@"\\s{2,}" options:NSRegularExpressionCaseInsensitive error:&error];
    
    NSArray *arr = [regex matchesInString:string options:NSMatchingReportCompletion range:NSMakeRange(0, [string length])];
    
    arr = [[arr reverseObjectEnumerator] allObjects];
    for (NSTextCheckingResult *str in arr) {
        string = [string stringByReplacingCharactersInRange:[str range] withString:@" "];
    }
    NSArray *array = [string componentsSeparatedByString:@" "];
    return array;
}

+(WXBoxShadow *_Nullable)getBoxShadowFromString:(NSString *_Nullable)string scaleFactor:(CGFloat)scaleFactor
{
    if ([string length] == 0) {
        return nil;
    }
    WXBoxShadow *boxShadow = [WXBoxShadow new];
    boxShadow.shadowOpacity = 1.0f;
    
    //parse color
    if ([string rangeOfString:@"rgb"].location != NSNotFound) {
        NSRange begin = [string rangeOfString:@"rgb"];
        NSRange end = [string rangeOfString:@")"];
        if (begin.location < end.location && end.location < [string length]) {
            NSRange range = NSMakeRange(begin.location, end.location-begin.location + 1);
            NSString *str = [string substringWithRange:range];
            boxShadow.shadowColor = [WXConvert UIColor:str].CGColor;
            string = [string stringByReplacingOccurrencesOfString:str withString:@""];// remove color string
        }
    } else {
        NSArray *boxShadowElements = [self getBoxShadowElementsByBlank:string];
        NSString *str = [boxShadowElements lastObject];
        boxShadow.shadowColor = [WXConvert UIColor:str].CGColor;
        string = [string stringByReplacingOccurrencesOfString:str withString:@""];// remove color string
    }

    // parse remain BoxShadow Elements
    NSMutableArray *remainBoxShadowElements = [[self getBoxShadowElementsByBlank:string] mutableCopy];
    
    if (remainBoxShadowElements && [remainBoxShadowElements count] > 0) {
        if ([@"inset" isEqualToString: remainBoxShadowElements[0]])
        {
            boxShadow.isInset = YES;
            [remainBoxShadowElements removeObjectAtIndex:0];
            
        }
        
        for (int i = 0; i < [remainBoxShadowElements count]; i++) {
            switch (i) {
                case 0:
                {
                    CGSize size = boxShadow.shadowOffset;
                    size.width = [WXConvert WXPixelType:remainBoxShadowElements[0] scaleFactor:scaleFactor];
                    boxShadow.shadowOffset = size;
                }
                    break;
                case 1:
                {
                    CGSize size = boxShadow.shadowOffset;
                    size.height = [WXConvert WXPixelType:remainBoxShadowElements[1] scaleFactor:scaleFactor];
                    boxShadow.shadowOffset = size;
                }
                    break;
                case 2:
                {
                    boxShadow.shadowRadius = [WXConvert WXPixelType:remainBoxShadowElements[2] scaleFactor:scaleFactor];
                }
                    break;
                    
                default:
                    break;
            }
        }
        
        if (boxShadow.isInset) {
            if (!boxShadow.innerLayer) {
                boxShadow.innerLayer = [[WXInnerLayer alloc] init];
            }
            boxShadow.innerLayer.boxShadowColor = boxShadow.shadowColor;
            boxShadow.innerLayer.boxShadowOffset = boxShadow.shadowOffset;
            boxShadow.innerLayer.boxShadowRadius = boxShadow.shadowRadius;
            boxShadow.innerLayer.boxShadowOpacity = boxShadow.shadowOpacity;
        }
    }
    return boxShadow;
}


@end
