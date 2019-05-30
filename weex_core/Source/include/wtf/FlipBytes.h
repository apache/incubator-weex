/*
 * Copyright (C) 2013 Apple Inc. All rights reserved.
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

#ifndef FlipBytes_h
#define FlipBytes_h

namespace WTF {

inline bool needToFlipBytesIfLittleEndian(bool littleEndian)
{
#if CPU(BIG_ENDIAN)
    return littleEndian;
#else
    return !littleEndian;
#endif
}

inline uint16_t flipBytes(uint16_t value)
{
    return ((value & 0x00ff) << 8)
        | ((value & 0xff00) >> 8);
}

inline uint32_t flipBytes(uint32_t value)
{
    return ((value & 0x000000ff) << 24)
        | ((value & 0x0000ff00) << 8)
        | ((value & 0x00ff0000) >> 8)
        | ((value & 0xff000000) >> 24);
}

inline uint64_t flipBytes(uint64_t value)
{
    return ((value & 0x00000000000000ffull) << 56)
        | ((value & 0x000000000000ff00ull) << 40)
        | ((value & 0x0000000000ff0000ull) << 24)
        | ((value & 0x00000000ff000000ull) << 8)
        | ((value & 0x000000ff00000000ull) >> 8)
        | ((value & 0x0000ff0000000000ull) >> 24)
        | ((value & 0x00ff000000000000ull) >> 40)
        | ((value & 0xff00000000000000ull) >> 56);
}

template<typename T>
inline T flipBytes(T value)
{
    if (sizeof(value) == 1)
        return value;
    if (sizeof(value) == 2) {
        union {
            T original;
            uint16_t word;
        } u;
        u.original = value;
        u.word = flipBytes(u.word);
        return u.original;
    }
    if (sizeof(value) == 4) {
        union {
            T original;
            uint32_t word;
        } u;
        u.original = value;
        u.word = flipBytes(u.word);
        return u.original;
    }
    if (sizeof(value) == 8) {
        union {
            T original;
            uint64_t word;
        } u;
        u.original = value;
        u.word = flipBytes(u.word);
        return u.original;
    }
    RELEASE_ASSERT_NOT_REACHED();
    return T();
}

template<typename T>
inline T flipBytesIfLittleEndian(T value, bool littleEndian)
{
    if (needToFlipBytesIfLittleEndian(littleEndian))
        return flipBytes(value);
    return value;
}

} // namespace WTF

using WTF::needToFlipBytesIfLittleEndian;
using WTF::flipBytes;
using WTF::flipBytesIfLittleEndian;

#endif // FlipBytes_h

