/*
 * (C) 1999 Lars Knoll (knoll@kde.org)
 * Copyright (C) 2004-2016 Apple Inc. All rights reserved.
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
#include "TextBreakIterator.h"

#include "LineBreakIteratorPoolICU.h"
#include "TextBreakIteratorInternalICU.h"
#include "UTextProviderLatin1.h"
#include "UTextProviderUTF16.h"
#include <atomic>
#include <mutex>
#include <unicode/ubrk.h>
#include <wtf/text/StringBuilder.h>

// FIXME: This needs a better name
#define ADDITIONAL_EMOJI_SUPPORT (PLATFORM(IOS) || (PLATFORM(MAC) && __MAC_OS_X_VERSION_MIN_REQUIRED >= 101100))

namespace WTF {

#if !PLATFORM(MAC) && !PLATFORM(IOS)

static Variant<TextBreakIteratorICU, TextBreakIteratorPlatform> mapModeToBackingIterator(StringView string, TextBreakIterator::Mode mode, const AtomicString& locale)
{
    switch (mode) {
    case TextBreakIterator::Mode::Line:
        return TextBreakIteratorICU(string, TextBreakIteratorICU::Mode::Line, locale.string().utf8().data());
    case TextBreakIterator::Mode::Caret:
        return TextBreakIteratorICU(string, TextBreakIteratorICU::Mode::Character, locale.string().utf8().data());
    case TextBreakIterator::Mode::Delete:
        return TextBreakIteratorICU(string, TextBreakIteratorICU::Mode::Character, locale.string().utf8().data());
    default:
        ASSERT_NOT_REACHED();
        return TextBreakIteratorICU(string, TextBreakIteratorICU::Mode::Character, locale.string().utf8().data());
    }
}

TextBreakIterator::TextBreakIterator(StringView string, Mode mode, const AtomicString& locale)
    : m_backing(mapModeToBackingIterator(string, mode, locale))
    , m_mode(mode)
    , m_locale(locale)
{
}

#endif

// Iterator initialization

static UBreakIterator* initializeIterator(UBreakIteratorType type, const char* locale = currentTextBreakLocaleID())
{
    UErrorCode openStatus = U_ZERO_ERROR;
    UBreakIterator* iterator = ubrk_open(type, locale, 0, 0, &openStatus);
    ASSERT_WITH_MESSAGE(U_SUCCESS(openStatus), "ICU could not open a break iterator: %s (%d)", u_errorName(openStatus), openStatus);
    return iterator;
}

// Iterator text setting

static UBreakIterator* setTextForIterator(UBreakIterator& iterator, StringView string)
{
    if (string.is8Bit()) {
        UTextWithBuffer textLocal;
        textLocal.text = UTEXT_INITIALIZER;
        textLocal.text.extraSize = sizeof(textLocal.buffer);
        textLocal.text.pExtra = textLocal.buffer;

        UErrorCode openStatus = U_ZERO_ERROR;
        UText* text = openLatin1UTextProvider(&textLocal, string.characters8(), string.length(), &openStatus);
        if (U_FAILURE(openStatus)) {
            LOG_ERROR("uTextOpenLatin1 failed with status %d", openStatus);
            return nullptr;
        }

        UErrorCode setTextStatus = U_ZERO_ERROR;
        ubrk_setUText(&iterator, text, &setTextStatus);
        if (U_FAILURE(setTextStatus)) {
            LOG_ERROR("ubrk_setUText failed with status %d", setTextStatus);
            return nullptr;
        }

        utext_close(text);
    } else {
        UErrorCode setTextStatus = U_ZERO_ERROR;
        ubrk_setText(&iterator, string.characters16(), string.length(), &setTextStatus);
        if (U_FAILURE(setTextStatus))
            return nullptr;
    }

    return &iterator;
}

static UBreakIterator* setContextAwareTextForIterator(UBreakIterator& iterator, StringView string, const UChar* priorContext, unsigned priorContextLength)
{
    if (string.is8Bit()) {
        UTextWithBuffer textLocal;
        textLocal.text = UTEXT_INITIALIZER;
        textLocal.text.extraSize = sizeof(textLocal.buffer);
        textLocal.text.pExtra = textLocal.buffer;

        UErrorCode openStatus = U_ZERO_ERROR;
        UText* text = openLatin1ContextAwareUTextProvider(&textLocal, string.characters8(), string.length(), priorContext, priorContextLength, &openStatus);
        if (U_FAILURE(openStatus)) {
            LOG_ERROR("openLatin1ContextAwareUTextProvider failed with status %d", openStatus);
            return nullptr;
        }

        UErrorCode setTextStatus = U_ZERO_ERROR;
        ubrk_setUText(&iterator, text, &setTextStatus);
        if (U_FAILURE(setTextStatus)) {
            LOG_ERROR("ubrk_setUText failed with status %d", setTextStatus);
            return nullptr;
        }

        utext_close(text);
    } else {
        UText textLocal = UTEXT_INITIALIZER;

        UErrorCode openStatus = U_ZERO_ERROR;
        UText* text = openUTF16ContextAwareUTextProvider(&textLocal, string.characters16(), string.length(), priorContext, priorContextLength, &openStatus);
        if (U_FAILURE(openStatus)) {
            LOG_ERROR("openUTF16ContextAwareUTextProvider failed with status %d", openStatus);
            return 0;
        }

        UErrorCode setTextStatus = U_ZERO_ERROR;
        ubrk_setUText(&iterator, text, &setTextStatus);
        if (U_FAILURE(setTextStatus)) {
            LOG_ERROR("ubrk_setUText failed with status %d", setTextStatus);
            return nullptr;
        }

        utext_close(text);
    }

    return &iterator;
}


// Static iterators

UBreakIterator* wordBreakIterator(StringView string)
{
    static UBreakIterator* staticWordBreakIterator = initializeIterator(UBRK_WORD);
    if (!staticWordBreakIterator)
        return nullptr;

    return setTextForIterator(*staticWordBreakIterator, string);
}

UBreakIterator* sentenceBreakIterator(StringView string)
{
    static UBreakIterator* staticSentenceBreakIterator = initializeIterator(UBRK_SENTENCE);
    if (!staticSentenceBreakIterator)
        return nullptr;

    return setTextForIterator(*staticSentenceBreakIterator, string);
}

UBreakIterator* acquireLineBreakIterator(StringView string, const AtomicString& locale, const UChar* priorContext, unsigned priorContextLength, LineBreakIteratorMode mode)
{
    UBreakIterator* iterator = LineBreakIteratorPool::sharedPool().take(locale, mode);
    if (!iterator)
        return nullptr;

    return setContextAwareTextForIterator(*iterator, string, priorContext, priorContextLength);
}

void releaseLineBreakIterator(UBreakIterator* iterator)
{
    ASSERT_ARG(iterator, iterator);

    LineBreakIteratorPool::sharedPool().put(iterator);
}

UBreakIterator* openLineBreakIterator(const AtomicString& locale)
{
    bool localeIsEmpty = locale.isEmpty();
    UErrorCode openStatus = U_ZERO_ERROR;
    UBreakIterator* ubrkIter = ubrk_open(UBRK_LINE, localeIsEmpty ? currentTextBreakLocaleID() : locale.string().utf8().data(), 0, 0, &openStatus);
    // locale comes from a web page and it can be invalid, leading ICU
    // to fail, in which case we fall back to the default locale.
    if (!localeIsEmpty && U_FAILURE(openStatus)) {
        openStatus = U_ZERO_ERROR;
        ubrkIter = ubrk_open(UBRK_LINE, currentTextBreakLocaleID(), 0, 0, &openStatus);
    }

    if (U_FAILURE(openStatus)) {
        LOG_ERROR("ubrk_open failed with status %d", openStatus);
        return nullptr;
    }

    return ubrkIter;
}

void closeLineBreakIterator(UBreakIterator*& iterator)
{
    UBreakIterator* ubrkIter = iterator;
    ASSERT(ubrkIter);
    ubrk_close(ubrkIter);
    iterator = nullptr;
}

static std::atomic<UBreakIterator*> nonSharedCharacterBreakIterator = ATOMIC_VAR_INIT(nullptr);

static inline UBreakIterator* getNonSharedCharacterBreakIterator()
{
    if (auto *res = nonSharedCharacterBreakIterator.exchange(nullptr, std::memory_order_acquire))
        return res;
    return initializeIterator(UBRK_CHARACTER);
}

static inline void cacheNonSharedCharacterBreakIterator(UBreakIterator* cacheMe)
{
    if (auto *old = nonSharedCharacterBreakIterator.exchange(cacheMe, std::memory_order_release))
        ubrk_close(old);
}

NonSharedCharacterBreakIterator::NonSharedCharacterBreakIterator(StringView string)
{
    if ((m_iterator = getNonSharedCharacterBreakIterator()))
        m_iterator = setTextForIterator(*m_iterator, string);
}

NonSharedCharacterBreakIterator::~NonSharedCharacterBreakIterator()
{
    if (m_iterator)
        cacheNonSharedCharacterBreakIterator(m_iterator);
}

NonSharedCharacterBreakIterator::NonSharedCharacterBreakIterator(NonSharedCharacterBreakIterator&& other)
    : m_iterator(nullptr)
{
    std::swap(m_iterator, other.m_iterator);
}

// Iterator implemenation.

bool isWordTextBreak(UBreakIterator* iterator)
{
    int ruleStatus = ubrk_getRuleStatus(iterator);
    return ruleStatus != UBRK_WORD_NONE;
}

unsigned numGraphemeClusters(StringView string)
{
    unsigned stringLength = string.length();
    
    if (!stringLength)
        return 0;

    // The only Latin-1 Extended Grapheme Cluster is CRLF.
    if (string.is8Bit()) {
        auto* characters = string.characters8();
        unsigned numCRLF = 0;
        for (unsigned i = 1; i < stringLength; ++i)
            numCRLF += characters[i - 1] == '\r' && characters[i] == '\n';
        return stringLength - numCRLF;
    }

    NonSharedCharacterBreakIterator iterator { string };
    if (!iterator) {
        ASSERT_NOT_REACHED();
        return stringLength;
    }

    unsigned numGraphemeClusters = 0;
    while (ubrk_next(iterator) != UBRK_DONE)
        ++numGraphemeClusters;
    return numGraphemeClusters;
}

unsigned numCharactersInGraphemeClusters(StringView string, unsigned numGraphemeClusters)
{
    unsigned stringLength = string.length();

    if (stringLength <= numGraphemeClusters)
        return stringLength;

    // The only Latin-1 Extended Grapheme Cluster is CRLF.
    if (string.is8Bit()) {
        auto* characters = string.characters8();
        unsigned i, j;
        for (i = 0, j = 0; i < numGraphemeClusters && j + 1 < stringLength; ++i, ++j)
            j += characters[j] == '\r' && characters[j + 1] == '\n';
        return j + (i < numGraphemeClusters);
    }

    NonSharedCharacterBreakIterator iterator { string };
    if (!iterator) {
        ASSERT_NOT_REACHED();
        return stringLength;
    }

    for (unsigned i = 0; i < numGraphemeClusters; ++i) {
        if (ubrk_next(iterator) == UBRK_DONE)
            return stringLength;
    }
    return ubrk_current(iterator);
}

} // namespace WTF
