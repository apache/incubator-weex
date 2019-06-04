/*
 * Copyright (C) 2016 Apple Inc. All rights reserved.
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

#include <wtf/StdLibExtras.h>

namespace JSC {

#if USE(JSVALUE64)
// According to C++ rules, a type used for the return signature of function with C linkage (i.e.
// 'extern "C"') needs to be POD; hence putting any constructors into it could cause either compiler
// warnings, or worse, a change in the ABI used to return these types.
struct SlowPathReturnType {
    void* a;
    void* b;
};

inline SlowPathReturnType encodeResult(void* a, void* b)
{
    SlowPathReturnType result;
    result.a = a;
    result.b = b;
    return result;
}

inline void decodeResult(SlowPathReturnType result, void*& a, void*& b)
{
    a = result.a;
    b = result.b;
}

#else // USE(JSVALUE32_64)
typedef int64_t SlowPathReturnType;

typedef union {
    struct {
        void* a;
        void* b;
    } pair;
    int64_t i;
} SlowPathReturnTypeEncoding;

inline SlowPathReturnType encodeResult(void* a, void* b)
{
    SlowPathReturnTypeEncoding u;
    u.pair.a = a;
    u.pair.b = b;
    return u.i;
}

inline void decodeResult(SlowPathReturnType result, void*& a, void*& b)
{
    SlowPathReturnTypeEncoding u;
    u.i = result;
    a = u.pair.a;
    b = u.pair.b;
}
#endif // USE(JSVALUE32_64)

} // namespace JSC
