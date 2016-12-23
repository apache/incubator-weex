//
//  WXComponent+GradientColor.m
//  Pods
//
//  Created by bobning on 16/12/23.
//
//

#import "WXComponent+GradientColor.h"
#import "WXComponent_internal.h"

@implementation  WXComponent (GradientColor)

- (void)setGradientLayer {
    if ([_backgroundImage hasPrefix:@"linear-gradient"] && [_backgroundImage hasSuffix:@")"] ) {
        NSRange range = NSMakeRange(16, _backgroundImage.length-17);
        NSString *str = [_backgroundImage substringWithRange:range];
        NSArray *array = [str componentsSeparatedByString:@","];
        
        if ([array count] != 3) {
            return;
        }
        
        WXGradientType gradientType = [self gradientType:array[0]];
        UIColor *startColor = [WXConvert UIColor:array[1]];
        UIColor *endColor = [WXConvert UIColor:array[2]];
        
        UIImage *bgImg = [self gradientColorImageFromColors:@[startColor, endColor] gradientType:gradientType imgSize:_view.frame.size];
        _view.backgroundColor = [UIColor colorWithPatternImage:bgImg];
    }
}

- (WXGradientType)gradientType:(NSString *)value
{
    WXGradientType type = WXGradientTypeToRight;
    
    if ([value isEqualToString:@"totop"]) {
        type = WXGradientTypeToTop;
    }
    else if ([value isEqualToString:@"tobottom"]) {
        type = WXGradientTypeToBottom;
    }
    else if ([value isEqualToString:@"toleft"]) {
        type = WXGradientTypeToLeft;
    }
    if ([value isEqualToString:@"toright"]) {
        type = WXGradientTypeToRight;
    }
    else if ([value isEqualToString:@"totopleft"]) {
        type = WXGradientTypeToTopleft;
    }
    else if ([value isEqualToString:@"tobottomright"]) {
        type = WXGradientTypeToBottomright;
    }
    
    return type;
}

- (UIImage *)gradientColorImageFromColors:(NSArray*)colors gradientType:(WXGradientType)gradientType imgSize:(CGSize)imgSize;{
    NSMutableArray *array = [NSMutableArray array];
    for(UIColor *color in colors) {
        [array addObject:(id)color.CGColor];
    }
    
    UIGraphicsBeginImageContextWithOptions(imgSize, YES, 1);
    CGContextRef context = UIGraphicsGetCurrentContext();
    CGContextSaveGState(context);
    CGColorSpaceRef colorSpace = CGColorGetColorSpace([[colors lastObject] CGColor]);
    CGGradientRef gradient = CGGradientCreateWithColors(colorSpace, (CFArrayRef)array, NULL);
    CGPoint start;
    CGPoint end;
    switch (gradientType) {
        case WXGradientTypeToTop:
            start = CGPointMake(0.0, imgSize.height);
            end = CGPointMake(0.0, 0.0);
            break;
        case WXGradientTypeToBottom:
            start = CGPointMake(0.0, 0.0);
            end = CGPointMake(0.0, imgSize.height);
            break;
        case WXGradientTypeToLeft:
            start = CGPointMake(imgSize.width, 0.0);
            end = CGPointMake(0.0, 0.0);
            break;
        case WXGradientTypeToRight:
            start = CGPointMake(0.0, 0.0);
            end = CGPointMake(imgSize.width, 0.0);
            break;
        case WXGradientTypeToTopleft:
            start = CGPointMake(imgSize.width, imgSize.height);
            end = CGPointMake(0.0, 0.0f);
            break;
        case WXGradientTypeToBottomright:
            start = CGPointMake(0.0, 0.0);
            end = CGPointMake(imgSize.width, imgSize.height);
            break;
        default:
            break;
    }
    CGContextDrawLinearGradient(context, gradient, start, end, kCGGradientDrawsBeforeStartLocation | kCGGradientDrawsAfterEndLocation);
    UIImage *image = UIGraphicsGetImageFromCurrentImageContext();
    CGGradientRelease(gradient);
    CGContextRestoreGState(context);
    CGColorSpaceRelease(colorSpace);
    UIGraphicsEndImageContext();
    return image;
}

@end
