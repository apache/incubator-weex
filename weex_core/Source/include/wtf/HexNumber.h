/*
 * Copyright (C) 2011 Research In Motion Limited. All rights reserved.
 * Copyright (C) 2016 Apple Inc. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#pragma once

#include <wtf/text/StringConcatenate.h>

namespace WTF {

enum HexConversionMode { Lowercase, Uppercase };

namespace Internal {

inline const LChar* hexDigitsForMode(HexConversionMode mode)
{
    static const LChar lowercaseHexDigits[17] = "0123456789abcdef";
    static const LChar uppercaseHexDigits[17] = "0123456789ABCDEF";
    return mode == Lowercase ? lowercaseHexDigits : uppercaseHexDigits;
}

} // namespace Internal

template<typename T>
inline void appendByteAsHex(unsigned char byte, T& destination, HexConversionMode mode = Uppercase)
{
    auto* hexDigits = Internal::hexDigitsForMode(mode);
    destination.append(hexDigits[byte >> 4]);
    destination.append(hexDigits[byte & 0xF]);
}

template<typename T>
inline void placeByteAsHexCompressIfPossible(unsigned char byte, T& destination, unsigned& index, HexConversionMode mode = Uppercase)
{
    auto* hexDigits = Internal::hexDigitsForMode(mode);
    if (byte >= 0x10)
        destination[index++] = hexDigits[byte >> 4];
    destination[index++] = hexDigits[byte & 0xF];
}

template<typename T>
inline void placeByteAsHex(unsigned char byte, T& destination, HexConversionMode mode = Uppercase)
{
    auto* hexDigits = Internal::hexDigitsForMode(mode);
    *destination++ = hexDigits[byte >> 4];
    *destination++ = hexDigits[byte & 0xF];
}

template<typename T>
inline void appendUnsignedAsHex(unsigned number, T& destination, HexConversionMode mode = Uppercase)
{
    auto* hexDigits = Internal::hexDigitsForMode(mode);
    Vector<LChar, 8> result;
    do {
        result.append(hexDigits[number % 16]);
        number >>= 4;
    } while (number > 0);

    result.reverse();
    destination.append(result.data(), result.size());
}
    
template<typename T>
inline void appendUnsigned64AsHex(uint64_t number, T& destination, HexConversionMode mode = Uppercase)
{
    auto* hexDigits = Internal::hexDigitsForMode(mode);
    Vector<LChar, 8> result;
    do {
        result.append(hexDigits[number % 16]);
        number >>= 4;
    } while (number > 0);
    
    result.reverse();
    destination.append(result.data(), result.size());
}

// Same as appendUnsignedAsHex, but using exactly 'desiredDigits' for the conversion.
template<typename T>
inline void appendUnsignedAsHexFixedSize(unsigned number, T& destination, unsigned desiredDigits, HexConversionMode mode = Uppercase)
{
    ASSERT(desiredDigits);

    auto* hexDigits = Internal::hexDigitsForMode(mode);
    Vector<LChar, 8> result;
    do {
        result.append(hexDigits[number % 16]);
        number >>= 4;
    } while (result.size() < desiredDigits);

    ASSERT(result.size() == desiredDigits);
    result.reverse();
    destination.append(result.data(), result.size());
}

} // namespace WTF

using WTF::appendByteAsHex;
using WTF::appendUnsignedAsHex;
using WTF::appendUnsignedAsHexFixedSize;
using WTF::placeByteAsHex;
using WTF::placeByteAsHexCompressIfPossible;
using WTF::Lowercase;
