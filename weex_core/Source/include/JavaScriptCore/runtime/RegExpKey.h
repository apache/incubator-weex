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
