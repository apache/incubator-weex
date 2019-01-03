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
