/**
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
#pragma once

#include <cmath>
#include <wtf/Optional.h>

namespace JSC {

const int32_t maxExponentForIntegerMathPow = 1000;
double JIT_OPERATION operationMathPow(double x, double y) WTF_INTERNAL;
int32_t JIT_OPERATION operationToInt32(double) WTF_INTERNAL;
int32_t JIT_OPERATION operationToInt32SensibleSlow(double) WTF_INTERNAL;

inline constexpr double maxSafeInteger()
{
    // 2 ^ 53 - 1
    return 9007199254740991.0;
}

inline constexpr double minSafeInteger()
{
    // -(2 ^ 53 - 1)
    return -9007199254740991.0;
}

inline int clz32(uint32_t number)
{
#if COMPILER(GCC_OR_CLANG)
    int zeroCount = 32;
    if (number)
        zeroCount = __builtin_clz(number);
    return zeroCount;
#else
    int zeroCount = 0;
    for (int i = 31; i >= 0; i--) {
        if (!(number >> i))
            zeroCount++;
        else
            break;
    }
    return zeroCount;
#endif
}

// This in the ToInt32 operation is defined in section 9.5 of the ECMA-262 spec.
// Note that this operation is identical to ToUInt32 other than to interpretation
// of the resulting bit-pattern (as such this method is also called to implement
// ToUInt32).
//
// The operation can be described as round towards zero, then select the 32 least
// bits of the resulting value in 2s-complement representation.
ALWAYS_INLINE int32_t toInt32(double number)
{
    int64_t bits = WTF::bitwise_cast<int64_t>(number);
    int32_t exp = (static_cast<int32_t>(bits >> 52) & 0x7ff) - 0x3ff;

    // If exponent < 0 there will be no bits to the left of the decimal point
    // after rounding; if the exponent is > 83 then no bits of precision can be
    // left in the low 32-bit range of the result (IEEE-754 doubles have 52 bits
    // of fractional precision).
    // Note this case handles 0, -0, and all infinite, NaN, & denormal value.
    if (exp < 0 || exp > 83)
        return 0;

    // Select the appropriate 32-bits from the floating point mantissa. If the
    // exponent is 52 then the bits we need to select are already aligned to the
    // lowest bits of the 64-bit integer representation of the number, no need
    // to shift. If the exponent is greater than 52 we need to shift the value
    // left by (exp - 52), if the value is less than 52 we need to shift right
    // accordingly.
    int32_t result = (exp > 52)
        ? static_cast<int32_t>(bits << (exp - 52))
        : static_cast<int32_t>(bits >> (52 - exp));

    // IEEE-754 double precision values are stored omitting an implicit 1 before
    // the decimal point; we need to reinsert this now. We may also the shifted
    // invalid bits into the result that are not a part of the mantissa (the sign
    // and exponent bits from the floatingpoint representation); mask these out.
    if (exp < 32) {
        int32_t missingOne = 1 << exp;
        result &= missingOne - 1;
        result += missingOne;
    }

    // If the input value was negative (we could test either 'number' or 'bits',
    // but testing 'bits' is likely faster) invert the result appropriately.
    return bits < 0 ? -result : result;
}

// This implements ToUInt32, defined in ECMA-262 9.6.
inline uint32_t toUInt32(double number)
{
    // As commented in the spec, the operation of ToInt32 and ToUint32 only differ
    // in how the result is interpreted; see NOTEs in sections 9.5 and 9.6.
    return toInt32(number);
}

inline std::optional<double> safeReciprocalForDivByConst(double constant)
{
    // No "weird" numbers (NaN, Denormal, etc).
    if (!constant || !std::isnormal(constant))
        return std::nullopt;

    int exponent;
    if (std::frexp(constant, &exponent) != 0.5)
        return std::nullopt;

    // Note that frexp() returns the value divided by two
    // so we to offset this exponent by one.
    exponent -= 1;

    // A double exponent is between -1022 and 1023.
    // Nothing we can do to invert 1023.
    if (exponent == 1023)
        return std::nullopt;

    double reciprocal = std::ldexp(1, -exponent);
    ASSERT(std::isnormal(reciprocal));
    ASSERT(1. / constant == reciprocal);
    ASSERT(constant == 1. / reciprocal);
    ASSERT(1. == constant * reciprocal);

    return reciprocal;
}

extern "C" {
double JIT_OPERATION jsRound(double value) REFERENCED_FROM_ASM WTF_INTERNAL;

// On Windows we need to wrap fmod; on other platforms we can call it directly.
// On ARMv7 we assert that all function pointers have to low bit set (point to thumb code).
#if CALLING_CONVENTION_IS_STDCALL || CPU(ARM_THUMB2)
double JIT_OPERATION jsMod(double x, double y) REFERENCED_FROM_ASM WTF_INTERNAL;
#else
#define jsMod fmod
#endif
}

} // namespace JSC
