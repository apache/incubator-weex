/**
 * Created by Weex.
 * Copyright (c) 2016, Alibaba, Inc. All rights reserved.
 *
 * This source code is licensed under the Apache Licence 2.0.
 * For the full copyright and license information,please view the LICENSE file in the root directory of this source tree.
 */

#import "WXComponent+GradientColor.h"
#import "WXComponent_internal.h"
#import "WXConvert.h"

@implementation  WXComponent (GradientColor)

- (void)setGradientLayer
{
    if (CGRectEqualToRect(self.view.frame, CGRectZero)) {
        return;
    }
    //parse gradient-color, linear-gradient(to right, #a80077,rgba(200, 54, 54, 0.5))
    if ([_backgroundImage hasPrefix:@"linear-gradient"] && [_backgroundImage hasSuffix:@")"] ) {
        NSRange range = NSMakeRange(16, _backgroundImage.length - 17);
        NSString *str = [_backgroundImage substringWithRange:range];
        NSArray *array = [str componentsSeparatedByString:@","];
        WXGradientType gradientType;
        UIColor *startColor, *endColor;
        
        if ([array count] == 3) {
            gradientType = [WXConvert gradientType:array[0]];
            startColor = [WXConvert UIColor:array[1]];
            endColor = [WXConvert UIColor:array[2]];
        }
        else if ([array count] > 3){
            NSString *gradientTypeStr = array[0];
            NSString *subStr = [str substringFromIndex:gradientTypeStr.length + 1];
            
            if ([subStr hasPrefix:@"rgb"]) {
                gradientType = [WXConvert gradientType:gradientTypeStr];
                
                range = [subStr rangeOfString:@")"];
                NSString *startColorStr = [subStr substringToIndex:range.location + 1];
                NSString *endColorStr = [subStr substringFromIndex:range.location + 2];
                startColor = [WXConvert UIColor:startColorStr];
                endColor = [WXConvert UIColor:endColorStr];
            }
            else {
                gradientType = [WXConvert gradientType:gradientTypeStr];
                
                startColor = [WXConvert UIColor:array[1]];
                
                NSString *startColorStr = array[1];
                NSString *endColorStr = [subStr substringFromIndex:startColorStr.length + 1];
                endColor = [WXConvert UIColor:endColorStr];
            }
        }
        else {
            return;
        }
    
        __weak typeof(self) weakSelf = self;
        dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
            CAGradientLayer * gradientLayer = [weakSelf gradientLayerFromColors:@[startColor, endColor] gradientType:gradientType];
            dispatch_async(dispatch_get_main_queue(), ^{
                __strong typeof(self) strongSelf = weakSelf;
                /*
                 must insert the gradientLayer at index 0, and then set masksToBounds to match the view bounds
                 or the subview will be invisible
                 */
                if(strongSelf) {
                    [strongSelf.view.layer insertSublayer:gradientLayer atIndex:0];
                    strongSelf.view.layer.masksToBounds = YES;
                }
            });
        });
    }
}

- (CAGradientLayer*)gradientLayerFromColors:(NSArray*)colors gradientType:(WXGradientType)gradientType
{
    CAGradientLayer * gradientLayer = [CAGradientLayer layer];
    NSMutableArray *newColors = [NSMutableArray array];
    for(UIColor *color in colors) {
        [newColors addObject:(id)color.CGColor];
    }
    gradientLayer.colors = newColors;
    CGPoint start;
    CGPoint end;
    switch (gradientType) {
        case WXGradientTypeToTop:
            start = CGPointMake(0.0, 1.0);
            end = CGPointMake(0.0, 0.0);
            break;
        case WXGradientTypeToBottom:
            start = CGPointMake(0.0, 0.0);
            end = CGPointMake(0.0, 1.0);
            break;
        case WXGradientTypeToLeft:
            start = CGPointMake(1.0, 0.0);
            end = CGPointMake(0.0, 0.0);
            break;
        case WXGradientTypeToRight:
            start = CGPointMake(0.0, 0.0);
            end = CGPointMake(1.0, 0.0);
            break;
        case WXGradientTypeToTopleft:
            start = CGPointMake(1.0, 1.0);
            end = CGPointMake(0.0, 0.0f);
            break;
        case WXGradientTypeToBottomright:
            start = CGPointMake(0.0, 0.0);
            end = CGPointMake(1.0, 1.0);
            break;
        default:
            break;
    }
    
    gradientLayer.startPoint = start;
    gradientLayer.endPoint = end;
    gradientLayer.frame = self.view.bounds;
    
    return gradientLayer;
}

@end
