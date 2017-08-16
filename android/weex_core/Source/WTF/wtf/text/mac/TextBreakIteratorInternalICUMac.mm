/*
 * Copyright (C) 2007, 2009 Apple Inc. All rights reserved.
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
#include "TextBreakIteratorInternalICU.h"

#include "TextBreakIterator.h"
#include <wtf/RetainPtr.h>

namespace WTF {

static Variant<TextBreakIteratorICU, TextBreakIteratorPlatform> mapModeToBackingIterator(StringView string, TextBreakIterator::Mode mode, const AtomicString& locale)
{
    switch (mode) {
    case TextBreakIterator::Mode::Line:
        return TextBreakIteratorICU(string, TextBreakIteratorICU::Mode::Line, locale.string().utf8().data());
    case TextBreakIterator::Mode::Caret:
#if USE_ICU_CARET_ITERATOR
        return TextBreakIteratorICU(string, TextBreakIteratorICU::Mode::Caret, locale.string().utf8().data());
#else
        return TextBreakIteratorCF(string, TextBreakIteratorCF::Mode::Caret);
#endif
    case TextBreakIterator::Mode::Delete:
        return TextBreakIteratorCF(string, TextBreakIteratorCF::Mode::Delete);
    }
}

TextBreakIterator::TextBreakIterator(StringView string, Mode mode, const AtomicString& locale)
    : m_backing(mapModeToBackingIterator(string, mode, locale))
    , m_mode(mode)
    , m_locale(locale)
{
}

static const int maxLocaleStringLength = 32;

static inline RetainPtr<CFStringRef> textBreakLocalePreference()
{
    RetainPtr<CFPropertyListRef> locale = adoptCF(CFPreferencesCopyValue(CFSTR("AppleTextBreakLocale"),
        kCFPreferencesAnyApplication, kCFPreferencesCurrentUser, kCFPreferencesAnyHost));
    if (!locale || CFGetTypeID(locale.get()) != CFStringGetTypeID())
        return nullptr;
    return static_cast<CFStringRef>(locale.get());
}

static RetainPtr<CFStringRef> topLanguagePreference()
{
    RetainPtr<CFArrayRef> languagesArray = adoptCF(CFLocaleCopyPreferredLanguages());
    if (!languagesArray)
        return nullptr;
    if (!CFArrayGetCount(languagesArray.get()))
        return nullptr;
    return static_cast<CFStringRef>(CFArrayGetValueAtIndex(languagesArray.get(), 0));
}

static void getLocale(CFStringRef locale, char localeStringBuffer[maxLocaleStringLength])
{
    // Empty string means "root locale", and that is what we use if we can't get a preference.
    localeStringBuffer[0] = 0;
    if (!locale)
        return;
    CFStringGetCString(locale, localeStringBuffer, maxLocaleStringLength, kCFStringEncodingASCII);
}

static void getSearchLocale(char localeStringBuffer[maxLocaleStringLength])
{
    getLocale(topLanguagePreference().get(), localeStringBuffer);
}

const char* currentSearchLocaleID()
{
    static char localeStringBuffer[maxLocaleStringLength];
    static bool gotSearchLocale = false;
    if (!gotSearchLocale) {
        getSearchLocale(localeStringBuffer);
        gotSearchLocale = true;
    }
    return localeStringBuffer;
}

static void getTextBreakLocale(char localeStringBuffer[maxLocaleStringLength])
{
    // If there is no text break locale, use the top language preference.
    RetainPtr<CFStringRef> locale = textBreakLocalePreference();
    if (locale) {
        if (RetainPtr<CFStringRef> canonicalLocale = adoptCF(CFLocaleCreateCanonicalLanguageIdentifierFromString(kCFAllocatorDefault, locale.get())))
            locale = canonicalLocale;
    } else
        locale = topLanguagePreference();
    getLocale(locale.get(), localeStringBuffer);
}

const char* currentTextBreakLocaleID()
{
    static char localeStringBuffer[maxLocaleStringLength];
    static bool gotTextBreakLocale = false;
    if (!gotTextBreakLocale) {
        getTextBreakLocale(localeStringBuffer);
        gotTextBreakLocale = true;
    }
    return localeStringBuffer;
}

}
