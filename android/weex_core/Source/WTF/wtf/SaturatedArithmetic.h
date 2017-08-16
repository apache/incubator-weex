/*
 * Copyright (c) 2012, Google Inc. All rights reserved.
 * Copyright (C) 2014 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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
