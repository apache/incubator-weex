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
#ifndef SaturatedArithmetic_h
#define SaturatedArithmetic_h

#include "Compiler.h"
#include <limits>
#include <stdint.h>
#include <stdlib.h>

inline bool signedAddOverflows(int32_t a, int32_t b, int32_t& result)
{
#if COMPILER_HAS_CLANG_BUILTIN(__builtin_sadd_overflow) && !(defined __clang_major__ && __clang_major__ < 7)
    return __builtin_sadd_overflow(a, b, &result);
#else
    uint32_t ua = a;
    uint32_t ub = b;
    uint32_t uresult = ua + ub;
    result = static_cast<int32_t>(uresult);

    // Can only overflow if the signed bit of the two values match. If the signed
    // bit of the result and one of the values differ it did overflow.
    return !((ua ^ ub) >> 31) && (uresult ^ ua) >> 31;
#endif
}

inline int32_t saturatedAddition(int32_t a, int32_t b)
{
    int32_t result;
#if CPU(ARM_THUMB2)
    asm("qadd %[sum], %[addend], %[augend]"
        : [sum]"=r"(result)
        : [augend]"r"(a), [addend]"r"(b)
        : /* Nothing is clobbered. */
        );
#else
    if (signedAddOverflows(a, b, result))
        result = std::numeric_limits<int32_t>::max() + (static_cast<uint32_t>(a) >> 31);
#endif
    return result;
}

inline bool signedSubtractOverflows(int32_t a, int32_t b, int32_t& result)
{
#if COMPILER_HAS_CLANG_BUILTIN(__builtin_ssub_overflow) && !(defined __clang_major__ && __clang_major__ < 7)
    return __builtin_ssub_overflow(a, b, &result);
#else
    uint32_t ua = a;
    uint32_t ub = b;
    uint32_t uresult = ua - ub;
    result = static_cast<int32_t>(uresult);

    // Can only overflow if the signed bit of the two values do not match. If the
    // signed bit of the result and the first value differ it did overflow.
    return (ua ^ ub) >> 31 && (uresult ^ ua) >> 31;
#endif
}

inline int32_t saturatedSubtraction(int32_t a, int32_t b)
{
    int32_t result;
#if CPU(ARM_THUMB2)
    asm("qsub %[difference], %[minuend], %[subtrahend]"
        : [difference]"=r"(result)
        : [minuend]"r"(a), [subtrahend]"r"(b)
        : /* Nothing is clobbered. */
        );
#else
    if (signedSubtractOverflows(a, b, result))
        result = std::numeric_limits<int32_t>::max() + (static_cast<uint32_t>(a) >> 31);
#endif
    return result;
}

#endif // SaturatedArithmetic_h
