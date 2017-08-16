/*
 * Copyright (C) 2010 University of Szeged
 * Copyright (C) 2010 Renata Hodovan (hodovan@inf.u-szeged.hu)
 * All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY UNIVERSITY OF SZEGED ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL UNIVERSITY OF SZEGED OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include <wtf/text/StringHash.h>
#include <wtf/text/WTFString.h>

namespace JSC {

enum RegExpFlags {
    NoFlags = 0,
    FlagGlobal = 1,
    FlagIgnoreCase = 2,
    FlagMultiline = 4,
    FlagSticky = 8,
    FlagUnicode = 16,
    InvalidFlags = 32,
    DeletedValueFlags = -1
};

struct RegExpKey {
    RegExpFlags flagsValue;
    RefPtr<StringImpl> pattern;

    RegExpKey()
        : flagsValue(NoFlags)
    {
    }

    RegExpKey(RegExpFlags flags)
        : flagsValue(flags)
    {
    }

    RegExpKey(RegExpFlags flags, const String& pattern)
        : flagsValue(flags)
        , pattern(pattern.impl())
    {
    }

    RegExpKey(RegExpFlags flags, RefPtr<StringImpl>&& pattern)
        : flagsValue(flags)
        , pattern(WTFMove(pattern))
    {
    }

    RegExpKey(RegExpFlags flags, const RefPtr<StringImpl>& pattern)
        : flagsValue(flags)
        , pattern(pattern)
    {
    }

    friend inline bool operator==(const RegExpKey& a, const RegExpKey& b);

    struct Hash {
        static unsigned hash(const RegExpKey& key) { return key.pattern->hash(); }
        static bool equal(const RegExpKey& a, const RegExpKey& b) { return a == b; }
        static const bool safeToCompareToEmptyOrDeleted = false;
    };
};

inline bool operator==(const RegExpKey& a, const RegExpKey& b)
{
    if (a.flagsValue != b.flagsValue)
        return false;
    if (!a.pattern)
        return !b.pattern;
    if (!b.pattern)
        return false;
    return equal(a.pattern.get(), b.pattern.get());
}

} // namespace JSC

namespace WTF {
template<typename T> struct DefaultHash;

template<> struct DefaultHash<JSC::RegExpKey> {
    typedef JSC::RegExpKey::Hash Hash;
};

template<> struct HashTraits<JSC::RegExpKey> : GenericHashTraits<JSC::RegExpKey> {
    static const bool emptyValueIsZero = true;
    static void constructDeletedValue(JSC::RegExpKey& slot) { slot.flagsValue = JSC::DeletedValueFlags; }
    static bool isDeletedValue(const JSC::RegExpKey& value) { return value.flagsValue == JSC::DeletedValueFlags; }
};
} // namespace WTF
