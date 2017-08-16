/*
 * Copyright (C) 2017 Apple Inc. All rights reserved.
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

#include "B3Bank.h"
#include "B3Type.h"
#include "B3Width.h"

namespace JSC { namespace B3 {

template<typename> struct NativeTraits;

template<> struct NativeTraits<int8_t> {
    typedef int32_t CanonicalType;
    static const Bank bank = GP;
    static const Width width = Width8;
    static const Type type = Int32;
};

template<> struct NativeTraits<uint8_t> {
    typedef int32_t CanonicalType;
    static const Bank bank = GP;
    static const Width width = Width8;
    static const Type type = Int32;
};

template<> struct NativeTraits<int16_t> {
    typedef int32_t CanonicalType;
    static const Bank bank = GP;
    static const Width width = Width16;
    static const Type type = Int32;
};

template<> struct NativeTraits<uint16_t> {
    typedef int32_t CanonicalType;
    static const Bank bank = GP;
    static const Width width = Width16;
    static const Type type = Int32;
};

template<> struct NativeTraits<int32_t> {
    typedef int32_t CanonicalType;
    static const Bank bank = GP;
    static const Width width = Width32;
    static const Type type = Int32;
};

template<> struct NativeTraits<uint32_t> {
    typedef int32_t CanonicalType;
    static const Bank bank = GP;
    static const Width width = Width32;
    static const Type type = Int32;
};

template<> struct NativeTraits<int64_t> {
    typedef int64_t CanonicalType;
    static const Bank bank = GP;
    static const Width width = Width64;
    static const Type type = Int64;
};

template<> struct NativeTraits<uint64_t> {
    typedef int64_t CanonicalType;
    static const Bank bank = GP;
    static const Width width = Width64;
    static const Type type = Int64;
};

template<> struct NativeTraits<float> {
    typedef float CanonicalType;
    static const Bank bank = FP;
    static const Width width = Width32;
    static const Type type = Float;
};

template<> struct NativeTraits<double> {
    typedef double CanonicalType;
    static const Bank bank = FP;
    static const Width width = Width64;
    static const Type type = Double;
};

} } // namespace JSC::B3

#endif // ENABLE(B3_JIT)

