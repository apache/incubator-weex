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

#include "B3Common.h"
#include <wtf/StdLibExtras.h>

#if COMPILER(GCC) && ASSERT_DISABLED
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wreturn-type"
#endif // COMPILER(GCC) && ASSERT_DISABLED

namespace JSC { namespace B3 {

enum Type : int8_t {
    Void,
    Int32,
    Int64,
    Float,
    Double,
};

inline bool isInt(Type type)
{
    return type == Int32 || type == Int64;
}

inline bool isFloat(Type type)
{
    return type == Float || type == Double;
}

inline Type pointerType()
{
    if (is32Bit())
        return Int32;
    return Int64;
}

inline size_t sizeofType(Type type)
{
    switch (type) {
    case Void:
        return 0;
    case Int32:
    case Float:
        return 4;
    case Int64:
    case Double:
        return 8;
    }
    ASSERT_NOT_REACHED();
}

} } // namespace JSC::B3

namespace WTF {

class PrintStream;

void printInternal(PrintStream&, JSC::B3::Type);

} // namespace WTF

#if COMPILER(GCC) && ASSERT_DISABLED
#pragma GCC diagnostic pop
#endif // COMPILER(GCC) && ASSERT_DISABLED

#endif // ENABLE(B3_JIT)
