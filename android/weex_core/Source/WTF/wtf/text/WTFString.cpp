/*
 * (C) 1999 Lars Knoll (knoll@kde.org)
 * Copyright (C) 2004, 2005, 2006, 2007, 2008, 2010, 2012 Apple Inc. All rights reserved.
 * Copyright (C) 2007-2009 Torch Mobile, Inc.
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

#include "config.h"
#include "WTFString.h"

#include "IntegerToStringConversion.h"
#include <stdarg.h>
#include <wtf/ASCIICType.h>
#include <wtf/DataLog.h>
#include <wtf/HexNumber.h>
#include <wtf/MathExtras.h>
#include <wtf/NeverDestroyed.h>
#include <wtf/text/CString.h>
#include <wtf/StringExtras.h>
#include <wtf/Vector.h>
#include <wtf/dtoa.h>
#include <wtf/unicode/CharacterNames.h>
#include <wtf/unicode/WTFUTF8.h>

namespace WTF {

using namespace Unicode;

// Construct a string with UTF-16 data.
String::String(const UChar* characters, unsigned length)
{
    if (characters)
        m_impl = StringImpl::create(characters, length);
}

// Construct a string with UTF-16 data, from a null-terminated source.
String::String(const UChar* str)
{
    if (!str)
        return;

    m_impl = StringImpl::create(str, lengthOfNullTerminatedString(str));
}

// Construct a string with latin1 data.
String::String(const LChar* characters, unsigned length)
{
    if (characters)
        m_impl = StringImpl::create(characters, length);
}

String::String(const char* characters, unsigned length)
{
    if (characters)
        m_impl = StringImpl::create(reinterpret_cast<const LChar*>(characters), length);
}

// Construct a string with latin1 data, from a null-terminated source.
String::String(const LChar* characters)
{
    if (characters)
        m_impl = StringImpl::create(characters);
}

String::String(const char* characters)
{
    if (characters)
        m_impl = StringImpl::create(reinterpret_cast<const LChar*>(characters));
}

String::String(ASCIILiteral characters)
    : m_impl(StringImpl::createFromLiteral(characters))
{
}

void String::append(const String& str)
{
    // FIXME: This is extremely inefficient. So much so that we might want to take this out of String's API.

    if (str.isEmpty())
       return;

    if (str.m_impl) {
        if (m_impl) {
            if (m_impl->is8Bit() && str.m_impl->is8Bit()) {
                LChar* data;
                if (str.length() > std::numeric_limits<unsigned>::max() - m_impl->length())
                    CRASH();
                auto newImpl = StringImpl::createUninitialized(m_impl->length() + str.length(), data);
                memcpy(data, m_impl->characters8(), m_impl->length() * sizeof(LChar));
                memcpy(data + m_impl->length(), str.characters8(), str.length() * sizeof(LChar));
                m_impl = WTFMove(newImpl);
                return;
            }
            UChar* data;
            if (str.length() > std::numeric_limits<unsigned>::max() - m_impl->length())
                CRASH();
            auto newImpl = StringImpl::createUninitialized(m_impl->length() + str.length(), data);
            StringView(*m_impl).getCharactersWithUpconvert(data);
            StringView(str).getCharactersWithUpconvert(data + m_impl->length());
            m_impl = WTFMove(newImpl);
        } else
            m_impl = str.m_impl;
    }
}

void String::append(LChar character)
{
    // FIXME: This is extremely inefficient. So much so that we might want to take this out of String's API.

    if (!m_impl) {
        m_impl = StringImpl::create(&character, 1);
        return;
    }
    if (!is8Bit()) {
        append(static_cast<UChar>(character));
        return;
    }
    if (m_impl->length() >= std::numeric_limits<unsigned>::max())
        CRASH();
    LChar* data;
    auto newImpl = StringImpl::createUninitialized(m_impl->length() + 1, data);
    memcpy(data, m_impl->characters8(), m_impl->length());
    data[m_impl->length()] = character;
    m_impl = WTFMove(newImpl);
}

void String::append(UChar character)
{
    // FIXME: This is extremely inefficient. So much so that we might want to take this out of String's API.

    if (!m_impl) {
        m_impl = StringImpl::create(&character, 1);
        return;
    }
    if (character <= 0xFF && is8Bit()) {
        append(static_cast<LChar>(character));
        return;
    }
    if (m_impl->length() >= std::numeric_limits<unsigned>::max())
        CRASH();
    UChar* data;
    auto newImpl = StringImpl::createUninitialized(m_impl->length() + 1, data);
    StringView(*m_impl).getCharactersWithUpconvert(data);
    data[m_impl->length()] = character;
    m_impl = WTFMove(newImpl);
}

int codePointCompare(const String& a, const String& b)
{
    return codePointCompare(a.impl(), b.impl());
}

void String::insert(const String& string, unsigned position)
{
    // FIXME: This is extremely inefficient. So much so that we might want to take this out of String's API.

    unsigned lengthToInsert = string.length();

    if (!lengthToInsert) {
        if (string.isNull())
            return;
        if (isNull())
            m_impl = string.impl();
        return;
    }

    if (position >= length()) {
        append(string);
        return;
    }

    if (lengthToInsert > std::numeric_limits<unsigned>::max() - length())
        CRASH();

    if (is8Bit() && string.is8Bit()) {
        LChar* data;
        auto newString = StringImpl::createUninitialized(length() + lengthToInsert, data);
        StringView(*m_impl).substring(0, position).getCharactersWithUpconvert(data);
        StringView(string).getCharactersWithUpconvert(data + position);
        StringView(*m_impl).substring(position).getCharactersWithUpconvert(data + position + lengthToInsert);
        m_impl = WTFMove(newString);
    } else {
        UChar* data;
        auto newString = StringImpl::createUninitialized(length() + lengthToInsert, data);
        StringView(*m_impl).substring(0, position).getCharactersWithUpconvert(data);
        StringView(string).getCharactersWithUpconvert(data + position);
        StringView(*m_impl).substring(position).getCharactersWithUpconvert(data + position + lengthToInsert);
        m_impl = WTFMove(newString);
    }
}

void String::append(const LChar* charactersToAppend, unsigned lengthToAppend)
{
    // FIXME: This is extremely inefficient. So much so that we might want to take this out of String's API.

    if (!m_impl) {
        if (!charactersToAppend)
            return;
        m_impl = StringImpl::create(charactersToAppend, lengthToAppend);
        return;
    }

    if (!lengthToAppend)
        return;

    ASSERT(charactersToAppend);

    unsigned strLength = m_impl->length();

    if (m_impl->is8Bit()) {
        if (lengthToAppend > std::numeric_limits<unsigned>::max() - strLength)
            CRASH();
        LChar* data;
        auto newImpl = StringImpl::createUninitialized(strLength + lengthToAppend, data);
        StringImpl::copyChars(data, m_impl->characters8(), strLength);
        StringImpl::copyChars(data + strLength, charactersToAppend, lengthToAppend);
        m_impl = WTFMove(newImpl);
        return;
    }

    if (lengthToAppend > std::numeric_limits<unsigned>::max() - strLength)
        CRASH();
    UChar* data;
    auto newImpl = StringImpl::createUninitialized(length() + lengthToAppend, data);
    StringImpl::copyChars(data, m_impl->characters16(), strLength);
    StringImpl::copyChars(data + strLength, charactersToAppend, lengthToAppend);
    m_impl = WTFMove(newImpl);
}

void String::append(const UChar* charactersToAppend, unsigned lengthToAppend)
{
    // FIXME: This is extremely inefficient. So much so that we might want to take this out of String's API.

    if (!m_impl) {
        if (!charactersToAppend)
            return;
        m_impl = StringImpl::create(charactersToAppend, lengthToAppend);
        return;
    }

    if (!lengthToAppend)
        return;

    unsigned strLength = m_impl->length();
    
    ASSERT(charactersToAppend);
    if (lengthToAppend > std::numeric_limits<unsigned>::max() - strLength)
        CRASH();
    UChar* data;
    auto newImpl = StringImpl::createUninitialized(strLength + lengthToAppend, data);
    if (m_impl->is8Bit())
        StringImpl::copyChars(data, characters8(), strLength);
    else
        StringImpl::copyChars(data, characters16(), strLength);
    StringImpl::copyChars(data + strLength, charactersToAppend, lengthToAppend);
    m_impl = WTFMove(newImpl);
}


UChar32 String::characterStartingAt(unsigned i) const
{
    if (!m_impl || i >= m_impl->length())
        return 0;
    return m_impl->characterStartingAt(i);
}

void String::truncate(unsigned position)
{
    if (m_impl)
        m_impl = m_impl->substring(0, position);
}

template <typename CharacterType>
inline void String::removeInternal(const CharacterType* characters, unsigned position, int lengthToRemove)
{
    CharacterType* data;
    auto newImpl = StringImpl::createUninitialized(length() - lengthToRemove, data);
    memcpy(data, characters, position * sizeof(CharacterType));
    memcpy(data + position, characters + position + lengthToRemove,
        (length() - lengthToRemove - position) * sizeof(CharacterType));

    m_impl = WTFMove(newImpl);
}

void String::remove(unsigned position, int lengthToRemove)
{
    if (lengthToRemove <= 0)
        return;
    if (position >= length())
        return;
    if (static_cast<unsigned>(lengthToRemove) > length() - position)
        lengthToRemove = length() - position;

    if (is8Bit()) {
        removeInternal(characters8(), position, lengthToRemove);

        return;
    }

    removeInternal(characters16(), position, lengthToRemove);
}

String String::substring(unsigned pos, unsigned len) const
{
    if (!m_impl) 
        return String();
    return m_impl->substring(pos, len);
}

String String::substringSharingImpl(unsigned offset, unsigned length) const
{
    // FIXME: We used to check against a limit of Heap::minExtraCost / sizeof(UChar).

    unsigned stringLength = this->length();
    offset = std::min(offset, stringLength);
    length = std::min(length, stringLength - offset);

    if (!offset && length == stringLength)
        return *this;
    return String(StringImpl::createSubstringSharingImpl(*m_impl, offset, length));
}

String String::convertToASCIILowercase() const
{
    // FIXME: Should this function, and the many others like it, be inlined?
    if (!m_impl)
        return String();
    return m_impl->convertToASCIILowercase();
}

String String::convertToASCIIUppercase() const
{
    // FIXME: Should this function, and the many others like it, be inlined?
    if (!m_impl)
        return String();
    return m_impl->convertToASCIIUppercase();
}

String String::convertToLowercaseWithoutLocale() const
{
    if (!m_impl)
        return String();
    return m_impl->convertToLowercaseWithoutLocale();
}

String String::convertToLowercaseWithoutLocaleStartingAtFailingIndex8Bit(unsigned failingIndex) const
{
    if (!m_impl)
        return String();
    return m_impl->convertToLowercaseWithoutLocaleStartingAtFailingIndex8Bit(failingIndex);
}

String String::convertToUppercaseWithoutLocale() const
{
    if (!m_impl)
        return String();
    return m_impl->convertToUppercaseWithoutLocale();
}

String String::convertToLowercaseWithLocale(const AtomicString& localeIdentifier) const
{
    if (!m_impl)
        return String();
    return m_impl->convertToLowercaseWithLocale(localeIdentifier);
}

String String::convertToUppercaseWithLocale(const AtomicString& localeIdentifier) const
{
    if (!m_impl)
        return String();
    return m_impl->convertToUppercaseWithLocale(localeIdentifier);
}

String String::stripWhiteSpace() const
{
    if (!m_impl)
        return String();
    return m_impl->stripWhiteSpace();
}

String String::stripWhiteSpace(IsWhiteSpaceFunctionPtr isWhiteSpace) const
{
    if (!m_impl)
        return String();
    return m_impl->stripWhiteSpace(isWhiteSpace);
}

String String::simplifyWhiteSpace() const
{
    if (!m_impl)
        return String();
    return m_impl->simplifyWhiteSpace();
}

String String::simplifyWhiteSpace(IsWhiteSpaceFunctionPtr isWhiteSpace) const
{
    if (!m_impl)
        return String();
    return m_impl->simplifyWhiteSpace(isWhiteSpace);
}

String String::removeCharacters(CharacterMatchFunctionPtr findMatch) const
{
    if (!m_impl)
        return String();
    return m_impl->removeCharacters(findMatch);
}

String String::foldCase() const
{
    if (!m_impl)
        return String();
    return m_impl->foldCase();
}

bool String::percentage(int& result) const
{
    if (!m_impl || !m_impl->length())
        return false;

    if ((*m_impl)[m_impl->length() - 1] != '%')
       return false;

    if (m_impl->is8Bit())
        result = charactersToIntStrict(m_impl->characters8(), m_impl->length() - 1);
    else
        result = charactersToIntStrict(m_impl->characters16(), m_impl->length() - 1);
    return true;
}

Vector<UChar> String::charactersWithNullTermination() const
{
    Vector<UChar> result;

    if (m_impl) {
        result.reserveInitialCapacity(length() + 1);

        if (is8Bit()) {
            const LChar* characters8 = m_impl->characters8();
            for (size_t i = 0; i < length(); ++i)
                result.uncheckedAppend(characters8[i]);
        } else {
            const UChar* characters16 = m_impl->characters16();
            result.append(characters16, m_impl->length());
        }

        result.append(0);
    }

    return result;
}

String String::format(const char *format, ...)
{
    va_list args;
    va_start(args, format);

#if USE(CF) && !OS(WINDOWS)
    if (strstr(format, "%@")) {
        RetainPtr<CFStringRef> cfFormat = adoptCF(CFStringCreateWithCString(kCFAllocatorDefault, format, kCFStringEncodingUTF8));

#if COMPILER(CLANG)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wformat-nonliteral"
#endif
        RetainPtr<CFStringRef> result = adoptCF(CFStringCreateWithFormatAndArguments(kCFAllocatorDefault, nullptr, cfFormat.get(), args));
#if COMPILER(CLANG)
#pragma clang diagnostic pop
#endif

        va_end(args);
        return result.get();
    }
#endif // USE(CF) && !OS(WINDOWS)

    // Do the format once to get the length.
#if COMPILER(MSVC)
    int result = _vscprintf(format, args);
#else
    char ch;
    int result = vsnprintf(&ch, 1, format, args);
#endif
    va_end(args);

    if (result == 0)
        return String("");
    if (result < 0)
        return String();

    Vector<char, 256> buffer;
    unsigned len = result;
    buffer.grow(len + 1);
    
    va_start(args, format);
    // Now do the formatting again, guaranteed to fit.
    vsnprintf(buffer.data(), buffer.size(), format, args);

    va_end(args);
    
    return StringImpl::create(reinterpret_cast<const LChar*>(buffer.data()), len);
}

String String::number(int number)
{
    return numberToStringSigned<String>(number);
}

String String::number(unsigned int number)
{
    return numberToStringUnsigned<String>(number);
}

String String::number(long number)
{
    return numberToStringSigned<String>(number);
}

String String::number(unsigned long number)
{
    return numberToStringUnsigned<String>(number);
}

String String::number(long long number)
{
    return numberToStringSigned<String>(number);
}

String String::number(unsigned long long number)
{
    return numberToStringUnsigned<String>(number);
}

String String::number(double number, unsigned precision, TrailingZerosTruncatingPolicy trailingZerosTruncatingPolicy)
{
    NumberToStringBuffer buffer;
    return String(numberToFixedPrecisionString(number, precision, buffer, trailingZerosTruncatingPolicy == TruncateTrailingZeros));
}

String String::numberToStringECMAScript(double number)
{
    NumberToStringBuffer buffer;
    return String(numberToString(number, buffer));
}

String String::numberToStringFixedWidth(double number, unsigned decimalPlaces)
{
    NumberToStringBuffer buffer;
    return String(numberToFixedWidthString(number, decimalPlaces, buffer));
}

int String::toIntStrict(bool* ok, int base) const
{
    if (!m_impl) {
        if (ok)
            *ok = false;
        return 0;
    }
    return m_impl->toIntStrict(ok, base);
}

unsigned String::toUIntStrict(bool* ok, int base) const
{
    if (!m_impl) {
        if (ok)
            *ok = false;
        return 0;
    }
    return m_impl->toUIntStrict(ok, base);
}

int64_t String::toInt64Strict(bool* ok, int base) const
{
    if (!m_impl) {
        if (ok)
            *ok = false;
        return 0;
    }
    return m_impl->toInt64Strict(ok, base);
}

uint64_t String::toUInt64Strict(bool* ok, int base) const
{
    if (!m_impl) {
        if (ok)
            *ok = false;
        return 0;
    }
    return m_impl->toUInt64Strict(ok, base);
}

intptr_t String::toIntPtrStrict(bool* ok, int base) const
{
    if (!m_impl) {
        if (ok)
            *ok = false;
        return 0;
    }
    return m_impl->toIntPtrStrict(ok, base);
}

int String::toInt(bool* ok) const
{
    if (!m_impl) {
        if (ok)
            *ok = false;
        return 0;
    }
    return m_impl->toInt(ok);
}

unsigned String::toUInt(bool* ok) const
{
    if (!m_impl) {
        if (ok)
            *ok = false;
        return 0;
    }
    return m_impl->toUInt(ok);
}

int64_t String::toInt64(bool* ok) const
{
    if (!m_impl) {
        if (ok)
            *ok = false;
        return 0;
    }
    return m_impl->toInt64(ok);
}

uint64_t String::toUInt64(bool* ok) const
{
    if (!m_impl) {
        if (ok)
            *ok = false;
        return 0;
    }
    return m_impl->toUInt64(ok);
}

intptr_t String::toIntPtr(bool* ok) const
{
    if (!m_impl) {
        if (ok)
            *ok = false;
        return 0;
    }
    return m_impl->toIntPtr(ok);
}

double String::toDouble(bool* ok) const
{
    if (!m_impl) {
        if (ok)
            *ok = false;
        return 0.0;
    }
    return m_impl->toDouble(ok);
}

float String::toFloat(bool* ok) const
{
    if (!m_impl) {
        if (ok)
            *ok = false;
        return 0.0f;
    }
    return m_impl->toFloat(ok);
}

#if COMPILER_SUPPORTS(CXX_REFERENCE_QUALIFIED_FUNCTIONS)
String String::isolatedCopy() const &
{
    if (!m_impl)
        return String();
    return m_impl->isolatedCopy();
}

String String::isolatedCopy() &&
{
    if (isSafeToSendToAnotherThread()) {
        // Since we know that our string is a temporary that will be destroyed
        // we can just steal the m_impl from it, thus avoiding a copy.
        return String(WTFMove(*this));
    }

    if (!m_impl)
        return String();

    return m_impl->isolatedCopy();
}
#else
String String::isolatedCopy() const
{
    if (!m_impl)
        return String();
    return m_impl->isolatedCopy();
}
#endif

bool String::isSafeToSendToAnotherThread() const
{
    if (!impl())
        return true;
    if (isEmpty())
        return true;
    // AtomicStrings are not safe to send between threads as ~StringImpl()
    // will try to remove them from the wrong AtomicStringTable.
    if (impl()->isAtomic())
        return false;
    if (impl()->hasOneRef())
        return true;
    return false;
}

void String::split(const String& separator, bool allowEmptyEntries, Vector<String>& result) const
{
    result.clear();

    unsigned startPos = 0;
    size_t endPos;
    while ((endPos = find(separator, startPos)) != notFound) {
        if (allowEmptyEntries || startPos != endPos)
            result.append(substring(startPos, endPos - startPos));
        startPos = endPos + separator.length();
    }
    if (allowEmptyEntries || startPos != length())
        result.append(substring(startPos));
}

void String::split(UChar separator, bool allowEmptyEntries, Vector<String>& result) const
{
    result.clear();

    unsigned startPos = 0;
    size_t endPos;
    while ((endPos = find(separator, startPos)) != notFound) {
        if (allowEmptyEntries || startPos != endPos)
            result.append(substring(startPos, endPos - startPos));
        startPos = endPos + 1;
    }
    if (allowEmptyEntries || startPos != length())
        result.append(substring(startPos));
}

CString String::ascii() const
{
    // Printable ASCII characters 32..127 and the null character are
    // preserved, characters outside of this range are converted to '?'.

    unsigned length = this->length();
    if (!length) { 
        char* characterBuffer;
        return CString::newUninitialized(length, characterBuffer);
    }

    if (this->is8Bit()) {
        const LChar* characters = this->characters8();

        char* characterBuffer;
        CString result = CString::newUninitialized(length, characterBuffer);

        for (unsigned i = 0; i < length; ++i) {
            LChar ch = characters[i];
            characterBuffer[i] = ch && (ch < 0x20 || ch > 0x7f) ? '?' : ch;
        }

        return result;        
    }

    const UChar* characters = this->characters16();

    char* characterBuffer;
    CString result = CString::newUninitialized(length, characterBuffer);

    for (unsigned i = 0; i < length; ++i) {
        UChar ch = characters[i];
        characterBuffer[i] = ch && (ch < 0x20 || ch > 0x7f) ? '?' : ch;
    }

    return result;
}

CString String::latin1() const
{
    // Basic Latin1 (ISO) encoding - Unicode characters 0..255 are
    // preserved, characters outside of this range are converted to '?'.

    unsigned length = this->length();

    if (!length)
        return CString("", 0);

    if (is8Bit())
        return CString(reinterpret_cast<const char*>(this->characters8()), length);

    const UChar* characters = this->characters16();

    char* characterBuffer;
    CString result = CString::newUninitialized(length, characterBuffer);

    for (unsigned i = 0; i < length; ++i) {
        UChar ch = characters[i];
        characterBuffer[i] = ch > 0xff ? '?' : ch;
    }

    return result;
}

CString String::utf8(ConversionMode mode) const
{
    if (!m_impl)
        return CString("", 0);
    
    return m_impl->utf8(mode);
}

CString String::utf8() const
{
    return utf8(LenientConversion);
}

String String::make8BitFrom16BitSource(const UChar* source, size_t length)
{
    if (!length)
        return String();

    LChar* destination;
    String result = String::createUninitialized(length, destination);

    copyLCharsFromUCharSource(destination, source, length);

    return result;
}

String String::make16BitFrom8BitSource(const LChar* source, size_t length)
{
    if (!length)
        return String();
    
    UChar* destination;
    String result = String::createUninitialized(length, destination);
    
    StringImpl::copyChars(destination, source, length);
    
    return result;
}

String String::fromUTF8(const LChar* stringStart, size_t length)
{
    if (length > std::numeric_limits<unsigned>::max())
        CRASH();

    if (!stringStart)
        return String();

    if (!length)
        return emptyString();

    if (charactersAreAllASCII(stringStart, length))
        return StringImpl::create(stringStart, length);

    Vector<UChar, 1024> buffer(length);
    UChar* bufferStart = buffer.data();
 
    UChar* bufferCurrent = bufferStart;
    const char* stringCurrent = reinterpret_cast<const char*>(stringStart);
    if (convertUTF8ToUTF16(&stringCurrent, reinterpret_cast<const char *>(stringStart + length), &bufferCurrent, bufferCurrent + buffer.size()) != conversionOK)
        return String();

    unsigned utf16Length = bufferCurrent - bufferStart;
    ASSERT_WITH_SECURITY_IMPLICATION(utf16Length < length);
    return StringImpl::create(bufferStart, utf16Length);
}

String String::fromUTF8(const LChar* string)
{
    if (!string)
        return String();
    return fromUTF8(string, strlen(reinterpret_cast<const char*>(string)));
}

String String::fromUTF8(const CString& s)
{
    return fromUTF8(s.data());
}

String String::fromUTF8WithLatin1Fallback(const LChar* string, size_t size)
{
    String utf8 = fromUTF8(string, size);
    if (!utf8)
        return String(string, size);
    return utf8;
}

// String Operations

static bool isCharacterAllowedInBase(UChar c, int base)
{
    if (c > 0x7F)
        return false;
    if (isASCIIDigit(c))
        return c - '0' < base;
    if (isASCIIAlpha(c)) {
        if (base > 36)
            base = 36;
        return (c >= 'a' && c < 'a' + base - 10)
            || (c >= 'A' && c < 'A' + base - 10);
    }
    return false;
}

template <typename IntegralType, typename CharType>
static inline IntegralType toIntegralType(const CharType* data, size_t length, bool* ok, int base)
{
    static const IntegralType integralMax = std::numeric_limits<IntegralType>::max();
    static const bool isSigned = std::numeric_limits<IntegralType>::is_signed;
    const IntegralType maxMultiplier = integralMax / base;

    IntegralType value = 0;
    bool isOk = false;
    bool isNegative = false;

    if (!data)
        goto bye;

    // skip leading whitespace
    while (length && isSpaceOrNewline(*data)) {
        --length;
        ++data;
    }

    if (isSigned && length && *data == '-') {
        --length;
        ++data;
        isNegative = true;
    } else if (length && *data == '+') {
        --length;
        ++data;
    }

    if (!length || !isCharacterAllowedInBase(*data, base))
        goto bye;

    while (length && isCharacterAllowedInBase(*data, base)) {
        --length;
        IntegralType digitValue;
        CharType c = *data;
        if (isASCIIDigit(c))
            digitValue = c - '0';
        else if (c >= 'a')
            digitValue = c - 'a' + 10;
        else
            digitValue = c - 'A' + 10;

        if (value > maxMultiplier || (value == maxMultiplier && digitValue > (integralMax % base) + isNegative))
            goto bye;

        value = base * value + digitValue;
        ++data;
    }

#if COMPILER(MSVC)
#pragma warning(push, 0)
#pragma warning(disable:4146)
#endif

    if (isNegative)
        value = -value;

#if COMPILER(MSVC)
#pragma warning(pop)
#endif

    // skip trailing space
    while (length && isSpaceOrNewline(*data)) {
        --length;
        ++data;
    }

    if (!length)
        isOk = true;
bye:
    if (ok)
        *ok = isOk;
    return isOk ? value : 0;
}

template <typename CharType>
static unsigned lengthOfCharactersAsInteger(const CharType* data, size_t length)
{
    size_t i = 0;

    // Allow leading spaces.
    for (; i != length; ++i) {
        if (!isSpaceOrNewline(data[i]))
            break;
    }
    
    // Allow sign.
    if (i != length && (data[i] == '+' || data[i] == '-'))
        ++i;
    
    // Allow digits.
    for (; i != length; ++i) {
        if (!isASCIIDigit(data[i]))
            break;
    }

    return i;
}

int charactersToIntStrict(const LChar* data, size_t length, bool* ok, int base)
{
    return toIntegralType<int, LChar>(data, length, ok, base);
}

int charactersToIntStrict(const UChar* data, size_t length, bool* ok, int base)
{
    return toIntegralType<int, UChar>(data, length, ok, base);
}

unsigned charactersToUIntStrict(const LChar* data, size_t length, bool* ok, int base)
{
    return toIntegralType<unsigned, LChar>(data, length, ok, base);
}

unsigned charactersToUIntStrict(const UChar* data, size_t length, bool* ok, int base)
{
    return toIntegralType<unsigned, UChar>(data, length, ok, base);
}

int64_t charactersToInt64Strict(const LChar* data, size_t length, bool* ok, int base)
{
    return toIntegralType<int64_t, LChar>(data, length, ok, base);
}

int64_t charactersToInt64Strict(const UChar* data, size_t length, bool* ok, int base)
{
    return toIntegralType<int64_t, UChar>(data, length, ok, base);
}

uint64_t charactersToUInt64Strict(const LChar* data, size_t length, bool* ok, int base)
{
    return toIntegralType<uint64_t, LChar>(data, length, ok, base);
}

uint64_t charactersToUInt64Strict(const UChar* data, size_t length, bool* ok, int base)
{
    return toIntegralType<uint64_t, UChar>(data, length, ok, base);
}

intptr_t charactersToIntPtrStrict(const LChar* data, size_t length, bool* ok, int base)
{
    return toIntegralType<intptr_t, LChar>(data, length, ok, base);
}

intptr_t charactersToIntPtrStrict(const UChar* data, size_t length, bool* ok, int base)
{
    return toIntegralType<intptr_t, UChar>(data, length, ok, base);
}

int charactersToInt(const LChar* data, size_t length, bool* ok)
{
    return toIntegralType<int, LChar>(data, lengthOfCharactersAsInteger<LChar>(data, length), ok, 10);
}

int charactersToInt(const UChar* data, size_t length, bool* ok)
{
    return toIntegralType<int, UChar>(data, lengthOfCharactersAsInteger(data, length), ok, 10);
}

unsigned charactersToUInt(const LChar* data, size_t length, bool* ok)
{
    return toIntegralType<unsigned, LChar>(data, lengthOfCharactersAsInteger<LChar>(data, length), ok, 10);
}

unsigned charactersToUInt(const UChar* data, size_t length, bool* ok)
{
    return toIntegralType<unsigned, UChar>(data, lengthOfCharactersAsInteger<UChar>(data, length), ok, 10);
}

int64_t charactersToInt64(const LChar* data, size_t length, bool* ok)
{
    return toIntegralType<int64_t, LChar>(data, lengthOfCharactersAsInteger<LChar>(data, length), ok, 10);
}

int64_t charactersToInt64(const UChar* data, size_t length, bool* ok)
{
    return toIntegralType<int64_t, UChar>(data, lengthOfCharactersAsInteger<UChar>(data, length), ok, 10);
}

uint64_t charactersToUInt64(const LChar* data, size_t length, bool* ok)
{
    return toIntegralType<uint64_t, LChar>(data, lengthOfCharactersAsInteger<LChar>(data, length), ok, 10);
}

uint64_t charactersToUInt64(const UChar* data, size_t length, bool* ok)
{
    return toIntegralType<uint64_t, UChar>(data, lengthOfCharactersAsInteger<UChar>(data, length), ok, 10);
}

intptr_t charactersToIntPtr(const LChar* data, size_t length, bool* ok)
{
    return toIntegralType<intptr_t, LChar>(data, lengthOfCharactersAsInteger<LChar>(data, length), ok, 10);
}

intptr_t charactersToIntPtr(const UChar* data, size_t length, bool* ok)
{
    return toIntegralType<intptr_t, UChar>(data, lengthOfCharactersAsInteger<UChar>(data, length), ok, 10);
}

enum TrailingJunkPolicy { DisallowTrailingJunk, AllowTrailingJunk };

template <typename CharType, TrailingJunkPolicy policy>
static inline double toDoubleType(const CharType* data, size_t length, bool* ok, size_t& parsedLength)
{
    size_t leadingSpacesLength = 0;
    while (leadingSpacesLength < length && isASCIISpace(data[leadingSpacesLength]))
        ++leadingSpacesLength;

    double number = parseDouble(data + leadingSpacesLength, length - leadingSpacesLength, parsedLength);
    if (!parsedLength) {
        if (ok)
            *ok = false;
        return 0.0;
    }

    parsedLength += leadingSpacesLength;
    if (ok)
        *ok = policy == AllowTrailingJunk || parsedLength == length;
    return number;
}

double charactersToDouble(const LChar* data, size_t length, bool* ok)
{
    size_t parsedLength;
    return toDoubleType<LChar, DisallowTrailingJunk>(data, length, ok, parsedLength);
}

double charactersToDouble(const UChar* data, size_t length, bool* ok)
{
    size_t parsedLength;
    return toDoubleType<UChar, DisallowTrailingJunk>(data, length, ok, parsedLength);
}

float charactersToFloat(const LChar* data, size_t length, bool* ok)
{
    // FIXME: This will return ok even when the string fits into a double but not a float.
    size_t parsedLength;
    return static_cast<float>(toDoubleType<LChar, DisallowTrailingJunk>(data, length, ok, parsedLength));
}

float charactersToFloat(const UChar* data, size_t length, bool* ok)
{
    // FIXME: This will return ok even when the string fits into a double but not a float.
    size_t parsedLength;
    return static_cast<float>(toDoubleType<UChar, DisallowTrailingJunk>(data, length, ok, parsedLength));
}

float charactersToFloat(const LChar* data, size_t length, size_t& parsedLength)
{
    // FIXME: This will return ok even when the string fits into a double but not a float.
    return static_cast<float>(toDoubleType<LChar, AllowTrailingJunk>(data, length, 0, parsedLength));
}

float charactersToFloat(const UChar* data, size_t length, size_t& parsedLength)
{
    // FIXME: This will return ok even when the string fits into a double but not a float.
    return static_cast<float>(toDoubleType<UChar, AllowTrailingJunk>(data, length, 0, parsedLength));
}

const String& emptyString()
{
    static NeverDestroyed<String> emptyString(StringImpl::empty());

    return emptyString;
}

} // namespace WTF

#ifndef NDEBUG
// For use in the debugger
String* string(const char*);
Vector<char> asciiDebug(StringImpl* impl);
Vector<char> asciiDebug(String& string);

void String::show() const
{
    dataLogF("%s\n", asciiDebug(impl()).data());
}

String* string(const char* s)
{
    // leaks memory!
    return new String(s);
}

Vector<char> asciiDebug(StringImpl* impl)
{
    if (!impl)
        return asciiDebug(String(ASCIILiteral("[null]")).impl());

    Vector<char> buffer;
    for (unsigned i = 0; i < impl->length(); ++i) {
        UChar ch = (*impl)[i];
        if (isASCIIPrintable(ch)) {
            if (ch == '\\')
                buffer.append(ch);
            buffer.append(ch);
        } else {
            buffer.append('\\');
            buffer.append('u');
            appendUnsignedAsHexFixedSize(ch, buffer, 4);
        }
    }
    buffer.append('\0');
    return buffer;
}

Vector<char> asciiDebug(String& string)
{
    return asciiDebug(string.impl());
}

#endif
