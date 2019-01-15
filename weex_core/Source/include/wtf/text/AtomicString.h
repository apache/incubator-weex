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
#ifndef AtomicString_h
#define AtomicString_h

#include <utility>
#include <wtf/text/AtomicStringImpl.h>
#include <wtf/text/IntegerToStringConversion.h>
#include <wtf/text/WTFString.h>

// Define 'NO_IMPLICIT_ATOMICSTRING' before including this header,
// to disallow (expensive) implicit String-->AtomicString conversions.
#ifdef NO_IMPLICIT_ATOMICSTRING
#define ATOMICSTRING_CONVERSION explicit
#else
#define ATOMICSTRING_CONVERSION
#endif

namespace WTF {

struct AtomicStringHash;

class AtomicString {
public:
    WTF_EXPORT_PRIVATE static void init();

    AtomicString();
    AtomicString(const LChar*);
    AtomicString(const char*);
    AtomicString(const LChar*, unsigned length);
    AtomicString(const UChar*, unsigned length);
    AtomicString(const UChar*, unsigned length, unsigned existingHash);
    AtomicString(const UChar*);

    template<size_t inlineCapacity>
    explicit AtomicString(const Vector<UChar, inlineCapacity>& characters)
        : m_string(AtomicStringImpl::add(characters.data(), characters.size()))
    {
    }

    AtomicString(AtomicStringImpl*);
    AtomicString(RefPtr<AtomicStringImpl>&&);
    ATOMICSTRING_CONVERSION AtomicString(StringImpl*);
    ATOMICSTRING_CONVERSION AtomicString(const String&);
    AtomicString(StringImpl* baseString, unsigned start, unsigned length);

    // FIXME: AtomicString doesn’t always have AtomicStringImpl, so one of those two names needs to change..
    AtomicString(UniquedStringImpl* uid);

    enum ConstructFromLiteralTag { ConstructFromLiteral };
    AtomicString(const char* characters, unsigned length, ConstructFromLiteralTag)
        : m_string(AtomicStringImpl::addLiteral(characters, length))
    {
    }

    template<unsigned charactersCount>
    ALWAYS_INLINE AtomicString(const char (&characters)[charactersCount], ConstructFromLiteralTag)
        : m_string(AtomicStringImpl::addLiteral(characters, charactersCount - 1))
    {
        COMPILE_ASSERT(charactersCount > 1, AtomicStringFromLiteralNotEmpty);
        COMPILE_ASSERT((charactersCount - 1 <= ((unsigned(~0) - sizeof(StringImpl)) / sizeof(LChar))), AtomicStringFromLiteralCannotOverflow);
    }

    // We have to declare the copy constructor and copy assignment operator as well, otherwise
    // they'll be implicitly deleted by adding the move constructor and move assignment operator.
    AtomicString(const AtomicString& other) : m_string(other.m_string) { }
    AtomicString(AtomicString&& other) : m_string(WTFMove(other.m_string)) { }
    AtomicString& operator=(const AtomicString& other) { m_string = other.m_string; return *this; }
    AtomicString& operator=(AtomicString&& other) { m_string = WTFMove(other.m_string); return *this; }

    // Hash table deleted values, which are only constructed and never copied or destroyed.
    AtomicString(WTF::HashTableDeletedValueType) : m_string(WTF::HashTableDeletedValue) { }
    bool isHashTableDeletedValue() const { return m_string.isHashTableDeletedValue(); }

    unsigned existingHash() const { return isNull() ? 0 : impl()->existingHash(); }

    operator const String&() const { return m_string; }
    const String& string() const { return m_string; };

    AtomicStringImpl* impl() const { return static_cast<AtomicStringImpl *>(m_string.impl()); }

    bool is8Bit() const { return m_string.is8Bit(); }
    const LChar* characters8() const { return m_string.characters8(); }
    const UChar* characters16() const { return m_string.characters16(); }
    unsigned length() const { return m_string.length(); }

    UChar operator[](unsigned int i) const { return m_string[i]; }

    WTF_EXPORT_STRING_API static AtomicString number(int);
    WTF_EXPORT_STRING_API static AtomicString number(unsigned);
    WTF_EXPORT_STRING_API static AtomicString number(unsigned long);
    WTF_EXPORT_STRING_API static AtomicString number(unsigned long long);
    WTF_EXPORT_STRING_API static AtomicString number(double);
    // If we need more overloads of the number function, we can add all the others that String has, but these seem to do for now.

    bool contains(UChar c) const { return m_string.contains(c); }
    bool contains(const LChar* s, bool caseSensitive = true) const
        { return m_string.contains(s, caseSensitive); }
    bool contains(const String& s) const
        { return m_string.contains(s); }
    bool contains(const String& s, bool caseSensitive) const
        { return m_string.contains(s, caseSensitive); }
    bool containsIgnoringASCIICase(const String& s) const
        { return m_string.containsIgnoringASCIICase(s); }

    size_t find(UChar c, unsigned start = 0) const { return m_string.find(c, start); }
    size_t find(const LChar* s, unsigned start = 0, bool caseSentitive = true) const
        { return m_string.find(s, start, caseSentitive); }
    size_t find(const String& s, unsigned start = 0, bool caseSentitive = true) const
        { return m_string.find(s, start, caseSentitive); }
    size_t findIgnoringASCIICase(const String& s) const
        { return m_string.findIgnoringASCIICase(s); }
    size_t findIgnoringASCIICase(const String& s, unsigned startOffset) const
        { return m_string.findIgnoringASCIICase(s, startOffset); }
    size_t find(CharacterMatchFunctionPtr matchFunction, unsigned start = 0) const
        { return m_string.find(matchFunction, start); }

    bool startsWith(const String& s) const
        { return m_string.startsWith(s); }
    bool startsWithIgnoringASCIICase(const String& s) const
        { return m_string.startsWithIgnoringASCIICase(s); }
    bool startsWith(const String& s, bool caseSensitive) const
        { return m_string.startsWith(s, caseSensitive); }
    bool startsWith(UChar character) const
        { return m_string.startsWith(character); }
    template<unsigned matchLength>
    bool startsWith(const char (&prefix)[matchLength], bool caseSensitive = true) const
        { return m_string.startsWith<matchLength>(prefix, caseSensitive); }

    bool endsWith(const String& s) const
        { return m_string.endsWith(s); }
    bool endsWithIgnoringASCIICase(const String& s) const
        { return m_string.endsWithIgnoringASCIICase(s); }
    bool endsWith(const String& s, bool caseSensitive) const
        { return m_string.endsWith(s, caseSensitive); }
    bool endsWith(UChar character) const
        { return m_string.endsWith(character); }
    template<unsigned matchLength>
    bool endsWith(const char (&prefix)[matchLength], bool caseSensitive = true) const
        { return m_string.endsWith<matchLength>(prefix, caseSensitive); }

    WTF_EXPORT_STRING_API AtomicString convertToASCIILowercase() const;
    WTF_EXPORT_STRING_API AtomicString convertToASCIIUppercase() const;

    int toInt(bool* ok = 0) const { return m_string.toInt(ok); }
    double toDouble(bool* ok = 0) const { return m_string.toDouble(ok); }
    float toFloat(bool* ok = 0) const { return m_string.toFloat(ok); }
    bool percentage(int& p) const { return m_string.percentage(p); }

    bool isNull() const { return m_string.isNull(); }
    bool isEmpty() const { return m_string.isEmpty(); }

#if USE(CF)
    AtomicString(CFStringRef);
#endif
#ifdef __OBJC__
    AtomicString(NSString*);
    operator NSString*() const { return m_string; }
#endif

    // AtomicString::fromUTF8 will return a null string if
    // the input data contains invalid UTF-8 sequences.
    static AtomicString fromUTF8(const char*, size_t);
    static AtomicString fromUTF8(const char*);

#ifndef NDEBUG
    void show() const;
#endif

private:
    // The explicit constructors with AtomicString::ConstructFromLiteral must be used for literals.
    AtomicString(ASCIILiteral);

    enum class CaseConvertType { Upper, Lower };
    template<CaseConvertType> AtomicString convertASCIICase() const;

    WTF_EXPORT_STRING_API static AtomicString fromUTF8Internal(const char*, const char*);

    String m_string;
};

static_assert(sizeof(AtomicString) == sizeof(String), "AtomicString and String must be same size!");

inline bool operator==(const AtomicString& a, const AtomicString& b) { return a.impl() == b.impl(); }
bool operator==(const AtomicString&, const LChar*);
inline bool operator==(const AtomicString& a, const char* b) { return WTF::equal(a.impl(), reinterpret_cast<const LChar*>(b)); }
inline bool operator==(const AtomicString& a, const Vector<UChar>& b) { return a.impl() && equal(a.impl(), b.data(), b.size()); }    
inline bool operator==(const AtomicString& a, const String& b) { return equal(a.impl(), b.impl()); }
inline bool operator==(const LChar* a, const AtomicString& b) { return b == a; }
inline bool operator==(const String& a, const AtomicString& b) { return equal(a.impl(), b.impl()); }
inline bool operator==(const Vector<UChar>& a, const AtomicString& b) { return b == a; }

inline bool operator!=(const AtomicString& a, const AtomicString& b) { return a.impl() != b.impl(); }
inline bool operator!=(const AtomicString& a, const LChar* b) { return !(a == b); }
inline bool operator!=(const AtomicString& a, const char* b) { return !(a == b); }
inline bool operator!=(const AtomicString& a, const String& b) { return !equal(a.impl(), b.impl()); }
inline bool operator!=(const AtomicString& a, const Vector<UChar>& b) { return !(a == b); }
inline bool operator!=(const LChar* a, const AtomicString& b) { return !(b == a); }
inline bool operator!=(const String& a, const AtomicString& b) { return !equal(a.impl(), b.impl()); }
inline bool operator!=(const Vector<UChar>& a, const AtomicString& b) { return !(a == b); }

bool equalIgnoringASCIICase(const AtomicString&, const AtomicString&);
bool equalIgnoringASCIICase(const AtomicString&, const String&);
bool equalIgnoringASCIICase(const String&, const AtomicString&);
bool equalIgnoringASCIICase(const AtomicString&, const char*);

template<unsigned length> bool equalLettersIgnoringASCIICase(const AtomicString&, const char (&lowercaseLetters)[length]);

inline AtomicString::AtomicString()
{
}

inline AtomicString::AtomicString(const LChar* s)
    : m_string(AtomicStringImpl::add(s))
{
}

inline AtomicString::AtomicString(const char* s)
    : m_string(AtomicStringImpl::add(s))
{
}

inline AtomicString::AtomicString(const LChar* s, unsigned length)
    : m_string(AtomicStringImpl::add(s, length))
{
}

inline AtomicString::AtomicString(const UChar* s, unsigned length)
    : m_string(AtomicStringImpl::add(s, length))
{
}

inline AtomicString::AtomicString(const UChar* s, unsigned length, unsigned existingHash)
    : m_string(AtomicStringImpl::add(s, length, existingHash))
{
}

inline AtomicString::AtomicString(const UChar* s)
    : m_string(AtomicStringImpl::add(s))
{
}

inline AtomicString::AtomicString(AtomicStringImpl* imp)
    : m_string(imp)
{
}

inline AtomicString::AtomicString(RefPtr<AtomicStringImpl>&& imp)
    : m_string(WTFMove(imp))
{
}

inline AtomicString::AtomicString(StringImpl* imp)
    : m_string(AtomicStringImpl::add(imp))
{
}

inline AtomicString::AtomicString(const String& s)
    : m_string(AtomicStringImpl::add(s.impl()))
{
}

inline AtomicString::AtomicString(StringImpl* baseString, unsigned start, unsigned length)
    : m_string(AtomicStringImpl::add(baseString, start, length))
{
}

inline AtomicString::AtomicString(UniquedStringImpl* uid)
    : m_string(uid)
{
}

#if USE(CF)
inline AtomicString::AtomicString(CFStringRef s)
    :  m_string(AtomicStringImpl::add(s))
{
}
#endif

#ifdef __OBJC__
inline AtomicString::AtomicString(NSString* s)
    : m_string(AtomicStringImpl::add((__bridge CFStringRef)s))
{
}
#endif

// Define external global variables for the commonly used atomic strings.
// These are only usable from the main thread.
#ifndef ATOMICSTRING_HIDE_GLOBALS
extern const WTF_EXPORTDATA AtomicString nullAtom;
extern const WTF_EXPORTDATA AtomicString emptyAtom;
extern const WTF_EXPORTDATA AtomicString starAtom;
extern const WTF_EXPORTDATA AtomicString xmlAtom;
extern const WTF_EXPORTDATA AtomicString xmlnsAtom;

inline AtomicString AtomicString::fromUTF8(const char* characters, size_t length)
{
    if (!characters)
        return nullAtom;
    if (!length)
        return emptyAtom;
    return fromUTF8Internal(characters, characters + length);
}

inline AtomicString AtomicString::fromUTF8(const char* characters)
{
    if (!characters)
        return nullAtom;
    if (!*characters)
        return emptyAtom;
    return fromUTF8Internal(characters, nullptr);
}
#endif

// AtomicStringHash is the default hash for AtomicString
template<typename T> struct DefaultHash;
template<> struct DefaultHash<AtomicString> {
    typedef AtomicStringHash Hash;
};

template<unsigned length> inline bool equalLettersIgnoringASCIICase(const AtomicString& string, const char (&lowercaseLetters)[length])
{
    return equalLettersIgnoringASCIICase(string.string(), lowercaseLetters);
}

inline bool equalIgnoringASCIICase(const AtomicString& a, const AtomicString& b)
{
    return equalIgnoringASCIICase(a.string(), b.string());
}

inline bool equalIgnoringASCIICase(const AtomicString& a, const String& b)
{
    return equalIgnoringASCIICase(a.string(), b);
}

inline bool equalIgnoringASCIICase(const String& a, const AtomicString& b)
{
    return equalIgnoringASCIICase(a, b.string());
}

inline bool equalIgnoringASCIICase(const AtomicString& a, const char* b)
{
    return equalIgnoringASCIICase(a.string(), b);
}

template<> struct IntegerToStringConversionTrait<AtomicString> {
    using ReturnType = AtomicString;
    using AdditionalArgumentType = void;
    static AtomicString flush(LChar* characters, unsigned length, void*) { return { characters, length }; }
};

} // namespace WTF

#ifndef ATOMICSTRING_HIDE_GLOBALS
using WTF::AtomicString;
using WTF::nullAtom;
using WTF::emptyAtom;
using WTF::starAtom;
using WTF::xmlAtom;
using WTF::xmlnsAtom;
#endif

#include <wtf/text/StringConcatenate.h>

#endif // AtomicString_h
