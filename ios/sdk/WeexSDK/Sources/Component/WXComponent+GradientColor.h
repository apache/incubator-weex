/**
 * Created by Weex.
 * Copyright (c) 2016, Alibaba, Inc. All rights reserved.
 *
 * This source code is licensed under the Apache Licence 2.0.
 * For the full copyright and license information,please view the LICENSE file in the root directory of this source tree.
 */


#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#import "WXComponent.h"
#import "WXType.h"

@interface WXComponent (GradientColor)

- (void)setGradientLayer;

- (UIImage *)gradientColorImageFromColors:(NSArray*)colors gradientType:(WXGradientType)gradientType imgSize:(CGSize)imgSize;

@end
