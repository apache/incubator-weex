/*
 * Copyright (C) 2008 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 * 3.  Neither the name of Apple Inc. ("Apple") nor the names of
 *     its contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef WTF_Collator_h
#define WTF_Collator_h

#include <unicode/uconfig.h>
#include <wtf/Noncopyable.h>

struct UCharIterator;
struct UCollator;

namespace WTF {

class StringView;

#if UCONFIG_NO_COLLATION

class Collator {
public:
    explicit Collator(const char* = nullptr, bool = false) { }

    WTF_EXPORT_PRIVATE static int collate(StringView, StringView);
    WTF_EXPORT_PRIVATE static int collateUTF8(const char*, const char*);
};

#else

class Collator {
    WTF_MAKE_NONCOPYABLE(Collator);
public:
    // The value nullptr is a special one meaning the system default locale.
    // Locale name parsing is lenient; e.g. language identifiers (such as "en-US") are accepted, too.
    WTF_EXPORT_PRIVATE explicit Collator(const char* locale = nullptr, bool shouldSortLowercaseFirst = false);
    WTF_EXPORT_PRIVATE ~Collator();

    WTF_EXPORT_PRIVATE int collate(StringView, StringView) const;
    WTF_EXPORT_PRIVATE int collateUTF8(const char*, const char*) const;

private:
    char* m_locale;
    bool m_shouldSortLowercaseFirst;
    UCollator* m_collator;
};

WTF_EXPORT_PRIVATE UCharIterator createIterator(StringView);

#endif

}

using WTF::Collator;

#endif
