/**
 * Created by Weex.
 * Copyright (c) 2016, Alibaba, Inc. All rights reserved.
 *
 * This source code is licensed under the Apache Licence 2.0.
 * For the full copyright and license information,please view the LICENSE file in the root directory of this source tree.
 */

#import <Foundation/Foundation.h>

@interface WXBoxShadow : NSObject

@property(nullable) CGColorRef shadowColor;
@property CGSize shadowOffset;
@property CGFloat shadowRadius;
@property BOOL isInset;

+(void)configLayer:(UIView *)view boxShadow:(WXBoxShadow *)boxShadow;
+(WXBoxShadow *)getBoxShadowFromString:(NSString *)string;
@end
