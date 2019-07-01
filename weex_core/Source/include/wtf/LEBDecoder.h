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

#include "Compiler.h"
#include <algorithm>

// This file contains a bunch of helper functions for decoding LEB numbers.
// See https://en.wikipedia.org/wiki/LEB128 for more information about the
// LEB format.

namespace WTF { namespace LEBDecoder {

template<typename T>
inline bool WARN_UNUSED_RETURN decodeUInt(const uint8_t* bytes, size_t length, size_t& offset, T& result)
{
    const size_t numBits = sizeof(T) * CHAR_BIT;
    const size_t maxByteLength = (numBits - 1) / 7 + 1; // numBits / 7 rounding up.
    if (length <= offset)
        return false;
    result = 0;
    unsigned shift = 0;
    size_t last = std::min(maxByteLength, length - offset) - 1;
    for (unsigned i = 0; true; ++i) {
        uint8_t byte = bytes[offset++];
        result |= static_cast<T>(byte & 0x7f) << shift;
        shift += 7;
        if (!(byte & 0x80))
            return true;
        if (i == last)
            return false;
    }
    RELEASE_ASSERT_NOT_REACHED();
    return true;
}

template<typename T>
inline bool WARN_UNUSED_RETURN decodeInt(const uint8_t* bytes, size_t length, size_t& offset, T& result)
{
    const size_t numBits = sizeof(T) * CHAR_BIT;
    const size_t maxByteLength = (numBits - 1) / 7 + 1; // numBits / 7 rounding up.
    if (length <= offset)
        return false;
    result = 0;
    unsigned shift = 0;
    size_t last = std::min(maxByteLength, length - offset) - 1;
    uint8_t byte;
    for (unsigned i = 0; true; ++i) {
        byte = bytes[offset++];
        result |= static_cast<T>(byte & 0x7f) << shift;
        shift += 7;
        if (!(byte & 0x80))
            break;
        if (i == last)
            return false;
    }

    using UnsignedT = typename std::make_unsigned<T>::type;
    if (shift < numBits && (byte & 0x40))
        result = static_cast<T>(static_cast<UnsignedT>(result) | (static_cast<UnsignedT>(-1) << shift));
    return true;
}

inline bool WARN_UNUSED_RETURN decodeUInt32(const uint8_t* bytes, size_t length, size_t& offset, uint32_t& result)
{
    return decodeUInt<uint32_t>(bytes, length, offset, result);
}

inline bool WARN_UNUSED_RETURN decodeUInt64(const uint8_t* bytes, size_t length, size_t& offset, uint64_t& result)
{
    return decodeUInt<uint64_t>(bytes, length, offset, result);
}

inline bool WARN_UNUSED_RETURN decodeInt32(const uint8_t* bytes, size_t length, size_t& offset, int32_t& result)
{
    return decodeInt<int32_t>(bytes, length, offset, result);
}

inline bool WARN_UNUSED_RETURN decodeInt64(const uint8_t* bytes, size_t length, size_t& offset, int64_t& result)
{
    return decodeInt<int64_t>(bytes, length, offset, result);
}

} } // WTF::LEBDecoder
