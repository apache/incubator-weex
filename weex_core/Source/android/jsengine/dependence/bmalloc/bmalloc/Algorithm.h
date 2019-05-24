/*
 * Copyright (C) 2014 Apple Inc. All rights reserved.
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
#ifndef Algorithm_h
#define Algorithm_h

#include "Algorithm.h"
#include "BAssert.h"
#include <algorithm>
#include <cstdint>
#include <cstddef>
#include <limits>
#include <type_traits>
#include <chrono>

namespace bmalloc {

// Versions of min and max that are compatible with compile-time constants.
template<typename T> inline constexpr T max(T a, T b)
{
    return a > b ? a : b;
}
    
template<typename T> inline constexpr T min(T a, T b)
{
    return a < b ? a : b;
}

template<typename T> inline constexpr T mask(T value, uintptr_t mask)
{
    return reinterpret_cast<T>(reinterpret_cast<uintptr_t>(value) & mask);
}

template<typename T> inline constexpr bool test(T value, uintptr_t mask)
{
    return !!(reinterpret_cast<uintptr_t>(value) & mask);
}

inline constexpr bool isPowerOfTwo(size_t size)
{
    return size && !(size & (size - 1));
}

template<typename T> inline T roundUpToMultipleOf(size_t divisor, T x)
{
    BASSERT(isPowerOfTwo(divisor));
    return reinterpret_cast<T>((reinterpret_cast<uintptr_t>(x) + (divisor - 1)) & ~(divisor - 1));
}

template<size_t divisor, typename T> inline constexpr T roundUpToMultipleOf(T x)
{
    static_assert(isPowerOfTwo(divisor), "'divisor' must be a power of two.");
    return roundUpToMultipleOf(divisor, x);
}

template<typename T> inline T roundDownToMultipleOf(size_t divisor, T x)
{
    BASSERT(isPowerOfTwo(divisor));
    return reinterpret_cast<T>(mask(reinterpret_cast<uintptr_t>(x), ~(divisor - 1ul)));
}

template<size_t divisor, typename T> inline constexpr T roundDownToMultipleOf(T x)
{
    static_assert(isPowerOfTwo(divisor), "'divisor' must be a power of two.");
    return roundDownToMultipleOf(divisor, x);
}

template<typename T> inline void divideRoundingUp(T numerator, T denominator, T& quotient, T& remainder)
{
    // We expect the compiler to emit a single divide instruction to extract both the quotient and the remainder.
    quotient = numerator / denominator;
    remainder = numerator % denominator;
    if (remainder)
        quotient += 1;
}

template<typename T> inline T divideRoundingUp(T numerator, T denominator)
{
    return (numerator + denominator - 1) / denominator;
}

template<typename T> inline T roundUpToMultipleOfNonPowerOfTwo(size_t divisor, T x)
{
    return divideRoundingUp(x, divisor) * divisor;
}

// Version of sizeof that returns 0 for empty classes.

template<typename T> inline constexpr size_t sizeOf()
{
    return std::is_empty<T>::value ? 0 : sizeof(T);
}

template<typename T> inline constexpr size_t bitCount()
{
    return sizeof(T) * 8;
}

inline constexpr unsigned long log2(unsigned long value)
{
    return bitCount<unsigned long>() - 1 - __builtin_clzl(value);
}

} // namespace bmalloc

#endif // Algorithm_h
