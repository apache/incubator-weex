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

#if COMPILER(GCC) && ASSERT_DISABLED
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wreturn-type"
#endif // COMPILER(GCC) && ASSERT_DISABLED

namespace JSC { namespace B3 {

enum Width : int8_t {
    Width8,
    Width16,
    Width32,
    Width64
};

inline Width pointerWidth()
{
    if (sizeof(void*) == 8)
        return Width64;
    return Width32;
}

inline Width widthForType(Type type)
{
    switch (type) {
    case Void:
        ASSERT_NOT_REACHED();
        return Width8;
    case Int32:
    case Float:
        return Width32;
    case Int64:
    case Double:
        return Width64;
    }
    ASSERT_NOT_REACHED();
    return Width8;
}

inline Width canonicalWidth(Width width)
{
    return std::max(Width32, width);
}

inline bool isCanonicalWidth(Width width)
{
    return width >= Width32;
}

Type bestType(Bank bank, Width width);

inline Width conservativeWidth(Bank bank)
{
    return bank == GP ? pointerWidth() : Width64;
}

inline Width minimumWidth(Bank bank)
{
    return bank == GP ? Width8 : Width32;
}

inline unsigned bytes(Width width)
{
    return 1 << width;
}

inline Width widthForBytes(unsigned bytes)
{
    switch (bytes) {
    case 0:
    case 1:
        return Width8;
    case 2:
        return Width16;
    case 3:
    case 4:
        return Width32;
    default:
        return Width64;
    }
}

inline uint64_t mask(Width width)
{
    switch (width) {
    case Width8:
        return 0x00000000000000ffllu;
    case Width16:
        return 0x000000000000ffffllu;
    case Width32:
        return 0x00000000ffffffffllu;
    case Width64:
        return 0xffffffffffffffffllu;
    }
}

} } // namespace JSC::B3

namespace WTF {

class PrintStream;

void printInternal(PrintStream&, JSC::B3::Width);

} // namespace WTF

#if COMPILER(GCC) && ASSERT_DISABLED
#pragma GCC diagnostic pop
#endif // COMPILER(GCC) && ASSERT_DISABLED

#endif // ENABLE(B3_JIT)

