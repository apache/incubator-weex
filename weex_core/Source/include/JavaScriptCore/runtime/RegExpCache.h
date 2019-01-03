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
