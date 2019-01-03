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
#ifndef WTFString_h
#define WTFString_h

// This file would be called String.h, but that conflicts with <string.h>
// on systems without case-sensitive file systems.

#include <wtf/text/ASCIIFastPath.h>
#include <wtf/text/IntegerToStringConversion.h>
#include <wtf/text/StringImpl.h>

#ifdef __OBJC__
#include <objc/objc.h>
#endif

namespace WTF {

class ASCIILiteral;

// Declarations of string operations

WTF_EXPORT_STRING_API int charactersToIntStrict(const LChar*, size_t, bool* ok = nullptr, int base = 10);
WTF_EXPORT_STRING_API int charactersToIntStrict(const UChar*, size_t, bool* ok = nullptr, int base = 10);
WTF_EXPORT_STRING_API unsigned charactersToUIntStrict(const LChar*, size_t, bool* ok = nullptr, int base = 10);
WTF_EXPORT_STRING_API unsigned charactersToUIntStrict(const UChar*, size_t, bool* ok = nullptr, int base = 10);
int64_t charactersToInt64Strict(const LChar*, size_t, bool* ok = nullptr, int base = 10);
int64_t charactersToInt64Strict(const UChar*, size_t, bool* ok = nullptr, int base = 10);
uint64_t charactersToUInt64Strict(const LChar*, size_t, bool* ok = nullptr, int base = 10);
uint64_t charactersToUInt64Strict(const UChar*, size_t, bool* ok = nullptr, int base = 10);
intptr_t charactersToIntPtrStrict(const LChar*, size_t, bool* ok = nullptr, int base = 10);
intptr_t charactersToIntPtrStrict(const UChar*, size_t, bool* ok = nullptr, int base = 10);

WTF_EXPORT_STRING_API int charactersToInt(const LChar*, size_t, bool* ok = nullptr); // ignores trailing garbage
WTF_EXPORT_STRING_API int charactersToInt(const UChar*, size_t, bool* ok = nullptr); // ignores trailing garbage
unsigned charactersToUInt(const LChar*, size_t, bool* ok = nullptr); // ignores trailing garbage
unsigned charactersToUInt(const UChar*, size_t, bool* ok = nullptr); // ignores trailing garbage
int64_t charactersToInt64(const LChar*, size_t, bool* ok = nullptr); // ignores trailing garbage
int64_t charactersToInt64(const UChar*, size_t, bool* ok = nullptr); // ignores trailing garbage
uint64_t charactersToUInt64(const LChar*, size_t, bool* ok = nullptr); // ignores trailing garbage
WTF_EXPORT_STRING_API uint64_t charactersToUInt64(const UChar*, size_t, bool* ok = nullptr); // ignores trailing garbage
intptr_t charactersToIntPtr(const LChar*, size_t, bool* ok = nullptr); // ignores trailing garbage
intptr_t charactersToIntPtr(const UChar*, size_t, bool* ok = nullptr); // ignores trailing garbage

// FIXME: Like the strict functions above, these give false for "ok" when there is trailing garbage.
// Like the non-strict functions above, these return the value when there is trailing garbage.
// It would be better if these were more consistent with the above functions instead.
WTF_EXPORT_STRING_API double charactersToDouble(const LChar*, size_t, bool* ok = nullptr);
WTF_EXPORT_STRING_API double charactersToDouble(const UChar*, size_t, bool* ok = nullptr);
WTF_EXPORT_STRING_API float charactersToFloat(const LChar*, size_t, bool* ok = nullptr);
WTF_EXPORT_STRING_API float charactersToFloat(const UChar*, size_t, bool* ok = nullptr);
WTF_EXPORT_STRING_API float charactersToFloat(const LChar*, size_t, size_t& parsedLength);
WTF_EXPORT_STRING_API float charactersToFloat(const UChar*, size_t, size_t& parsedLength);

template<bool isSpecialCharacter(UChar), typename CharacterType> bool isAllSpecialCharacters(const CharacterType*, size_t);

enum TrailingZerosTruncatingPolicy { KeepTrailingZeros, TruncateTrailingZeros };

class String {
public:
    // Construct a null string, distinguishable from an empty string.
    String() { }

    // Construct a string with UTF-16 data.
    WTF_EXPORT_STRING_API String(const UChar* characters, unsigned length);

    // Construct a string by copying the contents of a vector.  To avoid
    // copying, consider using String::adopt instead.
    // This method will never create a null string. Vectors with size() == 0
    // will return the empty string.
    // NOTE: This is different from String(vector.data(), vector.size())
    // which will sometimes return a null string when vector.data() is null
    // which can only occur for vectors without inline capacity.
    // See: https://bugs.webkit.org/show_bug.cgi?id=109792
    template<size_t inlineCapacity, typename OverflowHandler>
    explicit String(const Vector<UChar, inlineCapacity, OverflowHandler>&);

    // Construct a string with UTF-16 data, from a null-terminated source.
    WTF_EXPORT_STRING_API String(const UChar*);

    // Construct a string with latin1 data.
    WTF_EXPORT_STRING_API String(const LChar* characters, unsigned length);
    WTF_EXPORT_STRING_API String(const char* characters, unsigned length);

    // Construct a string with latin1 data, from a null-terminated source.
    WTF_EXPORT_STRING_API String(const LChar* characters);
    WTF_EXPORT_STRING_API String(const char* characters);

    // Construct a string referencing an existing StringImpl.
    String(StringImpl&);
    String(StringImpl*);
    String(Ref<StringImpl>&&);
    String(RefPtr<StringImpl>&&);

    String(Ref<AtomicStringImpl>&&);
    String(RefPtr<AtomicStringImpl>&&);

    // Construct a string from a constant string literal.
    WTF_EXPORT_STRING_API String(ASCIILiteral characters);

    // Construct a string from a constant string literal.
    // This constructor is the "big" version, as it put the length in the function call and generate bigger code.
    enum ConstructFromLiteralTag { ConstructFromLiteral };
    template<unsigned charactersCount>
    String(const char (&characters)[charactersCount], ConstructFromLiteralTag) : m_impl(StringImpl::createFromLiteral<charactersCount>(characters)) { }

    // We have to declare the copy constructor and copy assignment operator as well, otherwise
    // they'll be implicitly deleted by adding the move constructor and move assignment operator.
    String(const String& other)
        : m_impl(other.m_impl)
    { }
    String(String&& other)
        : m_impl(WTFMove(other.m_impl))
    { }
    String& operator=(const String& other) { m_impl = other.m_impl; return *this; }
    String& operator=(String&& other) { m_impl = WTFMove(other.m_impl); return *this; }

    // Inline the destructor.
    ALWAYS_INLINE ~String() { }

    void swap(String& o) { m_impl.swap(o.m_impl); }

    static String adopt(StringBuffer<LChar>&& buffer) { return StringImpl::adopt(WTFMove(buffer)); }
    static String adopt(StringBuffer<UChar>&& buffer) { return StringImpl::adopt(WTFMove(buffer)); }
    template<typename CharacterType, size_t inlineCapacity, typename OverflowHandler>
    static String adopt(Vector<CharacterType, inlineCapacity, OverflowHandler>&& vector) { return StringImpl::adopt(WTFMove(vector)); }

    bool isNull() const { return !m_impl; }
    bool isEmpty() const { return !m_impl || !m_impl->length(); }

    StringImpl* impl() const { return m_impl.get(); }
    RefPtr<StringImpl> releaseImpl() { return WTFMove(m_impl); }

    unsigned length() const
    {
        if (!m_impl)
            return 0;
        return m_impl->length();
    }

    const LChar* characters8() const
    {
        if (!m_impl)
            return 0;
        ASSERT(m_impl->is8Bit());
        return m_impl->characters8();
    }

    const UChar* characters16() const
    {
        if (!m_impl)
            return 0;
        ASSERT(!m_impl->is8Bit());
        return m_impl->characters16();
    }

    // Return characters8() or characters16() depending on CharacterType.
    template <typename CharacterType>
    inline const CharacterType* characters() const;

    bool is8Bit() const { return m_impl->is8Bit(); }

    unsigned sizeInBytes() const
    {
        if (!m_impl)
            return 0;
        return m_impl->length() * (is8Bit() ? sizeof(LChar) : sizeof(UChar));
    }

    WTF_EXPORT_STRING_API CString ascii() const;
    WTF_EXPORT_STRING_API CString latin1() const;

    WTF_EXPORT_STRING_API CString utf8(ConversionMode) const;
    WTF_EXPORT_STRING_API CString utf8() const;

    UChar at(unsigned index) const
    {
        if (!m_impl || index >= m_impl->length())
            return 0;
        return (*m_impl)[index];
    }
    UChar operator[](unsigned index) const { return at(index); }

    WTF_EXPORT_STRING_API static String number(int);
    WTF_EXPORT_STRING_API static String number(unsigned int);
    WTF_EXPORT_STRING_API static String number(long);
    WTF_EXPORT_STRING_API static String number(unsigned long);
    WTF_EXPORT_STRING_API static String number(long long);
    WTF_EXPORT_STRING_API static String number(unsigned long long);

    WTF_EXPORT_STRING_API static String number(double, unsigned precision = 6, TrailingZerosTruncatingPolicy = TruncateTrailingZeros);

    // Number to String conversion following the ECMAScript definition.
    WTF_EXPORT_STRING_API static String numberToStringECMAScript(double);
    WTF_EXPORT_STRING_API static String numberToStringFixedWidth(double, unsigned decimalPlaces);

    // Find a single character or string, also with match function & latin1 forms.
    size_t find(UChar c, unsigned start = 0) const
        { return m_impl ? m_impl->find(c, start) : notFound; }

    size_t find(const String& str) const
        { return m_impl ? m_impl->find(str.impl()) : notFound; }
    size_t find(const String& str, unsigned start) const
        { return m_impl ? m_impl->find(str.impl(), start) : notFound; }
    size_t findIgnoringASCIICase(const String& str) const
        { return m_impl ? m_impl->findIgnoringASCIICase(str.impl()) : notFound; }
    size_t findIgnoringASCIICase(const String& str, unsigned startOffset) const
        { return m_impl ? m_impl->findIgnoringASCIICase(str.impl(), startOffset) : notFound; }

    size_t find(CharacterMatchFunctionPtr matchFunction, unsigned start = 0) const
        { return m_impl ? m_impl->find(matchFunction, start) : notFound; }
    size_t find(const LChar* str, unsigned start = 0) const
        { return m_impl ? m_impl->find(str, start) : notFound; }

    // Find the last instance of a single character or string.
    size_t reverseFind(UChar c, unsigned start = UINT_MAX) const
        { return m_impl ? m_impl->reverseFind(c, start) : notFound; }
    size_t reverseFind(const String& str, unsigned start = UINT_MAX) const
        { return m_impl ? m_impl->reverseFind(str.impl(), start) : notFound; }

    // Case insensitive string matching.
    size_t findIgnoringCase(const LChar* str, unsigned start = 0) const
        { return m_impl ? m_impl->findIgnoringCase(str, start) : notFound; }
    size_t findIgnoringCase(const String& str, unsigned start = 0) const
        { return m_impl ? m_impl->findIgnoringCase(str.impl(), start) : notFound; }
    size_t reverseFindIgnoringCase(const String& str, unsigned start = UINT_MAX) const
        { return m_impl ? m_impl->reverseFindIgnoringCase(str.impl(), start) : notFound; }

    // Wrappers for find & reverseFind adding dynamic sensitivity check.
    size_t find(const LChar* str, unsigned start, bool caseSensitive) const
        { return caseSensitive ? find(str, start) : findIgnoringCase(str, start); }
    size_t find(const String& str, unsigned start, bool caseSensitive) const
        { return caseSensitive ? find(str, start) : findIgnoringCase(str, start); }
    size_t reverseFind(const String& str, unsigned start, bool caseSensitive) const
        { return caseSensitive ? reverseFind(str, start) : reverseFindIgnoringCase(str, start); }

    WTF_EXPORT_STRING_API Vector<UChar> charactersWithNullTermination() const;
    
    WTF_EXPORT_STRING_API UChar32 characterStartingAt(unsigned) const; // Ditto.
    
    bool contains(UChar c) const { return find(c) != notFound; }
    bool contains(const LChar* str, bool caseSensitive = true, unsigned startOffset = 0) const 
        { return find(str, startOffset, caseSensitive) != notFound; }
    bool contains(const String& str) const
        { return find(str) != notFound; }
    bool contains(const String& str, bool caseSensitive, unsigned startOffset = 0) const
        { return find(str, startOffset, caseSensitive) != notFound; }
    bool containsIgnoringASCIICase(const String& str) const
        { return findIgnoringASCIICase(str) != notFound; }
    bool containsIgnoringASCIICase(const String& str, unsigned startOffset) const
        { return findIgnoringASCIICase(str, startOffset) != notFound; }

    bool startsWith(const String& s) const
        { return m_impl ? m_impl->startsWith(s.impl()) : s.isEmpty(); }
    bool startsWithIgnoringASCIICase(const String& s) const
        { return m_impl ? m_impl->startsWithIgnoringASCIICase(s.impl()) : s.isEmpty(); }
    bool startsWith(const String& s, bool caseSensitive) const
        { return m_impl ? m_impl->startsWith(s.impl(), caseSensitive) : s.isEmpty(); }
    bool startsWith(UChar character) const
        { return m_impl ? m_impl->startsWith(character) : false; }
    template<unsigned matchLength>
    bool startsWith(const char (&prefix)[matchLength], bool caseSensitive = true) const
        { return m_impl ? m_impl->startsWith<matchLength>(prefix, caseSensitive) : !matchLength; }
    bool hasInfixStartingAt(const String& prefix, unsigned startOffset) const
        { return m_impl && prefix.impl() ? m_impl->hasInfixStartingAt(*prefix.impl(), startOffset) : false; }

    bool endsWith(const String& s) const
        { return m_impl ? m_impl->endsWith(s.impl()) : s.isEmpty(); }
    bool endsWithIgnoringASCIICase(const String& s) const
        { return m_impl ? m_impl->endsWithIgnoringASCIICase(s.impl()) : s.isEmpty(); }
    bool endsWith(const String& s, bool caseSensitive) const
        { return m_impl ? m_impl->endsWith(s.impl(), caseSensitive) : s.isEmpty(); }
    bool endsWith(UChar character) const
        { return m_impl ? m_impl->endsWith(character) : false; }
    bool endsWith(char character) const { return endsWith(static_cast<UChar>(character)); }
    template<unsigned matchLength>
    bool endsWith(const char (&prefix)[matchLength], bool caseSensitive = true) const
        { return m_impl ? m_impl->endsWith<matchLength>(prefix, caseSensitive) : !matchLength; }
    bool hasInfixEndingAt(const String& suffix, unsigned endOffset) const
        { return m_impl && suffix.impl() ? m_impl->hasInfixEndingAt(*suffix.impl(), endOffset) : false; }

    WTF_EXPORT_STRING_API void append(const String&);
    WTF_EXPORT_STRING_API void append(LChar);
    void append(char c) { append(static_cast<LChar>(c)); };
    WTF_EXPORT_STRING_API void append(UChar);
    WTF_EXPORT_STRING_API void append(const LChar*, unsigned length);
    WTF_EXPORT_STRING_API void append(const UChar*, unsigned length);
    WTF_EXPORT_STRING_API void insert(const String&, unsigned pos);

    String& replace(UChar a, UChar b) { if (m_impl) m_impl = m_impl->replace(a, b); return *this; }
    String& replace(UChar a, const String& b) { if (m_impl) m_impl = m_impl->replace(a, b.impl()); return *this; }
    String& replace(const String& a, const String& b) { if (m_impl) m_impl = m_impl->replace(a.impl(), b.impl()); return *this; }
    String& replace(unsigned index, unsigned len, const String& b) { if (m_impl) m_impl = m_impl->replace(index, len, b.impl()); return *this; }

    template<unsigned charactersCount>
    ALWAYS_INLINE String& replaceWithLiteral(UChar a, const char (&characters)[charactersCount])
    {
        if (m_impl)
            m_impl = m_impl->replace(a, characters, charactersCount - 1);

        return *this;
    }

    WTF_EXPORT_STRING_API void truncate(unsigned len);
    WTF_EXPORT_STRING_API void remove(unsigned pos, int len = 1);

    WTF_EXPORT_STRING_API String substring(unsigned pos, unsigned len = UINT_MAX) const;
    WTF_EXPORT_STRING_API String substringSharingImpl(unsigned pos, unsigned len = UINT_MAX) const;
    String left(unsigned len) const { return substring(0, len); }
    String right(unsigned len) const { return substring(length() - len, len); }

    WTF_EXPORT_STRING_API String convertToASCIILowercase() const;
    WTF_EXPORT_STRING_API String convertToASCIIUppercase() const;
    WTF_EXPORT_STRING_API String convertToLowercaseWithoutLocale() const;
    WTF_EXPORT_STRING_API String convertToLowercaseWithoutLocaleStartingAtFailingIndex8Bit(unsigned) const;
    WTF_EXPORT_STRING_API String convertToUppercaseWithoutLocale() const;
    WTF_EXPORT_STRING_API String convertToLowercaseWithLocale(const AtomicString& localeIdentifier) const;
    WTF_EXPORT_STRING_API String convertToUppercaseWithLocale(const AtomicString& localeIdentifier) const;

    WTF_EXPORT_STRING_API String stripWhiteSpace() const;
    WTF_EXPORT_STRING_API String stripWhiteSpace(IsWhiteSpaceFunctionPtr) const;
    WTF_EXPORT_STRING_API String simplifyWhiteSpace() const;
    WTF_EXPORT_STRING_API String simplifyWhiteSpace(IsWhiteSpaceFunctionPtr) const;

    WTF_EXPORT_STRING_API String removeCharacters(CharacterMatchFunctionPtr) const;
    template<bool isSpecialCharacter(UChar)> bool isAllSpecialCharacters() const;

    // Returns the string with case folded for case insensitive comparison.
    // Use convertToASCIILowercase instead if ASCII case insensitive comparison is desired.
    WTF_EXPORT_STRING_API String foldCase() const;

    WTF_EXPORT_STRING_API static String format(const char *, ...) WTF_ATTRIBUTE_PRINTF(1, 2);

    // Returns an uninitialized string. The characters needs to be written
    // into the buffer returned in data before the returned string is used.
    // Failure to do this will have unpredictable results.
    static String createUninitialized(unsigned length, UChar*& data) { return StringImpl::createUninitialized(length, data); }
    static String createUninitialized(unsigned length, LChar*& data) { return StringImpl::createUninitialized(length, data); }

    WTF_EXPORT_STRING_API void split(const String& separator, bool allowEmptyEntries, Vector<String>& result) const;
    void split(const String& separator, Vector<String>& result) const
    {
        split(separator, false, result);
    }
    WTF_EXPORT_STRING_API void split(UChar separator, bool allowEmptyEntries, Vector<String>& result) const;
    void split(UChar separator, Vector<String>& result) const
    {
        split(separator, false, result);
    }

    WTF_EXPORT_STRING_API int toIntStrict(bool* ok = nullptr, int base = 10) const;
    WTF_EXPORT_STRING_API unsigned toUIntStrict(bool* ok = nullptr, int base = 10) const;
    WTF_EXPORT_STRING_API int64_t toInt64Strict(bool* ok = nullptr, int base = 10) const;
    WTF_EXPORT_STRING_API uint64_t toUInt64Strict(bool* ok = nullptr, int base = 10) const;
    WTF_EXPORT_STRING_API intptr_t toIntPtrStrict(bool* ok = nullptr, int base = 10) const;

    WTF_EXPORT_STRING_API int toInt(bool* ok = nullptr) const;
    WTF_EXPORT_STRING_API unsigned toUInt(bool* ok = nullptr) const;
    WTF_EXPORT_STRING_API int64_t toInt64(bool* ok = nullptr) const;
    WTF_EXPORT_STRING_API uint64_t toUInt64(bool* ok = nullptr) const;
    WTF_EXPORT_STRING_API intptr_t toIntPtr(bool* ok = nullptr) const;

    // FIXME: Like the strict functions above, these give false for "ok" when there is trailing garbage.
    // Like the non-strict functions above, these return the value when there is trailing garbage.
    // It would be better if these were more consistent with the above functions instead.
    WTF_EXPORT_STRING_API double toDouble(bool* ok = nullptr) const;
    WTF_EXPORT_STRING_API float toFloat(bool* ok = nullptr) const;

    bool percentage(int& percentage) const;

#if COMPILER_SUPPORTS(CXX_REFERENCE_QUALIFIED_FUNCTIONS)
    WTF_EXPORT_STRING_API String isolatedCopy() const &;
    WTF_EXPORT_STRING_API String isolatedCopy() &&;
#else
    WTF_EXPORT_STRING_API String isolatedCopy() const;
#endif

    WTF_EXPORT_STRING_API bool isSafeToSendToAnotherThread() const;

    // Prevent Strings from being implicitly convertable to bool as it will be ambiguous on any platform that
    // allows implicit conversion to another pointer type (e.g., Mac allows implicit conversion to NSString *).
    typedef struct ImplicitConversionFromWTFStringToBoolDisallowedA* (String::*UnspecifiedBoolTypeA);
    typedef struct ImplicitConversionFromWTFStringToBoolDisallowedB* (String::*UnspecifiedBoolTypeB);
    operator UnspecifiedBoolTypeA() const;
    operator UnspecifiedBoolTypeB() const;

#if USE(CF)
    WTF_EXPORT_STRING_API String(CFStringRef);
    WTF_EXPORT_STRING_API RetainPtr<CFStringRef> createCFString() const;
#endif

#ifdef __OBJC__
    WTF_EXPORT_STRING_API String(NSString *);
    
    // This conversion converts the null string to an empty NSString rather than to nil.
    // Given Cocoa idioms, this is a more useful default. Clients that need to preserve the
    // null string can check isNull explicitly.
    operator NSString *() const;
#endif

    WTF_EXPORT_STRING_API static String make8BitFrom16BitSource(const UChar*, size_t);
    template<size_t inlineCapacity>
    static String make8BitFrom16BitSource(const Vector<UChar, inlineCapacity>& buffer)
    {
        return make8BitFrom16BitSource(buffer.data(), buffer.size());
    }

    WTF_EXPORT_STRING_API static String make16BitFrom8BitSource(const LChar*, size_t);

    // String::fromUTF8 will return a null string if
    // the input data contains invalid UTF-8 sequences.
    WTF_EXPORT_STRING_API static String fromUTF8(const LChar*, size_t);
    WTF_EXPORT_STRING_API static String fromUTF8(const LChar*);
    static String fromUTF8(const char* s, size_t length) { return fromUTF8(reinterpret_cast<const LChar*>(s), length); };
    static String fromUTF8(const char* s) { return fromUTF8(reinterpret_cast<const LChar*>(s)); };
    WTF_EXPORT_STRING_API static String fromUTF8(const CString&);

    // Tries to convert the passed in string to UTF-8, but will fall back to Latin-1 if the string is not valid UTF-8.
    WTF_EXPORT_STRING_API static String fromUTF8WithLatin1Fallback(const LChar*, size_t);
    static String fromUTF8WithLatin1Fallback(const char* s, size_t length) { return fromUTF8WithLatin1Fallback(reinterpret_cast<const LChar*>(s), length); };
    
    // Determines the writing direction using the Unicode Bidi Algorithm rules P2 and P3.
    UCharDirection defaultWritingDirection(bool* hasStrongDirectionality = nullptr) const
    {
        if (m_impl)
            return m_impl->defaultWritingDirection(hasStrongDirectionality);
        if (hasStrongDirectionality)
            *hasStrongDirectionality = false;
        return U_LEFT_TO_RIGHT;
    }

    bool containsOnlyASCII() const;
    bool containsOnlyLatin1() const;
    bool containsOnlyWhitespace() const { return !m_impl || m_impl->containsOnlyWhitespace(); }

    // Hash table deleted values, which are only constructed and never copied or destroyed.
    String(WTF::HashTableDeletedValueType) : m_impl(WTF::HashTableDeletedValue) { }
    bool isHashTableDeletedValue() const { return m_impl.isHashTableDeletedValue(); }

    unsigned existingHash() const { return isNull() ? 0 : impl()->existingHash(); }

#ifndef NDEBUG
    WTF_EXPORT_STRING_API void show() const;
#endif

    // Workaround for a compiler bug. Use operator[] instead.
    UChar characterAt(unsigned index) const
    {
        if (!m_impl || index >= m_impl->length())
            return 0;
        return (*m_impl)[index];
    }

    // Turns this String empty if the StringImpl is not referenced by anyone else.
    // This is useful for clearing String-based caches.
    void clearImplIfNotShared()
    {
        if (m_impl && m_impl->hasOneRef())
            m_impl = nullptr;
    }

private:
    template <typename CharacterType>
    void removeInternal(const CharacterType*, unsigned, int);

    template <typename CharacterType>
    void appendInternal(CharacterType);

    RefPtr<StringImpl> m_impl;
};

inline bool operator==(const String& a, const String& b) { return equal(a.impl(), b.impl()); }
inline bool operator==(const String& a, const LChar* b) { return equal(a.impl(), b); }
inline bool operator==(const String& a, const char* b) { return equal(a.impl(), reinterpret_cast<const LChar*>(b)); }
inline bool operator==(const LChar* a, const String& b) { return equal(a, b.impl()); }
inline bool operator==(const char* a, const String& b) { return equal(reinterpret_cast<const LChar*>(a), b.impl()); }
template<size_t inlineCapacity> inline bool operator==(const Vector<char, inlineCapacity>& a, const String& b) { return equal(b.impl(), a.data(), a.size()); }
template<size_t inlineCapacity> inline bool operator==(const String& a, const Vector<char, inlineCapacity>& b) { return b == a; }


inline bool operator!=(const String& a, const String& b) { return !equal(a.impl(), b.impl()); }
inline bool operator!=(const String& a, const LChar* b) { return !equal(a.impl(), b); }
inline bool operator!=(const String& a, const char* b) { return !equal(a.impl(), reinterpret_cast<const LChar*>(b)); }
inline bool operator!=(const LChar* a, const String& b) { return !equal(a, b.impl()); }
inline bool operator!=(const char* a, const String& b) { return !equal(reinterpret_cast<const LChar*>(a), b.impl()); }
template<size_t inlineCapacity> inline bool operator!=(const Vector<char, inlineCapacity>& a, const String& b) { return !(a == b); }
template<size_t inlineCapacity> inline bool operator!=(const String& a, const Vector<char, inlineCapacity>& b) { return b != a; }

bool equalIgnoringASCIICase(const String&, const String&);
bool equalIgnoringASCIICase(const String&, const char*);

template<unsigned length> bool equalLettersIgnoringASCIICase(const String&, const char (&lowercaseLetters)[length]);
template<unsigned length> bool startsWithLettersIgnoringASCIICase(const String&, const char (&lowercaseLetters)[length]);

inline bool equalIgnoringNullity(const String& a, const String& b) { return equalIgnoringNullity(a.impl(), b.impl()); }
template<size_t inlineCapacity> inline bool equalIgnoringNullity(const Vector<UChar, inlineCapacity>& a, const String& b) { return equalIgnoringNullity(a, b.impl()); }

inline bool operator!(const String& str) { return str.isNull(); }

inline void swap(String& a, String& b) { a.swap(b); }

#ifdef __OBJC__

// Used in a small number of places where the long standing behavior has been "nil if empty".
NSString * nsStringNilIfEmpty(const String&);

#endif

// Definitions of string operations

inline String::String(StringImpl& impl)
    : m_impl(&impl)
{
}

inline String::String(StringImpl* impl)
    : m_impl(impl)
{
}

inline String::String(Ref<StringImpl>&& impl)
    : m_impl(WTFMove(impl))
{
}

inline String::String(RefPtr<StringImpl>&& impl)
    : m_impl(WTFMove(impl))
{
}

inline String::String(Ref<AtomicStringImpl>&& impl)
    : m_impl(WTFMove(impl))
{
}

inline String::String(RefPtr<AtomicStringImpl>&& impl)
    : m_impl(WTFMove(impl))
{
}

template<size_t inlineCapacity, typename OverflowHandler>
String::String(const Vector<UChar, inlineCapacity, OverflowHandler>& vector)
    : m_impl(vector.size() ? StringImpl::create(vector.data(), vector.size()) : Ref<StringImpl>(*StringImpl::empty()))
{
}

template<>
inline const LChar* String::characters<LChar>() const
{
    ASSERT(is8Bit());
    return characters8();
}

template<>
inline const UChar* String::characters<UChar>() const
{
    ASSERT(!is8Bit());
    return characters16();
}

inline bool String::containsOnlyLatin1() const
{
    if (isEmpty())
        return true;

    if (is8Bit())
        return true;

    const UChar* characters = characters16();
    UChar ored = 0;
    for (size_t i = 0; i < m_impl->length(); ++i)
        ored |= characters[i];
    return !(ored & 0xFF00);
}

#ifdef __OBJC__

inline String::operator NSString *() const
{
    if (!m_impl)
        return @"";
    return *m_impl;
}

inline NSString * nsStringNilIfEmpty(const String& string)
{
    if (string.isEmpty())
        return nil;
    return *string.impl();
}

#endif

inline bool String::containsOnlyASCII() const
{
    if (isEmpty())
        return true;

    if (is8Bit())
        return charactersAreAllASCII(characters8(), m_impl->length());

    return charactersAreAllASCII(characters16(), m_impl->length());
}

WTF_EXPORT_STRING_API int codePointCompare(const String&, const String&);

inline bool codePointCompareLessThan(const String& a, const String& b)
{
    return codePointCompare(a.impl(), b.impl()) < 0;
}

template<typename CharacterType>
inline void appendNumber(Vector<CharacterType>& vector, unsigned char number)
{
    int numberLength = number > 99 ? 3 : (number > 9 ? 2 : 1);
    size_t vectorSize = vector.size();
    vector.grow(vectorSize + numberLength);

    switch (numberLength) {
    case 3:
        vector[vectorSize + 2] = number % 10 + '0';
        number /= 10;
        FALLTHROUGH;

    case 2:
        vector[vectorSize + 1] = number % 10 + '0';
        number /= 10;
        FALLTHROUGH;

    case 1:
        vector[vectorSize] = number % 10 + '0';
    }
}

template<bool isSpecialCharacter(UChar), typename CharacterType>
inline bool isAllSpecialCharacters(const CharacterType* characters, size_t length)
{
    for (size_t i = 0; i < length; ++i) {
        if (!isSpecialCharacter(characters[i]))
            return false;
    }
    return true;
}

template<bool isSpecialCharacter(UChar)>
inline bool String::isAllSpecialCharacters() const
{
    size_t len = length();

    if (!len)
        return true;

    if (is8Bit())
        return WTF::isAllSpecialCharacters<isSpecialCharacter, LChar>(characters8(), len);
    return WTF::isAllSpecialCharacters<isSpecialCharacter, UChar>(characters16(), len);
}

// StringHash is the default hash for String
template<typename T> struct DefaultHash;
template<> struct DefaultHash<String> {
    typedef StringHash Hash;
};

template <> struct VectorTraits<String> : SimpleClassVectorTraits { };

class ASCIILiteral {
public:
    explicit ASCIILiteral(const char* characters) : m_characters(characters) { }
    operator const char*() { return m_characters; }

private:
    const char* m_characters;
};

// Shared global empty string.
WTF_EXPORT_STRING_API const String& emptyString();

template<unsigned length> inline bool equalLettersIgnoringASCIICase(const String& string, const char (&lowercaseLetters)[length])
{
    return equalLettersIgnoringASCIICase(string.impl(), lowercaseLetters);
}

inline bool equalIgnoringASCIICase(const String& a, const String& b)
{
    return equalIgnoringASCIICase(a.impl(), b.impl());
}

inline bool equalIgnoringASCIICase(const String& a, const char* b)
{
    return equalIgnoringASCIICase(a.impl(), b);
}

template<unsigned length> inline bool startsWithLettersIgnoringASCIICase(const String& string, const char (&lowercaseLetters)[length])
{
    return startsWithLettersIgnoringASCIICase(string.impl(), lowercaseLetters);
}

template<> struct IntegerToStringConversionTrait<String> {
    using ReturnType = String;
    using AdditionalArgumentType = void;
    static String flush(LChar* characters, unsigned length, void*) { return { characters, length }; }
};

}

using WTF::CString;
using WTF::KeepTrailingZeros;
using WTF::String;
using WTF::emptyString;
using WTF::appendNumber;
using WTF::charactersAreAllASCII;
using WTF::charactersToIntStrict;
using WTF::charactersToUIntStrict;
using WTF::charactersToInt64Strict;
using WTF::charactersToUInt64Strict;
using WTF::charactersToIntPtrStrict;
using WTF::charactersToInt;
using WTF::charactersToUInt;
using WTF::charactersToInt64;
using WTF::charactersToUInt64;
using WTF::charactersToIntPtr;
using WTF::charactersToDouble;
using WTF::charactersToFloat;
using WTF::equal;
using WTF::find;
using WTF::isAllSpecialCharacters;
using WTF::isSpaceOrNewline;
using WTF::reverseFind;
using WTF::ASCIILiteral;

#include <wtf/text/AtomicString.h>

#endif
