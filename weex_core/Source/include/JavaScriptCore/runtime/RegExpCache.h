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

#include "RegExp.h"
#include "RegExpKey.h"
#include "Strong.h"
#include "Weak.h"
#include <array>
#include <wtf/HashMap.h>

namespace JSC {

class RegExpCache : private WeakHandleOwner {
    WTF_MAKE_FAST_ALLOCATED;

    friend class RegExp;
    typedef HashMap<RegExpKey, Weak<RegExp>> RegExpCacheMap;

public:
    RegExpCache(VM* vm);
    void deleteAllCode();

private:
    
    static const unsigned maxStrongCacheablePatternLength = 256;

    static const int maxStrongCacheableEntries = 32;

    void finalize(Handle<Unknown>, void* context) override;

    RegExp* lookupOrCreate(const WTF::String& patternString, RegExpFlags);
    void addToStrongCache(RegExp*);
    RegExpCacheMap m_weakCache; // Holds all regular expressions currently live.
    int m_nextEntryInStrongCache;
    std::array<Strong<RegExp>, maxStrongCacheableEntries> m_strongCache; // Holds a select few regular expressions that have compiled and executed
    VM* m_vm;
};

} // namespace JSC
