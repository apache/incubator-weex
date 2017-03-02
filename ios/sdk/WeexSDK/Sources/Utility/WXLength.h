/**
 * Created by Weex.
 * Copyright (c) 2016, Alibaba, Inc. All rights reserved.
 *
 * This source code is licensed under the Apache Licence 2.0.
 * For the full copyright and license information,please view the LICENSE file in the root directory of this source tree.
 */

#import <Foundation/Foundation.h>

typedef enum : NSUInteger {
    WXLengthTypeFixed,
    WXLengthTypePercent,
    WXLengthTypeAuto,
    WXLengthTypeNormal
} WXLengthType;

@interface WXLength : NSObject

+ (instancetype)lengthWithFloat:(float)value type:(WXLengthType)type;

+ (instancetype)lengthWithInt:(int)value type:(WXLengthType)type;

- (float)valueForMaximum:(float)maximumValue;

- (int)intValue;

- (float)floatValue;

- (BOOL)isEqualToLength:(WXLength *)length;

- (BOOL)isFixed;

- (BOOL)isPercent;

- (BOOL)isAuto;

- (BOOL)isNormal;

@end
