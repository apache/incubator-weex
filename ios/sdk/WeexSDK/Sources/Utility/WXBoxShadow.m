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

#import "WXBoxShadow.h"
#import "WXConvert.h"
#import "WXUtility.h"

@implementation WXBoxShadow
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
                boxShadow.shadowColor = color;
            }
            string = [string stringByReplacingOccurrencesOfString:str withString:@""];// remove color string
        }
    } else {
        NSArray *boxShadowElements = [self getBoxShadowElementsByBlank:string];
        NSString *str = [boxShadowElements lastObject];
        UIColor *color = [WXConvert UIColor:str];
        if (color && [color isKindOfClass:[UIColor class]]) {
            boxShadow.shadowColor = color;
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
