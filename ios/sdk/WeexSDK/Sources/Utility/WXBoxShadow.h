/**
 * Created by Weex.
 * Copyright (c) 2016, Alibaba, Inc. All rights reserved.
 *
 * This source code is licensed under the Apache Licence 2.0.
 * For the full copyright and license information,please view the LICENSE file in the root directory of this source tree.
 */

#import <Foundation/Foundation.h>
#import "WXInnerLayer.h"

@interface WXBoxShadow : NSObject

@property(nullable) CGColorRef shadowColor;
@property CGSize shadowOffset;
@property CGFloat shadowRadius;
@property BOOL isInset;
@property (nonatomic, strong, nullable)WXInnerLayer *innerLayer;
@property CGFloat shadowOpacity;

/**
 *  @abstract get boxshadow from string and adapter phone screen
 *
 *  @param string the boxshadow string
 *
 *  @param scaleFactor the boxshadow set last time
 *
 *  @return A WXBoxShadow object
 */
+(WXBoxShadow *_Nullable)getBoxShadowFromString:(NSString *_Nullable)string scaleFactor:(CGFloat)scaleFactor;
@end
