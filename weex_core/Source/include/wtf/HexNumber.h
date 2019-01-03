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
