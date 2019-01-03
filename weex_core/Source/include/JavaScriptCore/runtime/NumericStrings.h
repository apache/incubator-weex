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

#include <array>
#include <wtf/HashFunctions.h>
#include <wtf/text/WTFString.h>

namespace JSC {

class NumericStrings {
public:
    ALWAYS_INLINE const String& add(double d)
    {
        CacheEntry<double>& entry = lookup(d);
        if (d == entry.key && !entry.value.isNull())
            return entry.value;
        entry.key = d;
        entry.value = String::numberToStringECMAScript(d);
        return entry.value;
    }

    ALWAYS_INLINE const String& add(int i)
    {
        if (static_cast<unsigned>(i) < cacheSize)
            return lookupSmallString(static_cast<unsigned>(i));
        CacheEntry<int>& entry = lookup(i);
        if (i == entry.key && !entry.value.isNull())
            return entry.value;
        entry.key = i;
        entry.value = String::number(i);
        return entry.value;
    }

    ALWAYS_INLINE const String& add(unsigned i)
    {
        if (i < cacheSize)
            return lookupSmallString(static_cast<unsigned>(i));
        CacheEntry<unsigned>& entry = lookup(i);
        if (i == entry.key && !entry.value.isNull())
            return entry.value;
        entry.key = i;
        entry.value = String::number(i);
        return entry.value;
    }
private:
    static const size_t cacheSize = 64;

    template<typename T>
    struct CacheEntry {
        T key;
        String value;
    };

    CacheEntry<double>& lookup(double d) { return doubleCache[WTF::FloatHash<double>::hash(d) & (cacheSize - 1)]; }
    CacheEntry<int>& lookup(int i) { return intCache[WTF::IntHash<int>::hash(i) & (cacheSize - 1)]; }
    CacheEntry<unsigned>& lookup(unsigned i) { return unsignedCache[WTF::IntHash<unsigned>::hash(i) & (cacheSize - 1)]; }
    ALWAYS_INLINE const String& lookupSmallString(unsigned i)
    {
        ASSERT(i < cacheSize);
        if (smallIntCache[i].isNull())
            smallIntCache[i] = String::number(i);
        return smallIntCache[i];
    }

    std::array<CacheEntry<double>, cacheSize> doubleCache;
    std::array<CacheEntry<int>, cacheSize> intCache;
    std::array<CacheEntry<unsigned>, cacheSize> unsignedCache;
    std::array<String, cacheSize> smallIntCache;
};

} // namespace JSC
