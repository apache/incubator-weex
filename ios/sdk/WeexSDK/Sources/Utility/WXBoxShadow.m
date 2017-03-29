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

- (void) dealloc
{
    CGColorRelease(_shadowColor);
}

- (instancetype)init
{
    self = [super init];
    
    if (self) {
        self.shadowRadius = 0.0f;
        self.isInset = NO;
        self.shadowOffset = CGSizeZero;
        self.shadowOpacity = 1.0f;
    }
    
    return self;
}

- (void)setShadowColor: (CGColorRef)shadowColor
{
    if (shadowColor == _shadowColor)
    return;
    
    CGColorRetain(shadowColor);
    CGColorRelease(_shadowColor);
    _shadowColor = shadowColor;
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
    
    //parse color
    if ([string rangeOfString:@"rgb"].location != NSNotFound) {
        NSRange begin = [string rangeOfString:@"rgb"];
        NSRange end = [string rangeOfString:@")"];
        if (begin.location < end.location && end.location < [string length]) {
            NSRange range = NSMakeRange(begin.location, end.location-begin.location + 1);
            NSString *str = [string substringWithRange:range];
            UIColor *color = [WXConvert UIColor:str];
            if (color && [color isKindOfClass:[UIColor class]]) {
                boxShadow.shadowColor = color.CGColor;
            }
            string = [string stringByReplacingOccurrencesOfString:str withString:@""];// remove color string
        }
    } else {
        NSArray *boxShadowElements = [self getBoxShadowElementsByBlank:string];
        NSString *str = [boxShadowElements lastObject];
        UIColor *color = [WXConvert UIColor:str];
        if (color && [color isKindOfClass:[UIColor class]]) {
            boxShadow.shadowColor = color.CGColor;
        }
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
