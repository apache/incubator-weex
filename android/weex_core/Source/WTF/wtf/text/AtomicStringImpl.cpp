/*
 * Copyright (C) 2004-2008, 2013-2014 Apple Inc. All rights reserved.
 * Copyright (C) 2010 Patrick Gansterer <paroga@paroga.com>
 * Copyright (C) 2012 Google Inc. All rights reserved.
 * Copyright (C) 2015 Yusuke Suzuki<utatane.tea@gmail.com>. All rights reserved.
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
 *
 */

#include "config.h"
#include "AtomicStringImpl.h"

#include "AtomicStringTable.h"
#include "CommaPrinter.h"
#include "DataLog.h"
#include "HashSet.h"
#include "IntegerToStringConversion.h"
#include "StringHash.h"
#include "StringPrintStream.h"
#include "Threading.h"
#include "WTFThreadData.h"
#include <wtf/unicode/WTFUTF8.h>

#if USE(WEB_THREAD)
#include "Lock.h"
#endif

namespace WTF {

using namespace Unicode;

#if USE(WEB_THREAD)

class AtomicStringTableLocker : public LockHolder {
    WTF_MAKE_NONCOPYABLE(AtomicStringTableLocker);

    static StaticLock s_stringTableLock;
public:
    AtomicStringTableLocker()
        : LockHolder(&s_stringTableLock)
    {
    }
};

StaticLock AtomicStringTableLocker::s_stringTableLock;

#else

class AtomicStringTableLocker {
    WTF_MAKE_NONCOPYABLE(AtomicStringTableLocker);
public:
    AtomicStringTableLocker() { }
};

#endif // USE(WEB_THREAD)

using StringTableImpl = HashSet<StringImpl*>;

static ALWAYS_INLINE StringTableImpl& stringTable()
{
    return wtfThreadData().atomicStringTable()->table();
}

template<typename T, typename HashTranslator>
static inline Ref<AtomicStringImpl> addToStringTable(AtomicStringTableLocker&, StringTableImpl& atomicStringTable, const T& value)
{
    auto addResult = atomicStringTable.add<HashTranslator>(value);

    // If the string is newly-translated, then we need to adopt it.
    // The boolean in the pair tells us if that is so.
    if (addResult.isNewEntry)
        return adoptRef(static_cast<AtomicStringImpl&>(**addResult.iterator));
    return *static_cast<AtomicStringImpl*>(*addResult.iterator);
}

template<typename T, typename HashTranslator>
static inline Ref<AtomicStringImpl> addToStringTable(const T& value)
{
    AtomicStringTableLocker locker;
    return addToStringTable<T, HashTranslator>(locker, stringTable(), value);
}

struct CStringTranslator {
    static unsigned hash(const LChar* c)
    {
        return StringHasher::computeHashAndMaskTop8Bits(c);
    }

    static inline bool equal(StringImpl* r, const LChar* s)
    {
        return WTF::equal(r, s);
    }

    static void translate(StringImpl*& location, const LChar* const& c, unsigned hash)
    {
        location = &StringImpl::create(c).leakRef();
        location->setHash(hash);
        location->setIsAtomic(true);
    }
};

RefPtr<AtomicStringImpl> AtomicStringImpl::add(const LChar* c)
{
    if (!c)
        return nullptr;
    if (!*c)
        return static_cast<AtomicStringImpl*>(StringImpl::empty());

    return addToStringTable<const LChar*, CStringTranslator>(c);
}

template<typename CharacterType>
struct HashTranslatorCharBuffer {
    const CharacterType* s;
    unsigned length;
};

typedef HashTranslatorCharBuffer<UChar> UCharBuffer;
struct UCharBufferTranslator {
    static unsigned hash(const UCharBuffer& buf)
    {
        return StringHasher::computeHashAndMaskTop8Bits(buf.s, buf.length);
    }

    static bool equal(StringImpl* const& str, const UCharBuffer& buf)
    {
        return WTF::equal(str, buf.s, buf.length);
    }

    static void translate(StringImpl*& location, const UCharBuffer& buf, unsigned hash)
    {
        location = &StringImpl::create8BitIfPossible(buf.s, buf.length).leakRef();
        location->setHash(hash);
        location->setIsAtomic(true);
    }
};

template<typename CharacterType>
struct HashAndCharacters {
    unsigned hash;
    const CharacterType* characters;
    unsigned length;
};

template<typename CharacterType>
struct HashAndCharactersTranslator {
    static unsigned hash(const HashAndCharacters<CharacterType>& buffer)
    {
        ASSERT(buffer.hash == StringHasher::computeHashAndMaskTop8Bits(buffer.characters, buffer.length));
        return buffer.hash;
    }

    static bool equal(StringImpl* const& string, const HashAndCharacters<CharacterType>& buffer)
    {
        return WTF::equal(string, buffer.characters, buffer.length);
    }

    static void translate(StringImpl*& location, const HashAndCharacters<CharacterType>& buffer, unsigned hash)
    {
        location = &StringImpl::create(buffer.characters, buffer.length).leakRef();
        location->setHash(hash);
        location->setIsAtomic(true);
    }
};

struct HashAndUTF8Characters {
    unsigned hash;
    const char* characters;
    unsigned length;
    unsigned utf16Length;
};

struct HashAndUTF8CharactersTranslator {
    static unsigned hash(const HashAndUTF8Characters& buffer)
    {
        return buffer.hash;
    }

    static bool equal(StringImpl* const& string, const HashAndUTF8Characters& buffer)
    {
        if (buffer.utf16Length != string->length())
            return false;

        // If buffer contains only ASCII characters UTF-8 and UTF16 length are the same.
        if (buffer.utf16Length != buffer.length) {
            if (string->is8Bit())
                return equalLatin1WithUTF8(string->characters8(), buffer.characters, buffer.characters + buffer.length);

            return equalUTF16WithUTF8(string->characters16(), buffer.characters, buffer.characters + buffer.length);
        }

        if (string->is8Bit()) {
            const LChar* stringCharacters = string->characters8();

            for (unsigned i = 0; i < buffer.length; ++i) {
                ASSERT(isASCII(buffer.characters[i]));
                if (stringCharacters[i] != buffer.characters[i])
                    return false;
            }

            return true;
        }

        const UChar* stringCharacters = string->characters16();

        for (unsigned i = 0; i < buffer.length; ++i) {
            ASSERT(isASCII(buffer.characters[i]));
            if (stringCharacters[i] != buffer.characters[i])
                return false;
        }

        return true;
    }

    static void translate(StringImpl*& location, const HashAndUTF8Characters& buffer, unsigned hash)
    {
        UChar* target;
        auto newString = StringImpl::createUninitialized(buffer.utf16Length, target);

        bool isAllASCII;
        const char* source = buffer.characters;
        if (convertUTF8ToUTF16(&source, source + buffer.length, &target, target + buffer.utf16Length, &isAllASCII) != conversionOK)
            ASSERT_NOT_REACHED();

        if (isAllASCII)
            newString = StringImpl::create(buffer.characters, buffer.length);

        location = &newString.leakRef();
        location->setHash(hash);
        location->setIsAtomic(true);
    }
};

RefPtr<AtomicStringImpl> AtomicStringImpl::add(const UChar* s, unsigned length)
{
    if (!s)
        return nullptr;

    if (!length)
        return static_cast<AtomicStringImpl*>(StringImpl::empty());

    UCharBuffer buffer = { s, length };
    return addToStringTable<UCharBuffer, UCharBufferTranslator>(buffer);
}

Ref<AtomicStringImpl> AtomicStringImpl::add(const UChar* s, unsigned length, unsigned existingHash)
{
    ASSERT(s);
    ASSERT(existingHash);

    if (!length)
        return *static_cast<AtomicStringImpl*>(StringImpl::empty());

    HashAndCharacters<UChar> buffer = { existingHash, s, length };
    return addToStringTable<HashAndCharacters<UChar>, HashAndCharactersTranslator<UChar>>(buffer);
}

RefPtr<AtomicStringImpl> AtomicStringImpl::add(const UChar* s)
{
    if (!s)
        return nullptr;

    unsigned length = 0;
    while (s[length] != UChar(0))
        ++length;

    if (!length)
        return static_cast<AtomicStringImpl*>(StringImpl::empty());

    UCharBuffer buffer = { s, length };
    return addToStringTable<UCharBuffer, UCharBufferTranslator>(buffer);
}

struct SubstringLocation {
    StringImpl* baseString;
    unsigned start;
    unsigned length;
};

struct SubstringTranslator {
    static void translate(StringImpl*& location, const SubstringLocation& buffer, unsigned hash)
    {
        location = &StringImpl::createSubstringSharingImpl(*buffer.baseString, buffer.start, buffer.length).leakRef();
        location->setHash(hash);
        location->setIsAtomic(true);
    }
};

struct SubstringTranslator8 : SubstringTranslator {
    static unsigned hash(const SubstringLocation& buffer)
    {
        return StringHasher::computeHashAndMaskTop8Bits(buffer.baseString->characters8() + buffer.start, buffer.length);
    }

    static bool equal(StringImpl* const& string, const SubstringLocation& buffer)
    {
        return WTF::equal(string, buffer.baseString->characters8() + buffer.start, buffer.length);
    }
};

struct SubstringTranslator16 : SubstringTranslator {
    static unsigned hash(const SubstringLocation& buffer)
    {
        return StringHasher::computeHashAndMaskTop8Bits(buffer.baseString->characters16() + buffer.start, buffer.length);
    }

    static bool equal(StringImpl* const& string, const SubstringLocation& buffer)
    {
        return WTF::equal(string, buffer.baseString->characters16() + buffer.start, buffer.length);
    }
};

RefPtr<AtomicStringImpl> AtomicStringImpl::add(StringImpl* baseString, unsigned start, unsigned length)
{
    if (!baseString)
        return nullptr;

    if (!length || start >= baseString->length())
        return static_cast<AtomicStringImpl*>(StringImpl::empty());

    unsigned maxLength = baseString->length() - start;
    if (length >= maxLength) {
        if (!start)
            return add(baseString);
        length = maxLength;
    }

    SubstringLocation buffer = { baseString, start, length };
    if (baseString->is8Bit())
        return addToStringTable<SubstringLocation, SubstringTranslator8>(buffer);
    return addToStringTable<SubstringLocation, SubstringTranslator16>(buffer);
}
    
typedef HashTranslatorCharBuffer<LChar> LCharBuffer;
struct LCharBufferTranslator {
    static unsigned hash(const LCharBuffer& buf)
    {
        return StringHasher::computeHashAndMaskTop8Bits(buf.s, buf.length);
    }

    static bool equal(StringImpl* const& str, const LCharBuffer& buf)
    {
        return WTF::equal(str, buf.s, buf.length);
    }

    static void translate(StringImpl*& location, const LCharBuffer& buf, unsigned hash)
    {
        location = &StringImpl::create(buf.s, buf.length).leakRef();
        location->setHash(hash);
        location->setIsAtomic(true);
    }
};

typedef HashTranslatorCharBuffer<char> CharBuffer;
struct CharBufferFromLiteralDataTranslator {
    static unsigned hash(const CharBuffer& buf)
    {
        return StringHasher::computeHashAndMaskTop8Bits(reinterpret_cast<const LChar*>(buf.s), buf.length);
    }

    static bool equal(StringImpl* const& str, const CharBuffer& buf)
    {
        return WTF::equal(str, buf.s, buf.length);
    }

    static void translate(StringImpl*& location, const CharBuffer& buf, unsigned hash)
    {
        location = &StringImpl::createFromLiteral(buf.s, buf.length).leakRef();
        location->setHash(hash);
        location->setIsAtomic(true);
    }
};

RefPtr<AtomicStringImpl> AtomicStringImpl::add(const LChar* s, unsigned length)
{
    if (!s)
        return nullptr;

    if (!length)
        return static_cast<AtomicStringImpl*>(StringImpl::empty());

    LCharBuffer buffer = { s, length };
    return addToStringTable<LCharBuffer, LCharBufferTranslator>(buffer);
}

Ref<AtomicStringImpl> AtomicStringImpl::addLiteral(const char* characters, unsigned length)
{
    ASSERT(characters);
    ASSERT(length);

    CharBuffer buffer = { characters, length };
    return addToStringTable<CharBuffer, CharBufferFromLiteralDataTranslator>(buffer);
}

static inline Ref<AtomicStringImpl> addSubstring(AtomicStringTableLocker& locker, StringTableImpl& atomicStringTable, StringImpl& base)
{
    ASSERT(base.length());
    ASSERT(base.isSymbol() || base.isStatic());

    SubstringLocation buffer = { &base, 0, base.length() };
    if (base.is8Bit())
        return addToStringTable<SubstringLocation, SubstringTranslator8>(locker, atomicStringTable, buffer);
    return addToStringTable<SubstringLocation, SubstringTranslator16>(locker, atomicStringTable, buffer);
}

static inline Ref<AtomicStringImpl> addSubstring(StringImpl& base)
{
    AtomicStringTableLocker locker;
    return addSubstring(locker, stringTable(), base);
}

Ref<AtomicStringImpl> AtomicStringImpl::addSlowCase(StringImpl& string)
{
    if (!string.length())
        return *static_cast<AtomicStringImpl*>(StringImpl::empty());

    if (string.isSymbol() || string.isStatic())
        return addSubstring(string);

    ASSERT_WITH_MESSAGE(!string.isAtomic(), "AtomicStringImpl should not hit the slow case if the string is already atomic.");

    AtomicStringTableLocker locker;
    auto addResult = stringTable().add(&string);

    if (addResult.isNewEntry) {
        ASSERT(*addResult.iterator == &string);
        string.setIsAtomic(true);
    }

    return *static_cast<AtomicStringImpl*>(*addResult.iterator);
}

Ref<AtomicStringImpl> AtomicStringImpl::addSlowCase(AtomicStringTable& stringTable, StringImpl& string)
{
    if (!string.length())
        return *static_cast<AtomicStringImpl*>(StringImpl::empty());

    if (string.isSymbol() || string.isStatic()) {
        AtomicStringTableLocker locker;
        return addSubstring(locker, stringTable.table(), string);
    }

    ASSERT_WITH_MESSAGE(!string.isAtomic(), "AtomicStringImpl should not hit the slow case if the string is already atomic.");

    AtomicStringTableLocker locker;
    auto addResult = stringTable.table().add(&string);

    if (addResult.isNewEntry) {
        ASSERT(*addResult.iterator == &string);
        string.setIsAtomic(true);
    }

    return *static_cast<AtomicStringImpl*>(*addResult.iterator);
}

void AtomicStringImpl::remove(AtomicStringImpl* string)
{
    ASSERT(string->isAtomic());
    AtomicStringTableLocker locker;
    auto& atomicStringTable = stringTable();
    auto iterator = atomicStringTable.find(string);
    ASSERT_WITH_MESSAGE(iterator != atomicStringTable.end(), "The string being removed is atomic in the string table of an other thread!");
    ASSERT(string == *iterator);
    atomicStringTable.remove(iterator);
}

RefPtr<AtomicStringImpl> AtomicStringImpl::lookUpSlowCase(StringImpl& string)
{
    ASSERT_WITH_MESSAGE(!string.isAtomic(), "AtomicStringImpls should return from the fast case.");

    if (!string.length())
        return static_cast<AtomicStringImpl*>(StringImpl::empty());

    AtomicStringTableLocker locker;
    auto& atomicStringTable = stringTable();
    auto iterator = atomicStringTable.find(&string);
    if (iterator != atomicStringTable.end())
        return static_cast<AtomicStringImpl*>(*iterator);
    return nullptr;
}

RefPtr<AtomicStringImpl> AtomicStringImpl::addUTF8(const char* charactersStart, const char* charactersEnd)
{
    HashAndUTF8Characters buffer;
    buffer.characters = charactersStart;
    buffer.hash = calculateStringHashAndLengthFromUTF8MaskingTop8Bits(charactersStart, charactersEnd, buffer.length, buffer.utf16Length);

    if (!buffer.hash)
        return nullptr;

    return addToStringTable<HashAndUTF8Characters, HashAndUTF8CharactersTranslator>(buffer);
}

RefPtr<AtomicStringImpl> AtomicStringImpl::lookUpInternal(const LChar* characters, unsigned length)
{
    AtomicStringTableLocker locker;
    auto& table = stringTable();

    LCharBuffer buffer = { characters, length };
    auto iterator = table.find<LCharBufferTranslator>(buffer);
    if (iterator != table.end())
        return static_cast<AtomicStringImpl*>(*iterator);
    return nullptr;
}

RefPtr<AtomicStringImpl> AtomicStringImpl::lookUpInternal(const UChar* characters, unsigned length)
{
    AtomicStringTableLocker locker;
    auto& table = stringTable();

    UCharBuffer buffer = { characters, length };
    auto iterator = table.find<UCharBufferTranslator>(buffer);
    if (iterator != table.end())
        return static_cast<AtomicStringImpl*>(*iterator);
    return nullptr;
}

#if !ASSERT_DISABLED
bool AtomicStringImpl::isInAtomicStringTable(StringImpl* string)
{
    AtomicStringTableLocker locker;
    return stringTable().contains(string);
}
#endif

} // namespace WTF
