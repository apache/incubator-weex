/**
 * Created by Weex.
 * Copyright (c) 2016, Alibaba, Inc. All rights reserved.
 *
 * This source code is licensed under the Apache Licence 2.0.
 * For the full copyright and license information,please view the LICENSE file in the root directory of this source tree.
 */

#import "WXLength.h"
#import "WXAssert.h"

@implementation WXLength
{
    float _floatValue;
    int _intValue;
    WXLengthType _type;
    BOOL _isFloat;
}

+ (instancetype)lengthWithFloat:(float)value type:(WXLengthType)type
{
    WXLength *length = [WXLength new];
    length->_floatValue = value;
    length->_type = type;
    length->_isFloat = YES;
    return length;
}

+ (instancetype)lengthWithInt:(int)value type:(WXLengthType)type
{
    WXLength *length = [WXLength new];
    length->_intValue = value;
    length->_type = type;
    length->_isFloat = NO;
    return length;
}

- (float)valueForMaximum:(float)maximumValue
{
    
    switch (_type) {
        case WXLengthTypeFixed:
            return _isFloat ? _floatValue : _intValue;
        case WXLengthTypePercent:
            return maximumValue * (_isFloat ? _floatValue : _intValue) / 100.0;
        case WXLengthTypeAuto:
            return maximumValue;
        default:
            WXAssertNotReached();
            return 0;
    }
}

- (int)intValue
{
    WXAssert(!_isFloat, @"call `intValue` for non-int length");
    return _intValue;
}

- (float)floatValue
{
    WXAssert(_isFloat,  @"call `floatValue` for non-float length");
    return _floatValue;
}

- (BOOL)isEqualToLength:(WXLength *)length
{
    return length && _type == length->_type && _isFloat == length->_isFloat
    && _floatValue == length->_floatValue && _intValue == length->_intValue;
}

- (BOOL)isFixed
{
    return _type == WXLengthTypeFixed;
}

- (BOOL)isPercent
{
    return _type == WXLengthTypePercent;
}

- (BOOL)isAuto
{
    return _type == WXLengthTypeAuto;
}

- (BOOL)isNormal
{
    return _type == WXLengthTypeNormal;
}

@end
