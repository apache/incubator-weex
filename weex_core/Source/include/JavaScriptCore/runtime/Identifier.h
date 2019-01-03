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

#include "VM.h"
#include <wtf/Optional.h>
#include <wtf/ThreadSpecific.h>
#include <wtf/WTFThreadData.h>
#include <wtf/text/CString.h>
#include <wtf/text/UniquedStringImpl.h>
#include <wtf/text/WTFString.h>

namespace JSC {

class ExecState;

ALWAYS_INLINE bool isIndex(uint32_t index)
{
    return index != 0xFFFFFFFFU;
}

template <typename CharType>
ALWAYS_INLINE std::optional<uint32_t> parseIndex(const CharType* characters, unsigned length)
{
    // An empty string is not a number.
    if (!length)
        return std::nullopt;

    // Get the first character, turning it into a digit.
    uint32_t value = characters[0] - '0';
    if (value > 9)
        return std::nullopt;

    // Check for leading zeros. If the first characher is 0, then the
    // length of the string must be one - e.g. "042" is not equal to "42".
    if (!value && length > 1)
        return std::nullopt;

    while (--length) {
        // Multiply value by 10, checking for overflow out of 32 bits.
        if (value > 0xFFFFFFFFU / 10)
            return std::nullopt;
        value *= 10;

        // Get the next character, turning it into a digit.
        uint32_t newValue = *(++characters) - '0';
        if (newValue > 9)
            return std::nullopt;

        // Add in the old value, checking for overflow out of 32 bits.
        newValue += value;
        if (newValue < value)
            return std::nullopt;
        value = newValue;
    }

    if (!isIndex(value))
        return std::nullopt;
    return value;
}

ALWAYS_INLINE std::optional<uint32_t> parseIndex(StringImpl& impl)
{
    if (impl.is8Bit())
        return parseIndex(impl.characters8(), impl.length());
    return parseIndex(impl.characters16(), impl.length());
}

class Identifier {
    friend class Structure;
public:
    Identifier() { }
    enum EmptyIdentifierFlag { EmptyIdentifier };
    Identifier(EmptyIdentifierFlag) : m_string(StringImpl::empty()) { ASSERT(m_string.impl()->isAtomic()); }

    const String& string() const { return m_string; }
    UniquedStringImpl* impl() const { return static_cast<UniquedStringImpl*>(m_string.impl()); }

    int length() const { return m_string.length(); }

    CString ascii() const { return m_string.ascii(); }
    CString utf8() const { return m_string.utf8(); }

    // There's 2 functions to construct Identifier from string, (1) fromString and (2) fromUid.
    // They have different meanings in keeping or discarding symbol-ness of strings.
    // (1): fromString
    // Just construct Identifier from string. String held by Identifier is always atomized.
    // Symbol-ness of StringImpl*, which represents that the string is inteded to be used for ES6 Symbols, is discarded.
    // So a constructed Identifier never represents a symbol.
    // (2): fromUid
    // `StringImpl* uid` represents ether String or Symbol property.
    // fromUid keeps symbol-ness of provided StringImpl* while fromString discards it.
    // Use fromUid when constructing Identifier from StringImpl* which may represent symbols.

    // Only to be used with string literals.
    template<unsigned charactersCount>
    static Identifier fromString(VM*, const char (&characters)[charactersCount]);
    template<unsigned charactersCount>
    static Identifier fromString(ExecState*, const char (&characters)[charactersCount]);
    static Identifier fromString(VM*, const LChar*, int length);
    static Identifier fromString(VM*, const UChar*, int length);
    static Identifier fromString(VM*, const String&);
    static Identifier fromString(ExecState*, AtomicStringImpl*);
    static Identifier fromString(ExecState*, const AtomicString&);
    static Identifier fromString(ExecState*, const String&);
    static Identifier fromString(ExecState*, const char*);

    static Identifier fromUid(VM*, UniquedStringImpl* uid);
    static Identifier fromUid(ExecState*, UniquedStringImpl* uid);
    static Identifier fromUid(const PrivateName&);

    static Identifier createLCharFromUChar(VM* vm, const UChar* s, int length) { return Identifier(vm, add8(vm, s, length)); }

    JS_EXPORT_PRIVATE static Identifier from(ExecState*, unsigned y);
    JS_EXPORT_PRIVATE static Identifier from(ExecState*, int y);
    static Identifier from(ExecState*, double y);
    static Identifier from(VM*, unsigned y);
    static Identifier from(VM*, int y);
    static Identifier from(VM*, double y);

    bool isNull() const { return m_string.isNull(); }
    bool isEmpty() const { return m_string.isEmpty(); }
    bool isSymbol() const { return !isNull() && impl()->isSymbol(); }

    friend bool operator==(const Identifier&, const Identifier&);
    friend bool operator!=(const Identifier&, const Identifier&);

    friend bool operator==(const Identifier&, const LChar*);
    friend bool operator==(const Identifier&, const char*);
    friend bool operator!=(const Identifier&, const LChar*);
    friend bool operator!=(const Identifier&, const char*);

    static bool equal(const StringImpl*, const LChar*);
    static inline bool equal(const StringImpl*a, const char*b) { return Identifier::equal(a, reinterpret_cast<const LChar*>(b)); };
    static bool equal(const StringImpl*, const LChar*, unsigned length);
    static bool equal(const StringImpl*, const UChar*, unsigned length);
    static bool equal(const StringImpl* a, const StringImpl* b) { return ::equal(a, b); }

    // Only to be used with string literals.
    JS_EXPORT_PRIVATE static Ref<StringImpl> add(VM*, const char*);
    JS_EXPORT_PRIVATE static Ref<StringImpl> add(ExecState*, const char*);

    void dump(PrintStream&) const;

private:
    String m_string;

    // Only to be used with string literals.
    template<unsigned charactersCount>
    Identifier(VM* vm, const char (&characters)[charactersCount]) : m_string(add(vm, characters)) { ASSERT(m_string.impl()->isAtomic()); }

    Identifier(VM* vm, const LChar* s, int length) : m_string(add(vm, s, length)) { ASSERT(m_string.impl()->isAtomic()); }
    Identifier(VM* vm, const UChar* s, int length) : m_string(add(vm, s, length)) { ASSERT(m_string.impl()->isAtomic()); }
    Identifier(ExecState*, AtomicStringImpl*);
    Identifier(ExecState*, const AtomicString&);
    Identifier(VM* vm, const String& string) : m_string(add(vm, string.impl())) { ASSERT(m_string.impl()->isAtomic()); }
    Identifier(VM* vm, StringImpl* rep) : m_string(add(vm, rep)) { ASSERT(m_string.impl()->isAtomic()); }

    Identifier(SymbolImpl& uid)
        : m_string(&uid)
    {
    }

    template <typename CharType>
    ALWAYS_INLINE static uint32_t toUInt32FromCharacters(const CharType* characters, unsigned length, bool& ok);

    static bool equal(const Identifier& a, const Identifier& b) { return a.m_string.impl() == b.m_string.impl(); }
    static bool equal(const Identifier& a, const LChar* b) { return equal(a.m_string.impl(), b); }

    template <typename T> static Ref<StringImpl> add(VM*, const T*, int length);
    static Ref<StringImpl> add8(VM*, const UChar*, int length);
    template <typename T> ALWAYS_INLINE static bool canUseSingleCharacterString(T);

    static Ref<StringImpl> add(ExecState*, StringImpl*);
    static Ref<StringImpl> add(VM*, StringImpl*);

#ifndef NDEBUG
    JS_EXPORT_PRIVATE static void checkCurrentAtomicStringTable(ExecState*);
    JS_EXPORT_PRIVATE static void checkCurrentAtomicStringTable(VM*);
#else
    JS_EXPORT_PRIVATE NO_RETURN_DUE_TO_CRASH static void checkCurrentAtomicStringTable(ExecState*);
    JS_EXPORT_PRIVATE NO_RETURN_DUE_TO_CRASH static void checkCurrentAtomicStringTable(VM*);
#endif
};

template <> ALWAYS_INLINE bool Identifier::canUseSingleCharacterString(LChar)
{
    ASSERT(maxSingleCharacterString == 0xff);
    return true;
}

template <> ALWAYS_INLINE bool Identifier::canUseSingleCharacterString(UChar c)
{
    return (c <= maxSingleCharacterString);
}

template <typename T>
Ref<StringImpl> Identifier::add(VM* vm, const T* s, int length)
{
    if (length == 1) {
        T c = s[0];
        if (canUseSingleCharacterString(c))
            return vm->smallStrings.singleCharacterStringRep(c);
    }
    if (!length)
        return *StringImpl::empty();

    return *AtomicStringImpl::add(s, length);
}

inline bool operator==(const Identifier& a, const Identifier& b)
{
    return Identifier::equal(a, b);
}

inline bool operator!=(const Identifier& a, const Identifier& b)
{
    return !Identifier::equal(a, b);
}

inline bool operator==(const Identifier& a, const LChar* b)
{
    return Identifier::equal(a, b);
}

inline bool operator==(const Identifier& a, const char* b)
{
    return Identifier::equal(a, reinterpret_cast<const LChar*>(b));
}

inline bool operator!=(const Identifier& a, const LChar* b)
{
    return !Identifier::equal(a, b);
}

inline bool operator!=(const Identifier& a, const char* b)
{
    return !Identifier::equal(a, reinterpret_cast<const LChar*>(b));
}

inline bool Identifier::equal(const StringImpl* r, const LChar* s)
{
    return WTF::equal(r, s);
}

inline bool Identifier::equal(const StringImpl* r, const LChar* s, unsigned length)
{
    return WTF::equal(r, s, length);
}

inline bool Identifier::equal(const StringImpl* r, const UChar* s, unsigned length)
{
    return WTF::equal(r, s, length);
}

ALWAYS_INLINE std::optional<uint32_t> parseIndex(const Identifier& identifier)
{
    auto uid = identifier.impl();
    if (!uid)
        return std::nullopt;
    if (uid->isSymbol())
        return std::nullopt;
    return parseIndex(*uid);
}

JSValue identifierToJSValue(VM&, const Identifier&);
// This will stringify private symbols. When leaking JSValues to
// non-internal code, make sure to use this function and not the above one.
JSValue identifierToSafePublicJSValue(VM&, const Identifier&);

// FIXME: It may be better for this to just be a typedef for PtrHash, since PtrHash may be cheaper to
// compute than loading the StringImpl's hash from memory. That change would also reduce the likelihood of
// crashes in code that somehow dangled a StringImpl.
// https://bugs.webkit.org/show_bug.cgi?id=150137
struct IdentifierRepHash : PtrHash<RefPtr<UniquedStringImpl>> {
    static unsigned hash(const RefPtr<UniquedStringImpl>& key) { return key->existingSymbolAwareHash(); }
    static unsigned hash(UniquedStringImpl* key) { return key->existingSymbolAwareHash(); }
};

struct IdentifierMapIndexHashTraits : HashTraits<int> {
    static int emptyValue() { return std::numeric_limits<int>::max(); }
    static const bool emptyValueIsZero = false;
};

typedef HashSet<RefPtr<UniquedStringImpl>, IdentifierRepHash> IdentifierSet;
typedef HashMap<RefPtr<UniquedStringImpl>, int, IdentifierRepHash, HashTraits<RefPtr<UniquedStringImpl>>, IdentifierMapIndexHashTraits> IdentifierMap;
typedef HashMap<UniquedStringImpl*, int, IdentifierRepHash, HashTraits<UniquedStringImpl*>, IdentifierMapIndexHashTraits> BorrowedIdentifierMap;

} // namespace JSC

namespace WTF {

template <> struct VectorTraits<JSC::Identifier> : SimpleClassVectorTraits { };

} // namespace WTF
