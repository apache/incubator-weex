/*
 * Copyright (C) 2007, 2016 Apple Inc.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 */

#include "config.h"
#include "PlatformUserPreferredLanguages.h"

#include <mutex>
#include <windows.h>
#include <wtf/Lock.h>
#include <wtf/Vector.h>
#include <wtf/text/WTFString.h>

namespace WTF {

static StaticLock platformLanguageMutex;

void setPlatformUserPreferredLanguagesChangedCallback(void (*)()) { }

static String localeInfo(LCTYPE localeType, const String& fallback)
{
    LANGID langID = GetUserDefaultUILanguage();
    int localeChars = GetLocaleInfo(langID, localeType, 0, 0);
    if (!localeChars)
        return fallback;
    UChar* localeNameBuf;
    String localeName = String::createUninitialized(localeChars, localeNameBuf);
    localeChars = GetLocaleInfo(langID, localeType, localeNameBuf, localeChars);
    if (!localeChars)
        return fallback;
    if (localeName.isEmpty())
        return fallback;

    localeName.truncate(localeName.length() - 1);
    return localeName;
}

static String platformLanguage()
{
    std::lock_guard<StaticLock> lock(platformLanguageMutex);

    static String computedDefaultLanguage;
    if (!computedDefaultLanguage.isEmpty())
        return computedDefaultLanguage.isolatedCopy();

    String languageName = localeInfo(LOCALE_SISO639LANGNAME, "en");
    String countryName = localeInfo(LOCALE_SISO3166CTRYNAME, String());

    if (countryName.isEmpty())
        computedDefaultLanguage = languageName;
    else
        computedDefaultLanguage = languageName + '-' + countryName;

    return computedDefaultLanguage;
}

Vector<String> platformUserPreferredLanguages()
{
    return { platformLanguage() };
}

} // namespace WTF
