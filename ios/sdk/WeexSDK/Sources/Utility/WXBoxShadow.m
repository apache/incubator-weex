/**
 * Created by Weex.
 * Copyright (c) 2016, Alibaba, Inc. All rights reserved.
 *
 * This source code is licensed under the Apache Licence 2.0.
 * For the full copyright and license information,please view the LICENSE file in the root directory of this source tree.
 */

#import "WXBoxShadow.h"
#import "WXConvert.h"

@implementation WXBoxShadow

+(void)configLayer:(UIView *)view boxShadow:(WXBoxShadow *)boxShadow
{
    if(!boxShadow){
        return;
    }
    UIBezierPath *shadowPath = [UIBezierPath bezierPathWithRect:view.bounds];
    view.layer.masksToBounds = NO;
    view.layer.shadowColor = boxShadow.shadowColor;
    view.layer.shadowOffset = boxShadow.shadowOffset;
    view.layer.shadowRadius = boxShadow.shadowRadius;
    view.layer.shadowOpacity = 1.0f;
    view.layer.shadowPath = shadowPath.CGPath;
}

+(WXBoxShadow *)getBoxShadowFromString:(NSString *)string
{
    NSError *error = nil;
    string = [string stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceCharacterSet]];
    NSRegularExpression *regex = [NSRegularExpression regularExpressionWithPattern:@"\\s{2,}" options:NSRegularExpressionCaseInsensitive error:&error];
    
    NSArray *arr = [regex matchesInString:string options:NSMatchingReportCompletion range:NSMakeRange(0, [string length])];
    
    arr = [[arr reverseObjectEnumerator] allObjects];
    
    for (NSTextCheckingResult *str in arr) {
        string = [string stringByReplacingCharactersInRange:[str range] withString:@" "];
    }
    NSMutableArray *array = [[string componentsSeparatedByString:@" "] mutableCopy];
    
    if (array && [array count] > 0) {
        WXBoxShadow *boxShadow = [WXBoxShadow new];
        if ([@"inset" isEqualToString: array[0]])
        {
            boxShadow.isInset = YES;
            [array removeObjectAtIndex:0];
            
        }
        
        for (int i = 0; i < [array count]-1; i++) {
            switch (i) {
                case 0:
                {
                    CGSize size = boxShadow.shadowOffset;
                    size.width = [WXConvert CGFloat:array[0]];
                    boxShadow.shadowOffset = size;
                }
                    break;
                case 1:
                {
                    CGSize size = boxShadow.shadowOffset;
                    size.height = [WXConvert CGFloat:array[1]];
                    boxShadow.shadowOffset = size;
                }
                    break;
                case 2:
                {
                    boxShadow.shadowRadius = [WXConvert CGFloat:array[2]];
                }
                    break;
                    
                default:
                    break;
            }
        }
        boxShadow.shadowColor = [WXConvert UIColor:[array lastObject]].CGColor;
        return boxShadow;
    }
    return nil;
}


@end
