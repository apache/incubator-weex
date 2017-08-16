/*
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 *           (C) 2001 Dirk Mueller ( mueller@kde.org )
 * Copyright (C) 2003-2009, 2013-2016 Apple Inc. All rights reserved.
 * Copyright (C) 2006 Andrew Wellington (proton@wiretapped.net)
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
#include "StringImpl.h"

#include "AtomicString.h"
#include "StringBuffer.h"
#include "StringHash.h"
#include <wtf/ProcessID.h>
#include <wtf/StdLibExtras.h>
#include <wtf/text/CString.h>
#include <wtf/text/StringView.h>
#include <wtf/text/SymbolImpl.h>
#include <wtf/text/SymbolRegistry.h>
#include <wtf/unicode/CharacterNames.h>
#include <wtf/unicode/WTFUTF8.h>

#if STRING_STATS
#include <unistd.h>
#include <wtf/DataLog.h>
#endif

namespace WTF {

using namespace Unicode;

static_assert(sizeof(StringImpl) == 2 * sizeof(int) + 2 * sizeof(void*), "StringImpl should stay small");

#if STRING_STATS
StringStats StringImpl::m_stringStats;

std::atomic<unsigned> StringStats::s_stringRemovesTillPrintStats(s_printStringStatsFrequency);

void StringStats::removeString(StringImpl& string)
{
    unsigned length = string.length();
    bool isSubString = string.isSubString();

    --m_totalNumberStrings;

    if (string.is8Bit()) {
        --m_number8BitStrings;
        if (!isSubString)
            m_total8BitData -= length;
    } else {
        --m_number16BitStrings;
        if (!isSubString)
            m_total16BitData -= length;
    }

    if (!--s_stringRemovesTillPrintStats) {
        s_stringRemovesTillPrintStats = s_printStringStatsFrequency;
        printStats();
    }
}

void StringStats::printStats()
{
    dataLogF("String stats for process id %d:\n", getCurrentProcessID());

    unsigned long long totalNumberCharacters = m_total8BitData + m_total16BitData;
    double percent8Bit = m_totalNumberStrings ? ((double)m_number8BitStrings * 100) / (double)m_totalNumberStrings : 0.0;
    double average8bitLength = m_number8BitStrings ? (double)m_total8BitData / (double)m_number8BitStrings : 0.0;
    dataLogF("%8u (%5.2f%%) 8 bit        %12llu chars  %12llu bytes  avg length %6.1f\n", m_number8BitStrings.load(), percent8Bit, m_total8BitData.load(), m_total8BitData.load(), average8bitLength);

    double percent16Bit = m_totalNumberStrings ? ((double)m_number16BitStrings * 100) / (double)m_totalNumberStrings : 0.0;
    double average16bitLength = m_number16BitStrings ? (double)m_total16BitData / (double)m_number16BitStrings : 0.0;
    dataLogF("%8u (%5.2f%%) 16 bit       %12llu chars  %12llu bytes  avg length %6.1f\n", m_number16BitStrings.load(), percent16Bit, m_total16BitData.load(), m_total16BitData * 2, average16bitLength);

    double averageLength = m_totalNumberStrings ? (double)totalNumberCharacters / (double)m_totalNumberStrings : 0.0;
    unsigned long long totalDataBytes = m_total8BitData + m_total16BitData * 2;
    dataLogF("%8u Total                 %12llu chars  %12llu bytes  avg length %6.1f\n", m_totalNumberStrings.load(), totalNumberCharacters, totalDataBytes, averageLength);
    unsigned long long totalSavedBytes = m_total8BitData;
    double percentSavings = totalSavedBytes ? ((double)totalSavedBytes * 100) / (double)(totalDataBytes + totalSavedBytes) : 0.0;
    dataLogF("         Total savings %12llu bytes (%5.2f%%)\n", totalSavedBytes, percentSavings);

    dataLogF("%8u StringImpl::ref calls\n", m_refCalls.load());
    dataLogF("%8u StringImpl::deref calls\n", m_derefCalls.load());
}
#endif

StringImpl::StaticStringImpl StringImpl::s_atomicEmptyString("", StringImpl::StringAtomic);

StringImpl::~StringImpl()
{
    ASSERT(!isStatic());

    StringView::invalidate(*this);

    STRING_STATS_REMOVE_STRING(*this);

    if (isAtomic() && length() && !isSymbol())
        AtomicStringImpl::remove(static_cast<AtomicStringImpl*>(this));

    if (isSymbol()) {
        auto& symbol = static_cast<SymbolImpl&>(*this);
        auto* symbolRegistry = symbol.symbolRegistry();
        if (symbolRegistry)
            symbolRegistry->remove(symbol);
    }

    BufferOwnership ownership = bufferOwnership();

    if (ownership == BufferInternal)
        return;
    if (ownership == BufferOwned) {
        // We use m_data8, but since it is a union with m_data16 this works either way.
        ASSERT(m_data8);
        fastFree(const_cast<LChar*>(m_data8));
        return;
    }

    ASSERT(ownership == BufferSubstring);
    ASSERT(substringBuffer());
    substringBuffer()->deref();
}

void StringImpl::destroy(StringImpl* stringImpl)
{
    stringImpl->~StringImpl();
    fastFree(stringImpl);
}

Ref<StringImpl> StringImpl::createFromLiteral(const char* characters, unsigned length)
{
    ASSERT_WITH_MESSAGE(length, "Use StringImpl::empty() to create an empty string");
    ASSERT(charactersAreAllASCII<LChar>(reinterpret_cast<const LChar*>(characters), length));
    return adoptRef(*new StringImpl(reinterpret_cast<const LChar*>(characters), length, ConstructWithoutCopying));
}

Ref<StringImpl> StringImpl::createFromLiteral(const char* characters)
{
    return createFromLiteral(characters, strlen(characters));
}

Ref<StringImpl> StringImpl::createWithoutCopying(const UChar* characters, unsigned length)
{
    if (!length)
        return *empty();

    return adoptRef(*new StringImpl(characters, length, ConstructWithoutCopying));
}

Ref<StringImpl> StringImpl::createWithoutCopying(const LChar* characters, unsigned length)
{
    if (!length)
        return *empty();

    return adoptRef(*new StringImpl(characters, length, ConstructWithoutCopying));
}

template <typename CharType>
inline Ref<StringImpl> StringImpl::createUninitializedInternal(unsigned length, CharType*& data)
{
    if (!length) {
        data = 0;
        return *empty();
    }
    return createUninitializedInternalNonEmpty(length, data);
}

template <typename CharType>
inline Ref<StringImpl> StringImpl::createUninitializedInternalNonEmpty(unsigned length, CharType*& data)
{
    ASSERT(length);

    // Allocate a single buffer large enough to contain the StringImpl
    // struct as well as the data which it contains. This removes one
    // heap allocation from this call.
    if (length > ((std::numeric_limits<unsigned>::max() - sizeof(StringImpl)) / sizeof(CharType)))
        CRASH();
    StringImpl* string = static_cast<StringImpl*>(fastMalloc(allocationSize<CharType>(length)));

    data = string->tailPointer<CharType>();
    return constructInternal<CharType>(string, length);
}

Ref<StringImpl> StringImpl::createUninitialized(unsigned length, LChar*& data)
{
    return createUninitializedInternal(length, data);
}

Ref<StringImpl> StringImpl::createUninitialized(unsigned length, UChar*& data)
{
    return createUninitializedInternal(length, data);
}

template <typename CharType>
inline Ref<StringImpl> StringImpl::reallocateInternal(Ref<StringImpl>&& originalString, unsigned length, CharType*& data)
{
    ASSERT(originalString->hasOneRef());
    ASSERT(originalString->bufferOwnership() == BufferInternal);

    if (!length) {
        data = 0;
        return *empty();
    }

    // Same as createUninitialized() except here we use fastRealloc.
    if (length > ((std::numeric_limits<unsigned>::max() - sizeof(StringImpl)) / sizeof(CharType)))
        CRASH();

    originalString->~StringImpl();
    auto* string = static_cast<StringImpl*>(fastRealloc(&originalString.leakRef(), allocationSize<CharType>(length)));

    data = string->tailPointer<CharType>();
    return constructInternal<CharType>(string, length);
}

Ref<StringImpl> StringImpl::reallocate(Ref<StringImpl>&& originalString, unsigned length, LChar*& data)
{
    ASSERT(originalString->is8Bit());
    return reallocateInternal(WTFMove(originalString), length, data);
}

Ref<StringImpl> StringImpl::reallocate(Ref<StringImpl>&& originalString, unsigned length, UChar*& data)
{
    ASSERT(!originalString->is8Bit());
    return reallocateInternal(WTFMove(originalString), length, data);
}

template <typename CharType>
inline Ref<StringImpl> StringImpl::createInternal(const CharType* characters, unsigned length)
{
    if (!characters || !length)
        return *empty();

    CharType* data;
    auto string = createUninitializedInternalNonEmpty(length, data);
    memcpy(data, characters, length * sizeof(CharType));
    return string;
}

Ref<StringImpl> StringImpl::create(const UChar* characters, unsigned length)
{
    return createInternal(characters, length);
}

Ref<StringImpl> StringImpl::create(const LChar* characters, unsigned length)
{
    return createInternal(characters, length);
}

Ref<StringImpl> StringImpl::create8BitIfPossible(const UChar* characters, unsigned length)
{
    if (!characters || !length)
        return *empty();

    LChar* data;
    auto string = createUninitializedInternalNonEmpty(length, data);

    for (size_t i = 0; i < length; ++i) {
        if (characters[i] & 0xff00)
            return create(characters, length);
        data[i] = static_cast<LChar>(characters[i]);
    }

    return string;
}

Ref<StringImpl> StringImpl::create8BitIfPossible(const UChar* string)
{
    return StringImpl::create8BitIfPossible(string, lengthOfNullTerminatedString(string));
}

Ref<StringImpl> StringImpl::create(const LChar* string)
{
    if (!string)
        return *empty();
    size_t length = strlen(reinterpret_cast<const char*>(string));
    if (length > std::numeric_limits<unsigned>::max())
        CRASH();
    return create(string, length);
}

bool StringImpl::containsOnlyWhitespace()
{
    // FIXME: The definition of whitespace here includes a number of characters
    // that are not whitespace from the point of view of RenderText; I wonder if
    // that's a problem in practice.
    if (is8Bit()) {
        for (unsigned i = 0; i < m_length; ++i) {
            UChar c = m_data8[i];
            if (!isASCIISpace(c))
                return false;
        }

        return true;
    }

    for (unsigned i = 0; i < m_length; ++i) {
        UChar c = m_data16[i];
        if (!isASCIISpace(c))
            return false;
    }
    return true;
}

Ref<StringImpl> StringImpl::substring(unsigned start, unsigned length)
{
    if (start >= m_length)
        return *empty();
    unsigned maxLength = m_length - start;
    if (length >= maxLength) {
        if (!start)
            return *this;
        length = maxLength;
    }
    if (is8Bit())
        return create(m_data8 + start, length);

    return create(m_data16 + start, length);
}

UChar32 StringImpl::characterStartingAt(unsigned i)
{
    if (is8Bit())
        return m_data8[i];
    if (U16_IS_SINGLE(m_data16[i]))
        return m_data16[i];
    if (i + 1 < m_length && U16_IS_LEAD(m_data16[i]) && U16_IS_TRAIL(m_data16[i + 1]))
        return U16_GET_SUPPLEMENTARY(m_data16[i], m_data16[i + 1]);
    return 0;
}

Ref<StringImpl> StringImpl::convertToLowercaseWithoutLocale()
{
    // Note: At one time this was a hot function in the Dromaeo benchmark, specifically the
    // no-op code path that may return ourself if we find no upper case letters and no invalid
    // ASCII letters.

    // First scan the string for uppercase and non-ASCII characters:
    if (is8Bit()) {
        for (unsigned i = 0; i < m_length; ++i) {
            LChar character = m_data8[i];
            if (UNLIKELY((character & ~0x7F) || isASCIIUpper(character)))
                return convertToLowercaseWithoutLocaleStartingAtFailingIndex8Bit(i);
        }

        return *this;
    }

    bool noUpper = true;
    unsigned ored = 0;

    for (unsigned i = 0; i < m_length; ++i) {
        UChar character = m_data16[i];
        if (UNLIKELY(isASCIIUpper(character)))
            noUpper = false;
        ored |= character;
    }
    // Nothing to do if the string is all ASCII with no uppercase.
    if (noUpper && !(ored & ~0x7F))
        return *this;

    if (!(ored & ~0x7F)) {
        UChar* data16;
        auto newImpl = createUninitializedInternalNonEmpty(m_length, data16);
        
        for (unsigned i = 0; i < m_length; ++i) {
            UChar c = m_data16[i];
            data16[i] = toASCIILower(c);
        }
        return newImpl;
    }

    if (m_length > static_cast<unsigned>(std::numeric_limits<int32_t>::max()))
        CRASH();
    int32_t length = m_length;

    // Do a slower implementation for cases that include non-ASCII characters.
    UChar* data16;
    auto newImpl = createUninitializedInternalNonEmpty(m_length, data16);

    UErrorCode status = U_ZERO_ERROR;
    int32_t realLength = u_strToLower(data16, length, m_data16, m_length, "", &status);
    if (U_SUCCESS(status) && realLength == length)
        return newImpl;

    newImpl = createUninitialized(realLength, data16);
    status = U_ZERO_ERROR;
    u_strToLower(data16, realLength, m_data16, m_length, "", &status);
    if (U_FAILURE(status))
        return *this;
    return newImpl;
}

Ref<StringImpl> StringImpl::convertToLowercaseWithoutLocaleStartingAtFailingIndex8Bit(unsigned failingIndex)
{
    ASSERT(is8Bit());
    LChar* data8;
    auto newImpl = createUninitializedInternalNonEmpty(m_length, data8);

    for (unsigned i = 0; i < failingIndex; ++i) {
        ASSERT(!(m_data8[i] & ~0x7F) && !isASCIIUpper(m_data8[i]));
        data8[i] = m_data8[i];
    }

    for (unsigned i = failingIndex; i < m_length; ++i) {
        LChar character = m_data8[i];
        if (!(character & ~0x7F))
            data8[i] = toASCIILower(character);
        else {
            ASSERT(u_tolower(character) <= 0xFF);
            data8[i] = static_cast<LChar>(u_tolower(character));
        }
    }

    return newImpl;
}

Ref<StringImpl> StringImpl::convertToUppercaseWithoutLocale()
{
    // This function could be optimized for no-op cases the way
    // convertToLowercaseWithoutLocale() is, but in empirical testing,
    // few actual calls to upper() are no-ops, so it wouldn't be worth
    // the extra time for pre-scanning.

    if (m_length > static_cast<unsigned>(std::numeric_limits<int32_t>::max()))
        CRASH();
    int32_t length = m_length;

    if (is8Bit()) {
        LChar* data8;
        auto newImpl = createUninitialized(m_length, data8);
        
        // Do a faster loop for the case where all the characters are ASCII.
        unsigned ored = 0;
        for (int i = 0; i < length; ++i) {
            LChar c = m_data8[i];
            ored |= c;
            data8[i] = toASCIIUpper(c);
        }
        if (!(ored & ~0x7F))
            return newImpl;

        // Do a slower implementation for cases that include non-ASCII Latin-1 characters.
        int numberSharpSCharacters = 0;

        // There are two special cases.
        //  1. Some Latin-1 characters when converted to upper case are 16 bit characters.
        //  2. Lower case sharp-S converts to "SS" (two characters)
        for (int32_t i = 0; i < length; ++i) {
            LChar c = m_data8[i];
            if (UNLIKELY(c == smallLetterSharpS))
                ++numberSharpSCharacters;
            ASSERT(u_toupper(c) <= 0xFFFF);
            UChar upper = u_toupper(c);
            if (UNLIKELY(upper > 0xFF)) {
                // Since this upper-cased character does not fit in an 8-bit string, we need to take the 16-bit path.
                goto upconvert;
            }
            data8[i] = static_cast<LChar>(upper);
        }

        if (!numberSharpSCharacters)
            return newImpl;

        // We have numberSSCharacters sharp-s characters, but none of the other special characters.
        newImpl = createUninitialized(m_length + numberSharpSCharacters, data8);

        LChar* dest = data8;

        for (int32_t i = 0; i < length; ++i) {
            LChar c = m_data8[i];
            if (c == smallLetterSharpS) {
                *dest++ = 'S';
                *dest++ = 'S';
            } else {
                ASSERT(u_toupper(c) <= 0xFF);
                *dest++ = static_cast<LChar>(u_toupper(c));
            }
        }

        return newImpl;
    }

upconvert:
    auto upconvertedCharacters = StringView(*this).upconvertedCharacters();
    const UChar* source16 = upconvertedCharacters;

    UChar* data16;
    auto newImpl = createUninitialized(m_length, data16);
    
    // Do a faster loop for the case where all the characters are ASCII.
    unsigned ored = 0;
    for (int i = 0; i < length; ++i) {
        UChar c = source16[i];
        ored |= c;
        data16[i] = toASCIIUpper(c);
    }
    if (!(ored & ~0x7F))
        return newImpl;

    // Do a slower implementation for cases that include non-ASCII characters.
    UErrorCode status = U_ZERO_ERROR;
    int32_t realLength = u_strToUpper(data16, length, source16, m_length, "", &status);
    if (U_SUCCESS(status) && realLength == length)
        return newImpl;
    newImpl = createUninitialized(realLength, data16);
    status = U_ZERO_ERROR;
    u_strToUpper(data16, realLength, source16, m_length, "", &status);
    if (U_FAILURE(status))
        return *this;
    return newImpl;
}

static inline bool needsTurkishCasingRules(const AtomicString& localeIdentifier)
{
    // Either "tr" or "az" locale, with case sensitive comparison and allowing for an ignored subtag.
    UChar first = localeIdentifier[0];
    UChar second = localeIdentifier[1];
    return ((isASCIIAlphaCaselessEqual(first, 't') && isASCIIAlphaCaselessEqual(second, 'r'))
        || (isASCIIAlphaCaselessEqual(first, 'a') && isASCIIAlphaCaselessEqual(second, 'z')))
        && (localeIdentifier.length() == 2 || localeIdentifier[2] == '-');
}

Ref<StringImpl> StringImpl::convertToLowercaseWithLocale(const AtomicString& localeIdentifier)
{
    // Use the more-optimized code path most of the time.
    // Assuming here that the only locale-specific lowercasing is the Turkish casing rules.
    // FIXME: Could possibly optimize further by looking for the specific sequences
    // that have locale-specific lowercasing. There are only three of them.
    if (!needsTurkishCasingRules(localeIdentifier))
        return convertToLowercaseWithoutLocale();

    // FIXME: Could share more code with the main StringImpl::lower by factoring out
    // this last part into a shared function that takes a locale string, since this is
    // just like the end of that function.

    if (m_length > static_cast<unsigned>(std::numeric_limits<int32_t>::max()))
        CRASH();
    int length = m_length;

    // Below, we pass in the hardcoded locale "tr". Passing that is more efficient than
    // allocating memory just to turn localeIdentifier into a C string, and we assume
    // there is no difference between the uppercasing for "tr" and "az" locales.
    auto upconvertedCharacters = StringView(*this).upconvertedCharacters();
    const UChar* source16 = upconvertedCharacters;
    UChar* data16;
    auto newString = createUninitialized(length, data16);
    UErrorCode status = U_ZERO_ERROR;
    int realLength = u_strToLower(data16, length, source16, length, "tr", &status);
    if (U_SUCCESS(status) && realLength == length)
        return newString;
    newString = createUninitialized(realLength, data16);
    status = U_ZERO_ERROR;
    u_strToLower(data16, realLength, source16, length, "tr", &status);
    if (U_FAILURE(status))
        return *this;
    return newString;
}

Ref<StringImpl> StringImpl::convertToUppercaseWithLocale(const AtomicString& localeIdentifier)
{
    // Use the more-optimized code path most of the time.
    // Assuming here that the only locale-specific lowercasing is the Turkish casing rules,
    // and that the only affected character is lowercase "i".
    if (!needsTurkishCasingRules(localeIdentifier) || find('i') == notFound)
        return convertToUppercaseWithoutLocale();

    if (m_length > static_cast<unsigned>(std::numeric_limits<int32_t>::max()))
        CRASH();
    int length = m_length;

    // Below, we pass in the hardcoded locale "tr". Passing that is more efficient than
    // allocating memory just to turn localeIdentifier into a C string, and we assume
    // there is no difference between the uppercasing for "tr" and "az" locales.
    auto upconvertedCharacters = StringView(*this).upconvertedCharacters();
    const UChar* source16 = upconvertedCharacters;
    UChar* data16;
    auto newString = createUninitialized(length, data16);
    UErrorCode status = U_ZERO_ERROR;
    int realLength = u_strToUpper(data16, length, source16, length, "tr", &status);
    if (U_SUCCESS(status) && realLength == length)
        return newString;
    newString = createUninitialized(realLength, data16);
    status = U_ZERO_ERROR;
    u_strToUpper(data16, realLength, source16, length, "tr", &status);
    if (U_FAILURE(status))
        return *this;
    return newString;
}

Ref<StringImpl> StringImpl::foldCase()
{
    if (is8Bit()) {
        unsigned failingIndex;
        for (unsigned i = 0; i < m_length; ++i) {
            auto character = m_data8[i];
            if (UNLIKELY(!isASCII(character) || isASCIIUpper(character))) {
                failingIndex = i;
                goto SlowPath;
            }
        }
        // String was all ASCII and no uppercase, so just return as-is.
        return *this;

SlowPath:
        bool need16BitCharacters = false;
        for (unsigned i = failingIndex; i < m_length; ++i) {
            auto character = m_data8[i];
            if (character == 0xB5 || character == 0xDF) {
                need16BitCharacters = true;
                break;
            }
        }

        if (!need16BitCharacters) {
            LChar* data8;
            auto folded = createUninitializedInternalNonEmpty(m_length, data8);
            for (unsigned i = 0; i < failingIndex; ++i)
                data8[i] = m_data8[i];
            for (unsigned i = failingIndex; i < m_length; ++i) {
                auto character = m_data8[i];
                if (isASCII(character))
                    data8[i] = toASCIILower(character);
                else {
                    ASSERT(u_foldCase(character, U_FOLD_CASE_DEFAULT) <= 0xFF);
                    data8[i] = static_cast<LChar>(u_foldCase(character, U_FOLD_CASE_DEFAULT));
                }
            }
            return folded;
        }
    } else {
        // FIXME: Unclear why we use goto in the 8-bit case, and a different approach in the 16-bit case.
        bool noUpper = true;
        unsigned ored = 0;
        for (unsigned i = 0; i < m_length; ++i) {
            UChar character = m_data16[i];
            if (UNLIKELY(isASCIIUpper(character)))
                noUpper = false;
            ored |= character;
        }
        if (!(ored & ~0x7F)) {
            if (noUpper) {
                // String was all ASCII and no uppercase, so just return as-is.
                return *this;
            }
            UChar* data16;
            auto folded = createUninitializedInternalNonEmpty(m_length, data16);
            for (unsigned i = 0; i < m_length; ++i)
                data16[i] = toASCIILower(m_data16[i]);
            return folded;
        }
    }

    if (m_length > static_cast<unsigned>(std::numeric_limits<int32_t>::max()))
        CRASH();

    auto upconvertedCharacters = StringView(*this).upconvertedCharacters();

    UChar* data;
    auto folded = createUninitializedInternalNonEmpty(m_length, data);
    int32_t length = m_length;
    UErrorCode status = U_ZERO_ERROR;
    int32_t realLength = u_strFoldCase(data, length, upconvertedCharacters, length, U_FOLD_CASE_DEFAULT, &status);
    if (U_SUCCESS(status) && realLength == length)
        return folded;
    ASSERT(realLength > length);
    folded = createUninitializedInternalNonEmpty(realLength, data);
    status = U_ZERO_ERROR;
    u_strFoldCase(data, realLength, upconvertedCharacters, length, U_FOLD_CASE_DEFAULT, &status);
    if (U_FAILURE(status))
        return *this;
    return folded;
}

template<StringImpl::CaseConvertType type, typename CharacterType>
ALWAYS_INLINE Ref<StringImpl> StringImpl::convertASCIICase(StringImpl& impl, const CharacterType* data, unsigned length)
{
    unsigned failingIndex;
    for (unsigned i = 0; i < length; ++i) {
        CharacterType character = data[i];
        if (type == CaseConvertType::Lower ? UNLIKELY(isASCIIUpper(character)) : LIKELY(isASCIILower(character))) {
            failingIndex = i;
            goto SlowPath;
        }
    }
    return impl;

SlowPath:
    CharacterType* newData;
    auto newImpl = createUninitializedInternalNonEmpty(length, newData);
    for (unsigned i = 0; i < failingIndex; ++i)
        newData[i] = data[i];
    for (unsigned i = failingIndex; i < length; ++i)
        newData[i] = type == CaseConvertType::Lower ? toASCIILower(data[i]) : toASCIIUpper(data[i]);
    return newImpl;
}

Ref<StringImpl> StringImpl::convertToASCIILowercase()
{
    if (is8Bit())
        return convertASCIICase<CaseConvertType::Lower>(*this, m_data8, m_length);
    return convertASCIICase<CaseConvertType::Lower>(*this, m_data16, m_length);
}

Ref<StringImpl> StringImpl::convertToASCIIUppercase()
{
    if (is8Bit())
        return convertASCIICase<CaseConvertType::Upper>(*this, m_data8, m_length);
    return convertASCIICase<CaseConvertType::Upper>(*this, m_data16, m_length);
}

template <class UCharPredicate>
inline Ref<StringImpl> StringImpl::stripMatchedCharacters(UCharPredicate predicate)
{
    if (!m_length)
        return *this;

    unsigned start = 0;
    unsigned end = m_length - 1;
    
    // skip white space from start
    while (start <= end && predicate(is8Bit() ? m_data8[start] : m_data16[start]))
        ++start;
    
    // only white space
    if (start > end) 
        return *empty();

    // skip white space from end
    while (end && predicate(is8Bit() ? m_data8[end] : m_data16[end]))
        --end;

    if (!start && end == m_length - 1)
        return *this;
    if (is8Bit())
        return create(m_data8 + start, end + 1 - start);
    return create(m_data16 + start, end + 1 - start);
}

class UCharPredicate {
public:
    inline UCharPredicate(CharacterMatchFunctionPtr function): m_function(function) { }

    inline bool operator()(UChar ch) const
    {
        return m_function(ch);
    }

private:
    const CharacterMatchFunctionPtr m_function;
};

class SpaceOrNewlinePredicate {
public:
    inline bool operator()(UChar ch) const
    {
        return isSpaceOrNewline(ch);
    }
};

Ref<StringImpl> StringImpl::stripWhiteSpace()
{
    return stripMatchedCharacters(SpaceOrNewlinePredicate());
}

Ref<StringImpl> StringImpl::stripWhiteSpace(IsWhiteSpaceFunctionPtr isWhiteSpace)
{
    return stripMatchedCharacters(UCharPredicate(isWhiteSpace));
}

template <typename CharType>
ALWAYS_INLINE Ref<StringImpl> StringImpl::removeCharacters(const CharType* characters, CharacterMatchFunctionPtr findMatch)
{
    const CharType* from = characters;
    const CharType* fromend = from + m_length;
    
    // Assume the common case will not remove any characters
    while (from != fromend && !findMatch(*from))
        ++from;
    if (from == fromend)
        return *this;
    
    StringBuffer<CharType> data(m_length);
    CharType* to = data.characters();
    unsigned outc = from - characters;
    
    if (outc)
        memcpy(to, characters, outc * sizeof(CharType));

    while (true) {
        while (from != fromend && findMatch(*from))
            ++from;
        while (from != fromend && !findMatch(*from))
            to[outc++] = *from++;
        if (from == fromend)
            break;
    }

    data.shrink(outc);

    return adopt(WTFMove(data));
}

Ref<StringImpl> StringImpl::removeCharacters(CharacterMatchFunctionPtr findMatch)
{
    if (is8Bit())
        return removeCharacters(characters8(), findMatch);
    return removeCharacters(characters16(), findMatch);
}

template <typename CharType, class UCharPredicate>
inline Ref<StringImpl> StringImpl::simplifyMatchedCharactersToSpace(UCharPredicate predicate)
{
    StringBuffer<CharType> data(m_length);

    const CharType* from = characters<CharType>();
    const CharType* fromend = from + m_length;
    int outc = 0;
    bool changedToSpace = false;
    
    CharType* to = data.characters();
    
    while (true) {
        while (from != fromend && predicate(*from)) {
            if (*from != ' ')
                changedToSpace = true;
            ++from;
        }
        while (from != fromend && !predicate(*from))
            to[outc++] = *from++;
        if (from != fromend)
            to[outc++] = ' ';
        else
            break;
    }
    
    if (outc > 0 && to[outc - 1] == ' ')
        --outc;
    
    if (static_cast<unsigned>(outc) == m_length && !changedToSpace)
        return *this;
    
    data.shrink(outc);
    
    return adopt(WTFMove(data));
}

Ref<StringImpl> StringImpl::simplifyWhiteSpace()
{
    if (is8Bit())
        return StringImpl::simplifyMatchedCharactersToSpace<LChar>(SpaceOrNewlinePredicate());
    return StringImpl::simplifyMatchedCharactersToSpace<UChar>(SpaceOrNewlinePredicate());
}

Ref<StringImpl> StringImpl::simplifyWhiteSpace(IsWhiteSpaceFunctionPtr isWhiteSpace)
{
    if (is8Bit())
        return StringImpl::simplifyMatchedCharactersToSpace<LChar>(UCharPredicate(isWhiteSpace));
    return StringImpl::simplifyMatchedCharactersToSpace<UChar>(UCharPredicate(isWhiteSpace));
}

int StringImpl::toIntStrict(bool* ok, int base)
{
    if (is8Bit())
        return charactersToIntStrict(characters8(), m_length, ok, base);
    return charactersToIntStrict(characters16(), m_length, ok, base);
}

unsigned StringImpl::toUIntStrict(bool* ok, int base)
{
    if (is8Bit())
        return charactersToUIntStrict(characters8(), m_length, ok, base);
    return charactersToUIntStrict(characters16(), m_length, ok, base);
}

int64_t StringImpl::toInt64Strict(bool* ok, int base)
{
    if (is8Bit())
        return charactersToInt64Strict(characters8(), m_length, ok, base);
    return charactersToInt64Strict(characters16(), m_length, ok, base);
}

uint64_t StringImpl::toUInt64Strict(bool* ok, int base)
{
    if (is8Bit())
        return charactersToUInt64Strict(characters8(), m_length, ok, base);
    return charactersToUInt64Strict(characters16(), m_length, ok, base);
}

intptr_t StringImpl::toIntPtrStrict(bool* ok, int base)
{
    if (is8Bit())
        return charactersToIntPtrStrict(characters8(), m_length, ok, base);
    return charactersToIntPtrStrict(characters16(), m_length, ok, base);
}

int StringImpl::toInt(bool* ok)
{
    if (is8Bit())
        return charactersToInt(characters8(), m_length, ok);
    return charactersToInt(characters16(), m_length, ok);
}

unsigned StringImpl::toUInt(bool* ok)
{
    if (is8Bit())
        return charactersToUInt(characters8(), m_length, ok);
    return charactersToUInt(characters16(), m_length, ok);
}

int64_t StringImpl::toInt64(bool* ok)
{
    if (is8Bit())
        return charactersToInt64(characters8(), m_length, ok);
    return charactersToInt64(characters16(), m_length, ok);
}

uint64_t StringImpl::toUInt64(bool* ok)
{
    if (is8Bit())
        return charactersToUInt64(characters8(), m_length, ok);
    return charactersToUInt64(characters16(), m_length, ok);
}

intptr_t StringImpl::toIntPtr(bool* ok)
{
    if (is8Bit())
        return charactersToIntPtr(characters8(), m_length, ok);
    return charactersToIntPtr(characters16(), m_length, ok);
}

double StringImpl::toDouble(bool* ok)
{
    if (is8Bit())
        return charactersToDouble(characters8(), m_length, ok);
    return charactersToDouble(characters16(), m_length, ok);
}

float StringImpl::toFloat(bool* ok)
{
    if (is8Bit())
        return charactersToFloat(characters8(), m_length, ok);
    return charactersToFloat(characters16(), m_length, ok);
}

// Table is based on ftp://ftp.unicode.org/Public/UNIDATA/CaseFolding.txt
static const UChar latin1CaseFoldTable[256] = {
    0x0000, 0x0001, 0x0002, 0x0003, 0x0004, 0x0005, 0x0006, 0x0007, 0x0008, 0x0009, 0x000a, 0x000b, 0x000c, 0x000d, 0x000e, 0x000f,
    0x0010, 0x0011, 0x0012, 0x0013, 0x0014, 0x0015, 0x0016, 0x0017, 0x0018, 0x0019, 0x001a, 0x001b, 0x001c, 0x001d, 0x001e, 0x001f,
    0x0020, 0x0021, 0x0022, 0x0023, 0x0024, 0x0025, 0x0026, 0x0027, 0x0028, 0x0029, 0x002a, 0x002b, 0x002c, 0x002d, 0x002e, 0x002f,
    0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037, 0x0038, 0x0039, 0x003a, 0x003b, 0x003c, 0x003d, 0x003e, 0x003f,
    0x0040, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x0067, 0x0068, 0x0069, 0x006a, 0x006b, 0x006c, 0x006d, 0x006e, 0x006f,
    0x0070, 0x0071, 0x0072, 0x0073, 0x0074, 0x0075, 0x0076, 0x0077, 0x0078, 0x0079, 0x007a, 0x005b, 0x005c, 0x005d, 0x005e, 0x005f,
    0x0060, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x0067, 0x0068, 0x0069, 0x006a, 0x006b, 0x006c, 0x006d, 0x006e, 0x006f,
    0x0070, 0x0071, 0x0072, 0x0073, 0x0074, 0x0075, 0x0076, 0x0077, 0x0078, 0x0079, 0x007a, 0x007b, 0x007c, 0x007d, 0x007e, 0x007f,
    0x0080, 0x0081, 0x0082, 0x0083, 0x0084, 0x0085, 0x0086, 0x0087, 0x0088, 0x0089, 0x008a, 0x008b, 0x008c, 0x008d, 0x008e, 0x008f,
    0x0090, 0x0091, 0x0092, 0x0093, 0x0094, 0x0095, 0x0096, 0x0097, 0x0098, 0x0099, 0x009a, 0x009b, 0x009c, 0x009d, 0x009e, 0x009f,
    0x00a0, 0x00a1, 0x00a2, 0x00a3, 0x00a4, 0x00a5, 0x00a6, 0x00a7, 0x00a8, 0x00a9, 0x00aa, 0x00ab, 0x00ac, 0x00ad, 0x00ae, 0x00af,
    0x00b0, 0x00b1, 0x00b2, 0x00b3, 0x00b4, 0x03bc, 0x00b6, 0x00b7, 0x00b8, 0x00b9, 0x00ba, 0x00bb, 0x00bc, 0x00bd, 0x00be, 0x00bf,
    0x00e0, 0x00e1, 0x00e2, 0x00e3, 0x00e4, 0x00e5, 0x00e6, 0x00e7, 0x00e8, 0x00e9, 0x00ea, 0x00eb, 0x00ec, 0x00ed, 0x00ee, 0x00ef,
    0x00f0, 0x00f1, 0x00f2, 0x00f3, 0x00f4, 0x00f5, 0x00f6, 0x00d7, 0x00f8, 0x00f9, 0x00fa, 0x00fb, 0x00fc, 0x00fd, 0x00fe, 0x00df,
    0x00e0, 0x00e1, 0x00e2, 0x00e3, 0x00e4, 0x00e5, 0x00e6, 0x00e7, 0x00e8, 0x00e9, 0x00ea, 0x00eb, 0x00ec, 0x00ed, 0x00ee, 0x00ef,
    0x00f0, 0x00f1, 0x00f2, 0x00f3, 0x00f4, 0x00f5, 0x00f6, 0x00f7, 0x00f8, 0x00f9, 0x00fa, 0x00fb, 0x00fc, 0x00fd, 0x00fe, 0x00ff,
};

static inline bool equalCompatibilityCaseless(const LChar* a, const LChar* b, unsigned length)
{
    while (length--) {
        if (latin1CaseFoldTable[*a++] != latin1CaseFoldTable[*b++])
            return false;
    }
    return true;
}

static inline bool equalCompatibilityCaseless(const UChar* a, const LChar* b, unsigned length)
{
    while (length--) {
        if (u_foldCase(*a++, U_FOLD_CASE_DEFAULT) != latin1CaseFoldTable[*b++])
            return false;
    }
    return true;
}

static inline bool equalCompatibilityCaseless(const LChar* a, const UChar* b, unsigned length)
{
    return equalCompatibilityCaseless(b, a, length);
}

static inline bool equalCompatibilityCaseless(const UChar* a, const UChar* b, unsigned length)
{
    return !u_memcasecmp(a, b, length, U_FOLD_CASE_DEFAULT);
}

size_t StringImpl::find(CharacterMatchFunctionPtr matchFunction, unsigned start)
{
    if (is8Bit())
        return WTF::find(characters8(), m_length, matchFunction, start);
    return WTF::find(characters16(), m_length, matchFunction, start);
}

size_t StringImpl::find(const LChar* matchString, unsigned index)
{
    // Check for null or empty string to match against
    if (!matchString)
        return notFound;
    size_t matchStringLength = strlen(reinterpret_cast<const char*>(matchString));
    if (matchStringLength > std::numeric_limits<unsigned>::max())
        CRASH();
    unsigned matchLength = matchStringLength;
    if (!matchLength)
        return std::min(index, length());

    // Optimization 1: fast case for strings of length 1.
    if (matchLength == 1) {
        if (is8Bit())
            return WTF::find(characters8(), length(), matchString[0], index);
        return WTF::find(characters16(), length(), *matchString, index);
    }

    // Check index & matchLength are in range.
    if (index > length())
        return notFound;
    unsigned searchLength = length() - index;
    if (matchLength > searchLength)
        return notFound;
    // delta is the number of additional times to test; delta == 0 means test only once.
    unsigned delta = searchLength - matchLength;

    // Optimization 2: keep a running hash of the strings,
    // only call equal if the hashes match.

    if (is8Bit()) {
        const LChar* searchCharacters = characters8() + index;

        unsigned searchHash = 0;
        unsigned matchHash = 0;
        for (unsigned i = 0; i < matchLength; ++i) {
            searchHash += searchCharacters[i];
            matchHash += matchString[i];
        }

        unsigned i = 0;
        while (searchHash != matchHash || !equal(searchCharacters + i, matchString, matchLength)) {
            if (i == delta)
                return notFound;
            searchHash += searchCharacters[i + matchLength];
            searchHash -= searchCharacters[i];
            ++i;
        }
        return index + i;
    }

    const UChar* searchCharacters = characters16() + index;

    unsigned searchHash = 0;
    unsigned matchHash = 0;
    for (unsigned i = 0; i < matchLength; ++i) {
        searchHash += searchCharacters[i];
        matchHash += matchString[i];
    }

    unsigned i = 0;
    while (searchHash != matchHash || !equal(searchCharacters + i, matchString, matchLength)) {
        if (i == delta)
            return notFound;
        searchHash += searchCharacters[i + matchLength];
        searchHash -= searchCharacters[i];
        ++i;
    }
    return index + i;
}

size_t StringImpl::findIgnoringCase(const LChar* matchString, unsigned index)
{
    // Check for null or empty string to match against
    if (!matchString)
        return notFound;
    size_t matchStringLength = strlen(reinterpret_cast<const char*>(matchString));
    if (matchStringLength > std::numeric_limits<unsigned>::max())
        CRASH();
    unsigned matchLength = matchStringLength;
    if (!matchLength)
        return std::min(index, length());

    // Check index & matchLength are in range.
    if (index > length())
        return notFound;
    unsigned searchLength = length() - index;
    if (matchLength > searchLength)
        return notFound;
    // delta is the number of additional times to test; delta == 0 means test only once.
    unsigned delta = searchLength - matchLength;

    if (is8Bit()) {
        const LChar* searchCharacters = characters8() + index;

        unsigned i = 0;
        while (!equalCompatibilityCaseless(searchCharacters + i, matchString, matchLength)) {
            if (i == delta)
                return notFound;
            ++i;
        }
        return index + i;
    }

    const UChar* searchCharacters = characters16() + index;

    unsigned i = 0;
    while (!equalCompatibilityCaseless(searchCharacters + i, matchString, matchLength)) {
        if (i == delta)
            return notFound;
        ++i;
    }
    return index + i;
}

size_t StringImpl::find(StringImpl* matchString)
{
    // Check for null string to match against
    if (UNLIKELY(!matchString))
        return notFound;
    unsigned matchLength = matchString->length();

    // Optimization 1: fast case for strings of length 1.
    if (matchLength == 1) {
        if (is8Bit()) {
            if (matchString->is8Bit())
                return WTF::find(characters8(), length(), matchString->characters8()[0]);
            return WTF::find(characters8(), length(), matchString->characters16()[0]);
        }
        if (matchString->is8Bit())
            return WTF::find(characters16(), length(), matchString->characters8()[0]);
        return WTF::find(characters16(), length(), matchString->characters16()[0]);
    }

    // Check matchLength is in range.
    if (matchLength > length())
        return notFound;

    // Check for empty string to match against
    if (UNLIKELY(!matchLength))
        return 0;

    if (is8Bit()) {
        if (matchString->is8Bit())
            return findInner(characters8(), matchString->characters8(), 0, length(), matchLength);
        return findInner(characters8(), matchString->characters16(), 0, length(), matchLength);
    }

    if (matchString->is8Bit())
        return findInner(characters16(), matchString->characters8(), 0, length(), matchLength);

    return findInner(characters16(), matchString->characters16(), 0, length(), matchLength);
}

size_t StringImpl::find(StringImpl* matchString, unsigned index)
{
    // Check for null or empty string to match against
    if (UNLIKELY(!matchString))
        return notFound;

    return findCommon(*this, *matchString, index);
}

template <typename SearchCharacterType, typename MatchCharacterType>
ALWAYS_INLINE static size_t findIgnoringCaseInner(const SearchCharacterType* searchCharacters, const MatchCharacterType* matchCharacters, unsigned index, unsigned searchLength, unsigned matchLength)
{
    // delta is the number of additional times to test; delta == 0 means test only once.
    unsigned delta = searchLength - matchLength;

    unsigned i = 0;
    // keep looping until we match
    while (!equalCompatibilityCaseless(searchCharacters + i, matchCharacters, matchLength)) {
        if (i == delta)
            return notFound;
        ++i;
    }
    return index + i;
}

size_t StringImpl::findIgnoringCase(StringImpl* matchString, unsigned index)
{
    // Check for null or empty string to match against
    if (!matchString)
        return notFound;
    unsigned matchLength = matchString->length();
    if (!matchLength)
        return std::min(index, length());

    // Check index & matchLength are in range.
    if (index > length())
        return notFound;
    unsigned searchLength = length() - index;
    if (matchLength > searchLength)
        return notFound;

    if (is8Bit()) {
        if (matchString->is8Bit())
            return findIgnoringCaseInner(characters8() + index, matchString->characters8(), index, searchLength, matchLength);
        return findIgnoringCaseInner(characters8() + index, matchString->characters16(), index, searchLength, matchLength);
    }

    if (matchString->is8Bit())
        return findIgnoringCaseInner(characters16() + index, matchString->characters8(), index, searchLength, matchLength);

    return findIgnoringCaseInner(characters16() + index, matchString->characters16(), index, searchLength, matchLength);
}

size_t StringImpl::findIgnoringASCIICase(const StringImpl& matchString) const
{
    return ::WTF::findIgnoringASCIICase(*this, matchString, 0);
}

size_t StringImpl::findIgnoringASCIICase(const StringImpl& matchString, unsigned startOffset) const
{
    return ::WTF::findIgnoringASCIICase(*this, matchString, startOffset);
}

size_t StringImpl::findIgnoringASCIICase(const StringImpl* matchString) const
{
    if (!matchString)
        return notFound;
    return ::WTF::findIgnoringASCIICase(*this, *matchString, 0);
}

size_t StringImpl::findIgnoringASCIICase(const StringImpl* matchString, unsigned startOffset) const
{
    if (!matchString)
        return notFound;
    return ::WTF::findIgnoringASCIICase(*this, *matchString, startOffset);
}

size_t StringImpl::reverseFind(UChar c, unsigned index)
{
    if (is8Bit())
        return WTF::reverseFind(characters8(), m_length, c, index);
    return WTF::reverseFind(characters16(), m_length, c, index);
}

template <typename SearchCharacterType, typename MatchCharacterType>
ALWAYS_INLINE static size_t reverseFindInner(const SearchCharacterType* searchCharacters, const MatchCharacterType* matchCharacters, unsigned index, unsigned length, unsigned matchLength)
{
    // Optimization: keep a running hash of the strings,
    // only call equal if the hashes match.

    // delta is the number of additional times to test; delta == 0 means test only once.
    unsigned delta = std::min(index, length - matchLength);
    
    unsigned searchHash = 0;
    unsigned matchHash = 0;
    for (unsigned i = 0; i < matchLength; ++i) {
        searchHash += searchCharacters[delta + i];
        matchHash += matchCharacters[i];
    }

    // keep looping until we match
    while (searchHash != matchHash || !equal(searchCharacters + delta, matchCharacters, matchLength)) {
        if (!delta)
            return notFound;
        --delta;
        searchHash -= searchCharacters[delta + matchLength];
        searchHash += searchCharacters[delta];
    }
    return delta;
}

size_t StringImpl::reverseFind(StringImpl* matchString, unsigned index)
{
    // Check for null or empty string to match against
    if (!matchString)
        return notFound;
    unsigned matchLength = matchString->length();
    unsigned ourLength = length();
    if (!matchLength)
        return std::min(index, ourLength);

    // Optimization 1: fast case for strings of length 1.
    if (matchLength == 1) {
        if (is8Bit())
            return WTF::reverseFind(characters8(), ourLength, (*matchString)[0], index);
        return WTF::reverseFind(characters16(), ourLength, (*matchString)[0], index);
    }

    // Check index & matchLength are in range.
    if (matchLength > ourLength)
        return notFound;

    if (is8Bit()) {
        if (matchString->is8Bit())
            return reverseFindInner(characters8(), matchString->characters8(), index, ourLength, matchLength);
        return reverseFindInner(characters8(), matchString->characters16(), index, ourLength, matchLength);
    }
    
    if (matchString->is8Bit())
        return reverseFindInner(characters16(), matchString->characters8(), index, ourLength, matchLength);

    return reverseFindInner(characters16(), matchString->characters16(), index, ourLength, matchLength);
}

template <typename SearchCharacterType, typename MatchCharacterType>
ALWAYS_INLINE static size_t reverseFindIgnoringCaseInner(const SearchCharacterType* searchCharacters, const MatchCharacterType* matchCharacters, unsigned index, unsigned length, unsigned matchLength)
{
    // delta is the number of additional times to test; delta == 0 means test only once.
    unsigned delta = std::min(index, length - matchLength);

    // keep looping until we match
    while (!equalCompatibilityCaseless(searchCharacters + delta, matchCharacters, matchLength)) {
        if (!delta)
            return notFound;
        --delta;
    }
    return delta;
}

size_t StringImpl::reverseFindIgnoringCase(StringImpl* matchString, unsigned index)
{
    // Check for null or empty string to match against
    if (!matchString)
        return notFound;
    unsigned matchLength = matchString->length();
    unsigned ourLength = length();
    if (!matchLength)
        return std::min(index, ourLength);

    // Check index & matchLength are in range.
    if (matchLength > ourLength)
        return notFound;

    if (is8Bit()) {
        if (matchString->is8Bit())
            return reverseFindIgnoringCaseInner(characters8(), matchString->characters8(), index, ourLength, matchLength);
        return reverseFindIgnoringCaseInner(characters8(), matchString->characters16(), index, ourLength, matchLength);
    }

    if (matchString->is8Bit())
        return reverseFindIgnoringCaseInner(characters16(), matchString->characters8(), index, ourLength, matchLength);

    return reverseFindIgnoringCaseInner(characters16(), matchString->characters16(), index, ourLength, matchLength);
}

ALWAYS_INLINE static bool equalInner(const StringImpl* stringImpl, unsigned startOffset, const char* matchString, unsigned matchLength, bool caseSensitive)
{
    ASSERT(stringImpl);
    ASSERT(matchLength <= stringImpl->length());
    ASSERT(startOffset + matchLength <= stringImpl->length());

    if (caseSensitive) {
        if (stringImpl->is8Bit())
            return equal(stringImpl->characters8() + startOffset, reinterpret_cast<const LChar*>(matchString), matchLength);
        return equal(stringImpl->characters16() + startOffset, reinterpret_cast<const LChar*>(matchString), matchLength);
    }
    if (stringImpl->is8Bit())
        return equalCompatibilityCaseless(stringImpl->characters8() + startOffset, reinterpret_cast<const LChar*>(matchString), matchLength);
    return equalCompatibilityCaseless(stringImpl->characters16() + startOffset, reinterpret_cast<const LChar*>(matchString), matchLength);
}

ALWAYS_INLINE static bool equalInner(const StringImpl& stringImpl, unsigned startOffset, const StringImpl& matchString)
{
    if (startOffset > stringImpl.length())
        return false;
    if (matchString.length() > stringImpl.length())
        return false;
    if (matchString.length() + startOffset > stringImpl.length())
        return false;

    if (stringImpl.is8Bit()) {
        if (matchString.is8Bit())
            return equal(stringImpl.characters8() + startOffset, matchString.characters8(), matchString.length());
        return equal(stringImpl.characters8() + startOffset, matchString.characters16(), matchString.length());
    }
    if (matchString.is8Bit())
        return equal(stringImpl.characters16() + startOffset, matchString.characters8(), matchString.length());
    return equal(stringImpl.characters16() + startOffset, matchString.characters16(), matchString.length());
}

bool StringImpl::startsWith(const StringImpl* str) const
{
    if (!str)
        return false;
    return ::WTF::startsWith(*this, *str);
}

bool StringImpl::startsWith(const StringImpl& str) const
{
    return ::WTF::startsWith(*this, str);
}

bool StringImpl::startsWithIgnoringASCIICase(const StringImpl* prefix) const
{
    if (!prefix)
        return false;

    return ::WTF::startsWithIgnoringASCIICase(*this, *prefix);
}

bool StringImpl::startsWithIgnoringASCIICase(const StringImpl& prefix) const
{
    return ::WTF::startsWithIgnoringASCIICase(*this, prefix);
}

bool StringImpl::startsWith(UChar character) const
{
    return m_length && (*this)[0] == character;
}

bool StringImpl::startsWith(const char* matchString, unsigned matchLength, bool caseSensitive) const
{
    ASSERT(matchLength);
    if (matchLength > length())
        return false;
    return equalInner(this, 0, matchString, matchLength, caseSensitive);
}

bool StringImpl::hasInfixStartingAt(const StringImpl& matchString, unsigned startOffset) const
{
    return equalInner(*this, startOffset, matchString);
}

bool StringImpl::endsWith(StringImpl* suffix)
{
    if (!suffix)
        return false;

    return ::WTF::endsWith(*this, *suffix);
}

bool StringImpl::endsWith(StringImpl& suffix)
{
    return ::WTF::endsWith(*this, suffix);
}

bool StringImpl::endsWith(StringImpl* matchString, bool caseSensitive)
{
    ASSERT(matchString);
    if (m_length >= matchString->m_length) {
        unsigned start = m_length - matchString->m_length;
        return (caseSensitive ? find(matchString, start) : findIgnoringCase(matchString, start)) == start;
    }
    return false;
}

bool StringImpl::endsWithIgnoringASCIICase(const StringImpl* suffix) const
{
    if (!suffix)
        return false;

    return ::WTF::endsWithIgnoringASCIICase(*this, *suffix);
}

bool StringImpl::endsWithIgnoringASCIICase(const StringImpl& suffix) const
{
    return ::WTF::endsWithIgnoringASCIICase(*this, suffix);
}

bool StringImpl::endsWith(UChar character) const
{
    return m_length && (*this)[m_length - 1] == character;
}

bool StringImpl::endsWith(const char* matchString, unsigned matchLength, bool caseSensitive) const
{
    ASSERT(matchLength);
    if (matchLength > length())
        return false;
    unsigned startOffset = length() - matchLength;
    return equalInner(this, startOffset, matchString, matchLength, caseSensitive);
}

bool StringImpl::hasInfixEndingAt(const StringImpl& matchString, unsigned endOffset) const
{
    if (endOffset < matchString.length())
        return false;
    return equalInner(*this, endOffset - matchString.length(), matchString);
}

Ref<StringImpl> StringImpl::replace(UChar oldC, UChar newC)
{
    if (oldC == newC)
        return *this;
    unsigned i;
    for (i = 0; i != m_length; ++i) {
        UChar c = is8Bit() ? m_data8[i] : m_data16[i];
        if (c == oldC)
            break;
    }
    if (i == m_length)
        return *this;

    if (is8Bit()) {
        if (oldC > 0xff)
            // Looking for a 16 bit char in an 8 bit string, we're done.
            return *this;

        if (newC <= 0xff) {
            LChar* data;
            LChar oldChar = static_cast<LChar>(oldC);
            LChar newChar = static_cast<LChar>(newC);

            auto newImpl = createUninitializedInternalNonEmpty(m_length, data);

            for (i = 0; i != m_length; ++i) {
                LChar ch = m_data8[i];
                if (ch == oldChar)
                    ch = newChar;
                data[i] = ch;
            }
            return newImpl;
        }

        // There is the possibility we need to up convert from 8 to 16 bit,
        // create a 16 bit string for the result.
        UChar* data;
        auto newImpl = createUninitializedInternalNonEmpty(m_length, data);

        for (i = 0; i != m_length; ++i) {
            UChar ch = m_data8[i];
            if (ch == oldC)
                ch = newC;
            data[i] = ch;
        }

        return newImpl;
    }

    UChar* data;
    auto newImpl = createUninitializedInternalNonEmpty(m_length, data);

    for (i = 0; i != m_length; ++i) {
        UChar ch = m_data16[i];
        if (ch == oldC)
            ch = newC;
        data[i] = ch;
    }
    return newImpl;
}

Ref<StringImpl> StringImpl::replace(unsigned position, unsigned lengthToReplace, StringImpl* str)
{
    position = std::min(position, length());
    lengthToReplace = std::min(lengthToReplace, length() - position);
    unsigned lengthToInsert = str ? str->length() : 0;
    if (!lengthToReplace && !lengthToInsert)
        return *this;

    if ((length() - lengthToReplace) >= (std::numeric_limits<unsigned>::max() - lengthToInsert))
        CRASH();

    if (is8Bit() && (!str || str->is8Bit())) {
        LChar* data;
        auto newImpl = createUninitialized(length() - lengthToReplace + lengthToInsert, data);
        memcpy(data, m_data8, position * sizeof(LChar));
        if (str)
            memcpy(data + position, str->m_data8, lengthToInsert * sizeof(LChar));
        memcpy(data + position + lengthToInsert, m_data8 + position + lengthToReplace,
               (length() - position - lengthToReplace) * sizeof(LChar));
        return newImpl;
    }
    UChar* data;
    auto newImpl = createUninitialized(length() - lengthToReplace + lengthToInsert, data);
    if (is8Bit())
        for (unsigned i = 0; i < position; ++i)
            data[i] = m_data8[i];
    else
        memcpy(data, m_data16, position * sizeof(UChar));
    if (str) {
        if (str->is8Bit())
            for (unsigned i = 0; i < lengthToInsert; ++i)
                data[i + position] = str->m_data8[i];
        else
            memcpy(data + position, str->m_data16, lengthToInsert * sizeof(UChar));
    }
    if (is8Bit()) {
        for (unsigned i = 0; i < length() - position - lengthToReplace; ++i)
            data[i + position + lengthToInsert] = m_data8[i + position + lengthToReplace];
    } else {
        memcpy(data + position + lengthToInsert, characters16() + position + lengthToReplace,
            (length() - position - lengthToReplace) * sizeof(UChar));
    }
    return newImpl;
}

Ref<StringImpl> StringImpl::replace(UChar pattern, StringImpl* replacement)
{
    if (!replacement)
        return *this;

    if (replacement->is8Bit())
        return replace(pattern, replacement->m_data8, replacement->length());

    return replace(pattern, replacement->m_data16, replacement->length());
}

Ref<StringImpl> StringImpl::replace(UChar pattern, const LChar* replacement, unsigned repStrLength)
{
    ASSERT(replacement);

    size_t srcSegmentStart = 0;
    unsigned matchCount = 0;

    // Count the matches.
    while ((srcSegmentStart = find(pattern, srcSegmentStart)) != notFound) {
        ++matchCount;
        ++srcSegmentStart;
    }

    // If we have 0 matches then we don't have to do any more work.
    if (!matchCount)
        return *this;

    if (repStrLength && matchCount > std::numeric_limits<unsigned>::max() / repStrLength)
        CRASH();

    unsigned replaceSize = matchCount * repStrLength;
    unsigned newSize = m_length - matchCount;
    if (newSize >= (std::numeric_limits<unsigned>::max() - replaceSize))
        CRASH();

    newSize += replaceSize;

    // Construct the new data.
    size_t srcSegmentEnd;
    unsigned srcSegmentLength;
    srcSegmentStart = 0;
    unsigned dstOffset = 0;

    if (is8Bit()) {
        LChar* data;
        auto newImpl = createUninitialized(newSize, data);

        while ((srcSegmentEnd = find(pattern, srcSegmentStart)) != notFound) {
            srcSegmentLength = srcSegmentEnd - srcSegmentStart;
            memcpy(data + dstOffset, m_data8 + srcSegmentStart, srcSegmentLength * sizeof(LChar));
            dstOffset += srcSegmentLength;
            memcpy(data + dstOffset, replacement, repStrLength * sizeof(LChar));
            dstOffset += repStrLength;
            srcSegmentStart = srcSegmentEnd + 1;
        }

        srcSegmentLength = m_length - srcSegmentStart;
        memcpy(data + dstOffset, m_data8 + srcSegmentStart, srcSegmentLength * sizeof(LChar));

        ASSERT(dstOffset + srcSegmentLength == newImpl.get().length());

        return newImpl;
    }

    UChar* data;
    auto newImpl = createUninitialized(newSize, data);

    while ((srcSegmentEnd = find(pattern, srcSegmentStart)) != notFound) {
        srcSegmentLength = srcSegmentEnd - srcSegmentStart;
        memcpy(data + dstOffset, m_data16 + srcSegmentStart, srcSegmentLength * sizeof(UChar));

        dstOffset += srcSegmentLength;
        for (unsigned i = 0; i < repStrLength; ++i)
            data[i + dstOffset] = replacement[i];

        dstOffset += repStrLength;
        srcSegmentStart = srcSegmentEnd + 1;
    }

    srcSegmentLength = m_length - srcSegmentStart;
    memcpy(data + dstOffset, m_data16 + srcSegmentStart, srcSegmentLength * sizeof(UChar));

    ASSERT(dstOffset + srcSegmentLength == newImpl.get().length());

    return newImpl;
}

Ref<StringImpl> StringImpl::replace(UChar pattern, const UChar* replacement, unsigned repStrLength)
{
    ASSERT(replacement);

    size_t srcSegmentStart = 0;
    unsigned matchCount = 0;

    // Count the matches.
    while ((srcSegmentStart = find(pattern, srcSegmentStart)) != notFound) {
        ++matchCount;
        ++srcSegmentStart;
    }

    // If we have 0 matches then we don't have to do any more work.
    if (!matchCount)
        return *this;

    if (repStrLength && matchCount > std::numeric_limits<unsigned>::max() / repStrLength)
        CRASH();

    unsigned replaceSize = matchCount * repStrLength;
    unsigned newSize = m_length - matchCount;
    if (newSize >= (std::numeric_limits<unsigned>::max() - replaceSize))
        CRASH();

    newSize += replaceSize;

    // Construct the new data.
    size_t srcSegmentEnd;
    unsigned srcSegmentLength;
    srcSegmentStart = 0;
    unsigned dstOffset = 0;

    if (is8Bit()) {
        UChar* data;
        auto newImpl = createUninitialized(newSize, data);

        while ((srcSegmentEnd = find(pattern, srcSegmentStart)) != notFound) {
            srcSegmentLength = srcSegmentEnd - srcSegmentStart;
            for (unsigned i = 0; i < srcSegmentLength; ++i)
                data[i + dstOffset] = m_data8[i + srcSegmentStart];

            dstOffset += srcSegmentLength;
            memcpy(data + dstOffset, replacement, repStrLength * sizeof(UChar));

            dstOffset += repStrLength;
            srcSegmentStart = srcSegmentEnd + 1;
        }

        srcSegmentLength = m_length - srcSegmentStart;
        for (unsigned i = 0; i < srcSegmentLength; ++i)
            data[i + dstOffset] = m_data8[i + srcSegmentStart];

        ASSERT(dstOffset + srcSegmentLength == newImpl.get().length());

        return newImpl;
    }

    UChar* data;
    auto newImpl = createUninitialized(newSize, data);

    while ((srcSegmentEnd = find(pattern, srcSegmentStart)) != notFound) {
        srcSegmentLength = srcSegmentEnd - srcSegmentStart;
        memcpy(data + dstOffset, m_data16 + srcSegmentStart, srcSegmentLength * sizeof(UChar));

        dstOffset += srcSegmentLength;
        memcpy(data + dstOffset, replacement, repStrLength * sizeof(UChar));

        dstOffset += repStrLength;
        srcSegmentStart = srcSegmentEnd + 1;
    }

    srcSegmentLength = m_length - srcSegmentStart;
    memcpy(data + dstOffset, m_data16 + srcSegmentStart, srcSegmentLength * sizeof(UChar));

    ASSERT(dstOffset + srcSegmentLength == newImpl.get().length());

    return newImpl;
}

Ref<StringImpl> StringImpl::replace(StringImpl* pattern, StringImpl* replacement)
{
    if (!pattern || !replacement)
        return *this;

    unsigned patternLength = pattern->length();
    if (!patternLength)
        return *this;
        
    unsigned repStrLength = replacement->length();
    size_t srcSegmentStart = 0;
    unsigned matchCount = 0;
    
    // Count the matches.
    while ((srcSegmentStart = find(pattern, srcSegmentStart)) != notFound) {
        ++matchCount;
        srcSegmentStart += patternLength;
    }
    
    // If we have 0 matches, we don't have to do any more work
    if (!matchCount)
        return *this;
    
    unsigned newSize = m_length - matchCount * patternLength;
    if (repStrLength && matchCount > std::numeric_limits<unsigned>::max() / repStrLength)
        CRASH();

    if (newSize > (std::numeric_limits<unsigned>::max() - matchCount * repStrLength))
        CRASH();

    newSize += matchCount * repStrLength;

    
    // Construct the new data
    size_t srcSegmentEnd;
    unsigned srcSegmentLength;
    srcSegmentStart = 0;
    unsigned dstOffset = 0;
    bool srcIs8Bit = is8Bit();
    bool replacementIs8Bit = replacement->is8Bit();
    
    // There are 4 cases:
    // 1. This and replacement are both 8 bit.
    // 2. This and replacement are both 16 bit.
    // 3. This is 8 bit and replacement is 16 bit.
    // 4. This is 16 bit and replacement is 8 bit.
    if (srcIs8Bit && replacementIs8Bit) {
        // Case 1
        LChar* data;
        auto newImpl = createUninitialized(newSize, data);
        while ((srcSegmentEnd = find(pattern, srcSegmentStart)) != notFound) {
            srcSegmentLength = srcSegmentEnd - srcSegmentStart;
            memcpy(data + dstOffset, m_data8 + srcSegmentStart, srcSegmentLength * sizeof(LChar));
            dstOffset += srcSegmentLength;
            memcpy(data + dstOffset, replacement->m_data8, repStrLength * sizeof(LChar));
            dstOffset += repStrLength;
            srcSegmentStart = srcSegmentEnd + patternLength;
        }

        srcSegmentLength = m_length - srcSegmentStart;
        memcpy(data + dstOffset, m_data8 + srcSegmentStart, srcSegmentLength * sizeof(LChar));

        ASSERT(dstOffset + srcSegmentLength == newImpl.get().length());

        return newImpl;
    }

    UChar* data;
    auto newImpl = createUninitialized(newSize, data);
    while ((srcSegmentEnd = find(pattern, srcSegmentStart)) != notFound) {
        srcSegmentLength = srcSegmentEnd - srcSegmentStart;
        if (srcIs8Bit) {
            // Case 3.
            for (unsigned i = 0; i < srcSegmentLength; ++i)
                data[i + dstOffset] = m_data8[i + srcSegmentStart];
        } else {
            // Case 2 & 4.
            memcpy(data + dstOffset, m_data16 + srcSegmentStart, srcSegmentLength * sizeof(UChar));
        }
        dstOffset += srcSegmentLength;
        if (replacementIs8Bit) {
            // Cases 2 & 3.
            for (unsigned i = 0; i < repStrLength; ++i)
                data[i + dstOffset] = replacement->m_data8[i];
        } else {
            // Case 4
            memcpy(data + dstOffset, replacement->m_data16, repStrLength * sizeof(UChar));
        }
        dstOffset += repStrLength;
        srcSegmentStart = srcSegmentEnd + patternLength;
    }

    srcSegmentLength = m_length - srcSegmentStart;
    if (srcIs8Bit) {
        // Case 3.
        for (unsigned i = 0; i < srcSegmentLength; ++i)
            data[i + dstOffset] = m_data8[i + srcSegmentStart];
    } else {
        // Cases 2 & 4.
        memcpy(data + dstOffset, m_data16 + srcSegmentStart, srcSegmentLength * sizeof(UChar));
    }

    ASSERT(dstOffset + srcSegmentLength == newImpl.get().length());

    return newImpl;
}

bool equal(const StringImpl* a, const StringImpl* b)
{
    return equalCommon(a, b);
}

template <typename CharType>
inline bool equalInternal(const StringImpl* a, const CharType* b, unsigned length)
{
    if (!a)
        return !b;
    if (!b)
        return false;

    if (a->length() != length)
        return false;
    if (a->is8Bit())
        return equal(a->characters8(), b, length);
    return equal(a->characters16(), b, length);
}

bool equal(const StringImpl* a, const LChar* b, unsigned length)
{
    return equalInternal(a, b, length);
}

bool equal(const StringImpl* a, const UChar* b, unsigned length)
{
    return equalInternal(a, b, length);
}

bool equal(const StringImpl* a, const LChar* b)
{
    if (!a)
        return !b;
    if (!b)
        return !a;

    unsigned length = a->length();

    if (a->is8Bit()) {
        const LChar* aPtr = a->characters8();
        for (unsigned i = 0; i != length; ++i) {
            LChar bc = b[i];
            LChar ac = aPtr[i];
            if (!bc)
                return false;
            if (ac != bc)
                return false;
        }

        return !b[length];
    }

    const UChar* aPtr = a->characters16();
    for (unsigned i = 0; i != length; ++i) {
        LChar bc = b[i];
        if (!bc)
            return false;
        if (aPtr[i] != bc)
            return false;
    }

    return !b[length];
}

bool equal(const StringImpl& a, const StringImpl& b)
{
    return equalCommon(a, b);
}

bool equalIgnoringNullity(StringImpl* a, StringImpl* b)
{
    if (!a && b && !b->length())
        return true;
    if (!b && a && !a->length())
        return true;
    return equal(a, b);
}

bool equalIgnoringASCIICase(const StringImpl* a, const StringImpl* b)
{
    if (a == b)
        return true;
    if (!a || !b)
        return false;
    return equalIgnoringASCIICaseCommon(*a, *b);
}

bool equalIgnoringASCIICaseNonNull(const StringImpl* a, const StringImpl* b)
{
    ASSERT(a);
    ASSERT(b);
    return equalIgnoringASCIICase(*a, *b);
}

UCharDirection StringImpl::defaultWritingDirection(bool* hasStrongDirectionality)
{
    for (unsigned i = 0; i < m_length; ++i) {
        UCharDirection charDirection = u_charDirection(is8Bit() ? m_data8[i] : m_data16[i]);
        if (charDirection == U_LEFT_TO_RIGHT) {
            if (hasStrongDirectionality)
                *hasStrongDirectionality = true;
            return U_LEFT_TO_RIGHT;
        }
        if (charDirection == U_RIGHT_TO_LEFT || charDirection == U_RIGHT_TO_LEFT_ARABIC) {
            if (hasStrongDirectionality)
                *hasStrongDirectionality = true;
            return U_RIGHT_TO_LEFT;
        }
    }
    if (hasStrongDirectionality)
        *hasStrongDirectionality = false;
    return U_LEFT_TO_RIGHT;
}

Ref<StringImpl> StringImpl::adopt(StringBuffer<LChar>&& buffer)
{
    unsigned length = buffer.length();
    if (!length)
        return *empty();
    return adoptRef(*new StringImpl(buffer.release(), length));
}

Ref<StringImpl> StringImpl::adopt(StringBuffer<UChar>&& buffer)
{
    unsigned length = buffer.length();
    if (!length)
        return *empty();
    return adoptRef(*new StringImpl(buffer.release(), length));
}

size_t StringImpl::sizeInBytes() const
{
    // FIXME: support substrings
    size_t size = length();
    if (!is8Bit())
        size *= 2;
    return size + sizeof(*this);
}

// Helper to write a three-byte UTF-8 code point to the buffer, caller must check room is available.
static inline void putUTF8Triple(char*& buffer, UChar ch)
{
    ASSERT(ch >= 0x0800);
    *buffer++ = static_cast<char>(((ch >> 12) & 0x0F) | 0xE0);
    *buffer++ = static_cast<char>(((ch >> 6) & 0x3F) | 0x80);
    *buffer++ = static_cast<char>((ch & 0x3F) | 0x80);
}

bool StringImpl::utf8Impl(const UChar* characters, unsigned length, char*& buffer, size_t bufferSize, ConversionMode mode)
{
    if (mode == StrictConversionReplacingUnpairedSurrogatesWithFFFD) {
        const UChar* charactersEnd = characters + length;
        char* bufferEnd = buffer + bufferSize;
        while (characters < charactersEnd) {
            // Use strict conversion to detect unpaired surrogates.
            ConversionResult result = convertUTF16ToUTF8(&characters, charactersEnd, &buffer, bufferEnd, true);
            ASSERT(result != targetExhausted);
            // Conversion fails when there is an unpaired surrogate.
            // Put replacement character (U+FFFD) instead of the unpaired surrogate.
            if (result != conversionOK) {
                ASSERT((0xD800 <= *characters && *characters <= 0xDFFF));
                // There should be room left, since one UChar hasn't been converted.
                ASSERT((buffer + 3) <= bufferEnd);
                putUTF8Triple(buffer, replacementCharacter);
                ++characters;
            }
        }
    } else {
        bool strict = mode == StrictConversion;
        const UChar* originalCharacters = characters;
        ConversionResult result = convertUTF16ToUTF8(&characters, characters + length, &buffer, buffer + bufferSize, strict);
        ASSERT(result != targetExhausted); // (length * 3) should be sufficient for any conversion

        // Only produced from strict conversion.
        if (result == sourceIllegal) {
            ASSERT(strict);
            return false;
        }

        // Check for an unconverted high surrogate.
        if (result == sourceExhausted) {
            if (strict)
                return false;
            // This should be one unpaired high surrogate. Treat it the same
            // was as an unpaired high surrogate would have been handled in
            // the middle of a string with non-strict conversion - which is
            // to say, simply encode it to UTF-8.
            ASSERT_UNUSED(
                originalCharacters, (characters + 1) == (originalCharacters + length));
            ASSERT((*characters >= 0xD800) && (*characters <= 0xDBFF));
            // There should be room left, since one UChar hasn't been converted.
            ASSERT((buffer + 3) <= (buffer + bufferSize));
            putUTF8Triple(buffer, *characters);
        }
    }
    
    return true;
}

CString StringImpl::utf8ForCharacters(const LChar* characters, unsigned length)
{
    if (!length)
        return CString("", 0);
    if (length > std::numeric_limits<unsigned>::max() / 3)
        return CString();
    Vector<char, 1024> bufferVector(length * 3);
    char* buffer = bufferVector.data();
    const LChar* source = characters;
    ConversionResult result = convertLatin1ToUTF8(&source, source + length, &buffer, buffer + bufferVector.size());
    ASSERT_UNUSED(result, result != targetExhausted); // (length * 3) should be sufficient for any conversion
    return CString(bufferVector.data(), buffer - bufferVector.data());
}

CString StringImpl::utf8ForCharacters(const UChar* characters, unsigned length, ConversionMode mode)
{
    if (!length)
        return CString("", 0);
    if (length > std::numeric_limits<unsigned>::max() / 3)
        return CString();
    Vector<char, 1024> bufferVector(length * 3);
    char* buffer = bufferVector.data();
    if (!utf8Impl(characters, length, buffer, bufferVector.size(), mode))
        return CString();
    return CString(bufferVector.data(), buffer - bufferVector.data());
}

CString StringImpl::utf8ForRange(unsigned offset, unsigned length, ConversionMode mode) const
{
    ASSERT(offset <= this->length());
    ASSERT(offset + length <= this->length());
    
    if (!length)
        return CString("", 0);

    // Allocate a buffer big enough to hold all the characters
    // (an individual UTF-16 UChar can only expand to 3 UTF-8 bytes).
    // Optimization ideas, if we find this function is hot:
    //  * We could speculatively create a CStringBuffer to contain 'length' 
    //    characters, and resize if necessary (i.e. if the buffer contains
    //    non-ascii characters). (Alternatively, scan the buffer first for
    //    ascii characters, so we know this will be sufficient).
    //  * We could allocate a CStringBuffer with an appropriate size to
    //    have a good chance of being able to write the string into the
    //    buffer without reallocing (say, 1.5 x length).
    if (length > std::numeric_limits<unsigned>::max() / 3)
        return CString();
    Vector<char, 1024> bufferVector(length * 3);

    char* buffer = bufferVector.data();

    if (is8Bit()) {
        const LChar* characters = this->characters8() + offset;

        ConversionResult result = convertLatin1ToUTF8(&characters, characters + length, &buffer, buffer + bufferVector.size());
        ASSERT_UNUSED(result, result != targetExhausted); // (length * 3) should be sufficient for any conversion
    } else {
        if (!utf8Impl(this->characters16() + offset, length, buffer, bufferVector.size(), mode))
            return CString();
    }

    return CString(bufferVector.data(), buffer - bufferVector.data());
}

CString StringImpl::utf8(ConversionMode mode) const
{
    return utf8ForRange(0, length(), mode);
}

bool equalIgnoringNullity(const UChar* a, size_t aLength, StringImpl* b)
{
    if (!b)
        return !aLength;
    if (aLength != b->length())
        return false;
    if (b->is8Bit()) {
        const LChar* bCharacters = b->characters8();
        for (unsigned i = 0; i < aLength; ++i) {
            if (a[i] != bCharacters[i])
                return false;
        }
        return true;
    }
    return !memcmp(a, b->characters16(), b->length() * sizeof(UChar));
}

} // namespace WTF
