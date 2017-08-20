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
