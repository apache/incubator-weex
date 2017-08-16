/*
 * Copyright (C) 2015-2016 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 */

#pragma once

#if ENABLE(B3_JIT)

#include "JSExportMacros.h"
#include <wtf/Optional.h>

namespace JSC { namespace B3 {

inline bool is64Bit() { return sizeof(void*) == 8; }
inline bool is32Bit() { return !is64Bit(); }

enum B3ComplitationMode {
    B3Mode,
    AirMode
};

JS_EXPORT_PRIVATE bool shouldDumpIR(B3ComplitationMode);
bool shouldDumpIRAtEachPhase(B3ComplitationMode);
bool shouldValidateIR();
bool shouldValidateIRAtEachPhase();
bool shouldSaveIRBeforePhase();
bool shouldMeasurePhaseTiming();

template<typename BitsType, typename InputType>
inline bool isIdentical(InputType left, InputType right)
{
    BitsType leftBits = bitwise_cast<BitsType>(left);
    BitsType rightBits = bitwise_cast<BitsType>(right);
    return leftBits == rightBits;
}

inline bool isIdentical(int32_t left, int32_t right)
{
    return isIdentical<int32_t>(left, right);
}

inline bool isIdentical(int64_t left, int64_t right)
{
    return isIdentical<int64_t>(left, right);
}

inline bool isIdentical(double left, double right)
{
    return isIdentical<int64_t>(left, right);
}

inline bool isIdentical(float left, float right)
{
    return isIdentical<int32_t>(left, right);
}

template<typename ResultType, typename InputType, typename BitsType>
inline bool isRepresentableAsImpl(InputType originalValue)
{
    // Convert the original value to the desired result type.
    ResultType result = static_cast<ResultType>(originalValue);

    // Convert the converted value back to the original type. The original value is representable
    // using the new type if such round-tripping doesn't lose bits.
    InputType newValue = static_cast<InputType>(result);

    return isIdentical<BitsType>(originalValue, newValue);
}

template<typename ResultType>
inline bool isRepresentableAs(int32_t value)
{
    return isRepresentableAsImpl<ResultType, int32_t, int32_t>(value);
}

template<typename ResultType>
inline bool isRepresentableAs(int64_t value)
{
    return isRepresentableAsImpl<ResultType, int64_t, int64_t>(value);
}

template<typename ResultType>
inline bool isRepresentableAs(double value)
{
    return isRepresentableAsImpl<ResultType, double, int64_t>(value);
}

template<typename IntType>
static IntType chillDiv(IntType numerator, IntType denominator)
{
    if (!denominator)
        return 0;
    if (denominator == -1 && numerator == std::numeric_limits<IntType>::min())
        return std::numeric_limits<IntType>::min();
    return numerator / denominator;
}

template<typename IntType>
static IntType chillMod(IntType numerator, IntType denominator)
{
    if (!denominator)
        return 0;
    if (denominator == -1 && numerator == std::numeric_limits<IntType>::min())
        return 0;
    return numerator % denominator;
}

template<typename IntType>
static IntType chillUDiv(IntType numerator, IntType denominator)
{
    typedef typename std::make_unsigned<IntType>::type UnsignedIntType;
    UnsignedIntType unsignedNumerator = static_cast<UnsignedIntType>(numerator);
    UnsignedIntType unsignedDenominator = static_cast<UnsignedIntType>(denominator);
    if (!unsignedDenominator)
        return 0;
    return unsignedNumerator / unsignedDenominator;
}

template<typename IntType>
static IntType chillUMod(IntType numerator, IntType denominator)
{
    typedef typename std::make_unsigned<IntType>::type UnsignedIntType;
    UnsignedIntType unsignedNumerator = static_cast<UnsignedIntType>(numerator);
    UnsignedIntType unsignedDenominator = static_cast<UnsignedIntType>(denominator);
    if (!unsignedDenominator)
        return 0;
    return unsignedNumerator % unsignedDenominator;
}

template<typename IntType>
static IntType rotateRight(IntType value, int32_t shift)
{
    typedef typename std::make_unsigned<IntType>::type UnsignedIntType;
    UnsignedIntType uValue = static_cast<UnsignedIntType>(value);
    int32_t bits = sizeof(IntType) * 8;
    int32_t mask = bits - 1;
    shift &= mask;
    return (uValue >> shift) | (uValue << ((bits - shift) & mask));
}

template<typename IntType>
static IntType rotateLeft(IntType value, int32_t shift)
{
    typedef typename std::make_unsigned<IntType>::type UnsignedIntType;
    UnsignedIntType uValue = static_cast<UnsignedIntType>(value);
    int32_t bits = sizeof(IntType) * 8;
    int32_t mask = bits - 1;
    shift &= mask;
    return (uValue << shift) | (uValue >> ((bits - shift) & mask));
}

} } // namespace JSC::B3

#endif // ENABLE(B3_JIT)
