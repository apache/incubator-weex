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

#import <UIKit/UIKit.h>
#import <objc/runtime.h>
#import "WXLog.h"
#import "WXLayoutDefine.h"
#import "WXType.h"

@class WXLength;
@class WXBoxShadow;
@interface WXConvert : NSObject

+ (BOOL)BOOL:(id)value;
+ (CGFloat)CGFloat:(id)value;
+ (NSUInteger)NSUInteger:(id)value;
+ (NSInteger)NSInteger:(id)value;
+ (NSString *)NSString:(id)value;

/**
 *  750px Adaptive
 */
typedef CGFloat WXPixelType;
// @parameter scaleFactor: please use weexInstance's pixelScaleFactor property
+ (WXPixelType)WXPixelType:(id)value scaleFactor:(CGFloat)scaleFactor;

+ (css_flex_direction_t)css_flex_direction_t:(id)value;
+ (css_align_t)css_align_t:(id)value;
+ (css_wrap_type_t)css_wrap_type_t:(id)value;
+ (css_justify_t)css_justify_t:(id)value;
+ (css_position_type_t)css_position_type_t:(id)value;

+ (UIViewContentMode)UIViewContentMode:(id)value;
+ (WXImageQuality)WXImageQuality:(id)value;
+ (WXImageSharp)WXImageSharp:(id)value;
+ (UIAccessibilityTraits)WXUIAccessibilityTraits:(id)value;

+ (UIColor *)UIColor:(id)value;
+ (CGColorRef)CGColor:(id)value;
+ (NSString *)HexWithColor:(UIColor *)color;
+ (WXBorderStyle)WXBorderStyle:(id)value;
typedef BOOL WXClipType;
+ (WXClipType)WXClipType:(id)value;
+ (WXPositionType)WXPositionType:(id)value;

+ (WXTextStyle)WXTextStyle:(id)value;
/**
 * @abstract UIFontWeightRegular ,UIFontWeightBold,etc are not support by the system which is less than 8.2. weex sdk set the float value.
 *
 * @param value support normal,blod,100,200,300,400,500,600,700,800,900
 *
 * @return A float value.
 *
 */
+ (CGFloat)WXTextWeight:(id)value;
+ (WXTextDecoration)WXTextDecoration:(id)value;
+ (NSTextAlignment)NSTextAlignment:(id)value;
+ (UIReturnKeyType)UIReturnKeyType:(id)value;

+ (WXScrollDirection)WXScrollDirection:(id)value;
+ (UITableViewRowAnimation)UITableViewRowAnimation:(id)value;

+ (UIViewAnimationOptions)UIViewAnimationTimingFunction:(id)value;
+ (CAMediaTimingFunction *)CAMediaTimingFunction:(id)value;

+ (WXVisibility)WXVisibility:(id)value;

+ (WXGradientType)gradientType:(id)value;

+ (WXLength *)WXLength:(id)value isFloat:(BOOL)isFloat scaleFactor:(CGFloat)scaleFactor;
+ (WXBoxShadow *)WXBoxShadow:(id)value scaleFactor:(CGFloat)scaleFactor;

@end

@interface WXConvert (Deprecated)

+ (WXPixelType)WXPixelType:(id)value DEPRECATED_MSG_ATTRIBUTE("Use [WXConvert WXPixelType:scaleFactor:] instead");

@end
