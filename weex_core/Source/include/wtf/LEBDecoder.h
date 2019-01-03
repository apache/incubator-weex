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
