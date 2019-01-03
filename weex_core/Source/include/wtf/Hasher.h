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
#ifndef WTF_Hasher_h
#define WTF_Hasher_h

#include <unicode/utypes.h>
#include <wtf/text/LChar.h>

namespace WTF {

// Paul Hsieh's SuperFastHash
// http://www.azillionmonkeys.com/qed/hash.html

// LChar data is interpreted as Latin-1-encoded (zero extended to 16 bits).

// NOTE: The hash computation here must stay in sync with the create_hash_table script in
// JavaScriptCore and the CodeGeneratorJS.pm script in WebCore.

// Golden ratio. Arbitrary start value to avoid mapping all zeros to a hash value of zero.
static constexpr const unsigned stringHashingStartValue = 0x9E3779B9U;

class StringHasher {
public:
    static constexpr const unsigned flagCount = 8; // Save 8 bits for StringImpl to use as flags.
    static constexpr const unsigned maskHash = (1U << (sizeof(unsigned) * 8 - flagCount)) - 1;

    StringHasher()
        : m_hash(stringHashingStartValue)
        , m_hasPendingCharacter(false)
        , m_pendingCharacter(0)
    {
    }

    // The hasher hashes two characters at a time, and thus an "aligned" hasher is one
    // where an even number of characters have been added. Callers that always add
    // characters two at a time can use the "assuming aligned" functions.
    void addCharactersAssumingAligned(UChar a, UChar b)
    {
        ASSERT(!m_hasPendingCharacter);
        m_hash += a;
        m_hash = (m_hash << 16) ^ ((b << 11) ^ m_hash);
        m_hash += m_hash >> 11;
    }

    void addCharacter(UChar character)
    {
        if (m_hasPendingCharacter) {
            m_hasPendingCharacter = false;
            addCharactersAssumingAligned(m_pendingCharacter, character);
            return;
        }

        m_pendingCharacter = character;
        m_hasPendingCharacter = true;
    }

    void addCharacters(UChar a, UChar b)
    {
        if (m_hasPendingCharacter) {
#if !ASSERT_DISABLED
            m_hasPendingCharacter = false;
#endif
            addCharactersAssumingAligned(m_pendingCharacter, a);
            m_pendingCharacter = b;
#if !ASSERT_DISABLED
            m_hasPendingCharacter = true;
#endif
            return;
        }

        addCharactersAssumingAligned(a, b);
    }

    template<typename T, UChar Converter(T)> void addCharactersAssumingAligned(const T* data, unsigned length)
    {
        ASSERT(!m_hasPendingCharacter);

        bool remainder = length & 1;
        length >>= 1;

        while (length--) {
            addCharactersAssumingAligned(Converter(data[0]), Converter(data[1]));
            data += 2;
        }

        if (remainder)
            addCharacter(Converter(*data));
    }

    template<typename T> void addCharactersAssumingAligned(const T* data, unsigned length)
    {
        addCharactersAssumingAligned<T, defaultConverter>(data, length);
    }

    template<typename T, UChar Converter(T)> void addCharactersAssumingAligned(const T* data)
    {
        ASSERT(!m_hasPendingCharacter);

        while (T a = *data++) {
            T b = *data++;
            if (!b) {
                addCharacter(Converter(a));
                break;
            }
            addCharactersAssumingAligned(Converter(a), Converter(b));
        }
    }

    template<typename T> void addCharactersAssumingAligned(const T* data)
    {
        addCharactersAssumingAligned<T, defaultConverter>(data);
    }

    template<typename T, UChar Converter(T)> void addCharacters(const T* data, unsigned length)
    {
        if (!length)
            return;
        if (m_hasPendingCharacter) {
            m_hasPendingCharacter = false;
            addCharactersAssumingAligned(m_pendingCharacter, Converter(*data++));
            --length;
        }
        addCharactersAssumingAligned<T, Converter>(data, length);
    }

    template<typename T> void addCharacters(const T* data, unsigned length)
    {
        addCharacters<T, defaultConverter>(data, length);
    }

    template<typename T, UChar Converter(T)> void addCharacters(const T* data)
    {
        if (m_hasPendingCharacter && *data) {
            m_hasPendingCharacter = false;
            addCharactersAssumingAligned(m_pendingCharacter, Converter(*data++));
        }
        addCharactersAssumingAligned<T, Converter>(data);
    }

    template<typename T> void addCharacters(const T* data)
    {
        addCharacters<T, defaultConverter>(data);
    }

    unsigned hashWithTop8BitsMasked() const
    {
        return finalizeAndMaskTop8Bits(processPendingCharacter());
    }

    unsigned hash() const
    {
        return finalize(processPendingCharacter());
    }

    template<typename T, UChar Converter(T)> static unsigned computeHashAndMaskTop8Bits(const T* data, unsigned length)
    {
        StringHasher hasher;
        hasher.addCharactersAssumingAligned<T, Converter>(data, length);
        return hasher.hashWithTop8BitsMasked();
    }

    template<typename T, UChar Converter(T)> static unsigned computeHashAndMaskTop8Bits(const T* data)
    {
        StringHasher hasher;
        hasher.addCharactersAssumingAligned<T, Converter>(data);
        return hasher.hashWithTop8BitsMasked();
    }

    template<typename T> static unsigned computeHashAndMaskTop8Bits(const T* data, unsigned length)
    {
        return computeHashAndMaskTop8Bits<T, defaultConverter>(data, length);
    }

    template<typename T> static unsigned computeHashAndMaskTop8Bits(const T* data)
    {
        return computeHashAndMaskTop8Bits<T, defaultConverter>(data);
    }

    template<typename T, UChar Converter(T)> static unsigned computeHash(const T* data, unsigned length)
    {
        StringHasher hasher;
        hasher.addCharactersAssumingAligned<T, Converter>(data, length);
        return hasher.hash();
    }

    template<typename T, UChar Converter(T)> static unsigned computeHash(const T* data)
    {
        StringHasher hasher;
        hasher.addCharactersAssumingAligned<T, Converter>(data);
        return hasher.hash();
    }

    template<typename T> static unsigned computeHash(const T* data, unsigned length)
    {
        return computeHash<T, defaultConverter>(data, length);
    }

    template<typename T> static unsigned computeHash(const T* data)
    {
        return computeHash<T, defaultConverter>(data);
    }

    static unsigned hashMemory(const void* data, unsigned length)
    {
        size_t lengthInUChar = length / sizeof(UChar);
        StringHasher hasher;
        hasher.addCharactersAssumingAligned(static_cast<const UChar*>(data), lengthInUChar);

        for (size_t i = 0; i < length % sizeof(UChar); ++i)
            hasher.addCharacter(static_cast<const char*>(data)[lengthInUChar * sizeof(UChar) + i]);

        return hasher.hash();
    }

    template<size_t length> static unsigned hashMemory(const void* data)
    {
        return hashMemory(data, length);
    }

    static constexpr unsigned finalize(unsigned hash)
    {
        return avoidZero(avalancheBits(hash));
    }

    static constexpr unsigned finalizeAndMaskTop8Bits(unsigned hash)
    {
        // Reserving space from the high bits for flags preserves most of the hash's
        // value, since hash lookup typically masks out the high bits anyway.
        return avoidZero(avalancheBits(hash) & StringHasher::maskHash);
    }

    template<typename T, unsigned charactersCount>
    static constexpr unsigned computeLiteralHash(const T (&characters)[charactersCount])
    {
        return StringHasher::finalize(computeLiteralHashImpl(stringHashingStartValue, 0, characters, charactersCount - 1));
    }

    template<typename T, unsigned charactersCount>
    static constexpr unsigned computeLiteralHashAndMaskTop8Bits(const T (&characters)[charactersCount])
    {
        return StringHasher::finalizeAndMaskTop8Bits(computeLiteralHashImpl(stringHashingStartValue, 0, characters, charactersCount - 1));
    }

private:
    static UChar defaultConverter(UChar character)
    {
        return character;
    }

    static UChar defaultConverter(LChar character)
    {
        return character;
    }

    ALWAYS_INLINE static constexpr unsigned avalancheBits3(unsigned hash)
    {
        return hash ^ (hash << 10);
    }

    ALWAYS_INLINE static constexpr unsigned avalancheBits2(unsigned hash)
    {
        return avalancheBits3(hash + (hash >> 15));
    }

    ALWAYS_INLINE static constexpr unsigned avalancheBits1(unsigned hash)
    {
        return avalancheBits2(hash ^ (hash << 2));
    }

    ALWAYS_INLINE static constexpr unsigned avalancheBits0(unsigned hash)
    {
        return avalancheBits1(hash + (hash >> 5));
    }

    ALWAYS_INLINE static constexpr unsigned avalancheBits(unsigned hash)
    {
        return avalancheBits0(hash ^ (hash << 3));
    }

    // This avoids ever returning a hash code of 0, since that is used to
    // signal "hash not computed yet". Setting the high bit maintains
    // reasonable fidelity to a hash code of 0 because it is likely to yield
    // exactly 0 when hash lookup masks out the high bits.
    ALWAYS_INLINE static constexpr unsigned avoidZero(unsigned hash)
    {
        return hash ? hash : (0x80000000 >> StringHasher::flagCount);
    }

    unsigned processPendingCharacter() const
    {
        unsigned result = m_hash;

        // Handle end case.
        if (m_hasPendingCharacter) {
            result += m_pendingCharacter;
            result ^= result << 11;
            result += result >> 17;
        }
        return result;
    }


    // FIXME: This code limits itself to the older, more limited C++11 constexpr capabilities, using
    // recursion instead of looping, for example. Would be nice to rewrite this in a simpler way
    // once we no longer need to support compilers like GCC 4.9 that do not yet support it.
    static constexpr unsigned calculateWithRemainingLastCharacter1(unsigned hash)
    {
        return hash + (hash >> 17);
    }

    static constexpr unsigned calculateWithRemainingLastCharacter0(unsigned hash)
    {
        return calculateWithRemainingLastCharacter1((hash << 11) ^ hash);
    }

    static constexpr unsigned calculateWithRemainingLastCharacter(unsigned hash, unsigned character)
    {
        return calculateWithRemainingLastCharacter0(hash + character);
    }

    static constexpr unsigned calculate1(unsigned hash)
    {
        return hash + (hash >> 11);
    }

    static constexpr unsigned calculate0(unsigned hash, unsigned secondCharacter)
    {
        return calculate1((hash << 16) ^ ((secondCharacter << 11) ^ hash));
    }

    static constexpr unsigned calculate(unsigned hash, unsigned firstCharacter, unsigned secondCharacter)
    {
        return calculate0(hash + firstCharacter, secondCharacter);
    }

    static constexpr unsigned computeLiteralHashImpl(unsigned hash, unsigned index, const char* characters, unsigned length)
    {
        return (index == length)
            ? hash : ((index + 1) == length)
            ? calculateWithRemainingLastCharacter(hash, characters[index])
            : computeLiteralHashImpl(calculate(hash, characters[index], characters[index + 1]), index + 2, characters, length);
    }

    unsigned m_hash;
    bool m_hasPendingCharacter;
    UChar m_pendingCharacter;
};

class IntegerHasher {
public:
    void add(unsigned integer)
    {
        m_underlyingHasher.addCharactersAssumingAligned(integer, integer >> 16);
    }

    unsigned hash() const
    {
        return m_underlyingHasher.hash();
    }

private:
    StringHasher m_underlyingHasher;
};

} // namespace WTF

using WTF::IntegerHasher;
using WTF::StringHasher;

#endif // WTF_Hasher_h
