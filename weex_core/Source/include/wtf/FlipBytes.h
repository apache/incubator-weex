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

