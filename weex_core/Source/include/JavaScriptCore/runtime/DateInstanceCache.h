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

#include "JSCJSValue.h"
#include "JSDateMath.h"
#include <array>
#include <wtf/HashFunctions.h>
#include <wtf/RefCounted.h>

namespace JSC {

class DateInstanceData : public RefCounted<DateInstanceData> {
public:
    static Ref<DateInstanceData> create() { return adoptRef(*new DateInstanceData); }

    double m_gregorianDateTimeCachedForMS;
    GregorianDateTime m_cachedGregorianDateTime;
    double m_gregorianDateTimeUTCCachedForMS;
    GregorianDateTime m_cachedGregorianDateTimeUTC;

private:
    DateInstanceData()
        : m_gregorianDateTimeCachedForMS(PNaN)
        , m_gregorianDateTimeUTCCachedForMS(PNaN)
    {
    }
};

class DateInstanceCache {
public:
    DateInstanceCache()
    {
        reset();
    }

    void reset()
    {
        for (size_t i = 0; i < cacheSize; ++i)
            m_cache[i].key = PNaN;
    }

    DateInstanceData* add(double d)
    {
        CacheEntry& entry = lookup(d);
        if (d == entry.key)
            return entry.value.get();

        entry.key = d;
        entry.value = DateInstanceData::create();
        return entry.value.get();
    }

private:
    static const size_t cacheSize = 16;

    struct CacheEntry {
        double key;
        RefPtr<DateInstanceData> value;
    };

    CacheEntry& lookup(double d) { return m_cache[WTF::FloatHash<double>::hash(d) & (cacheSize - 1)]; }

    std::array<CacheEntry, cacheSize> m_cache;
};

} // namespace JSC
