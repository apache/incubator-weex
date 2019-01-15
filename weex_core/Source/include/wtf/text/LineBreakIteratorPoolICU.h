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

#include "TextBreakIterator.h"
#include <unicode/uloc.h>
#include <wtf/HashMap.h>
#include <wtf/NeverDestroyed.h>
#include <wtf/ThreadSpecific.h>
#include <wtf/text/AtomicString.h>

namespace WTF {

class LineBreakIteratorPool {
    WTF_MAKE_NONCOPYABLE(LineBreakIteratorPool);
public:
    LineBreakIteratorPool() = default;

    static LineBreakIteratorPool& sharedPool()
    {
        static NeverDestroyed<WTF::ThreadSpecific<LineBreakIteratorPool>> pool;
        return *pool.get();
    }

    static AtomicString makeLocaleWithBreakKeyword(const AtomicString& locale, LineBreakIteratorMode mode)
    {
        // The uloc functions model locales as char*, so we have to downconvert our AtomicString.
        auto utf8Locale = locale.string().utf8();
        if (!utf8Locale.length())
            return locale;
        Vector<char> scratchBuffer(utf8Locale.length() + 11, 0);
        memcpy(scratchBuffer.data(), utf8Locale.data(), utf8Locale.length());

        const char* keywordValue = nullptr;
        switch (mode) {
        case LineBreakIteratorMode::Default:
            // nullptr will cause any existing values to be removed.
            break;
        case LineBreakIteratorMode::Loose:
            keywordValue = "loose";
            break;
        case LineBreakIteratorMode::Normal:
            keywordValue = "normal";
            break;
        case LineBreakIteratorMode::Strict:
            keywordValue = "strict";
            break;
        }

        UErrorCode status = U_ZERO_ERROR;
        int32_t lengthNeeded = uloc_setKeywordValue("lb", keywordValue, scratchBuffer.data(), scratchBuffer.size(), &status);
        if (U_SUCCESS(status))
            return AtomicString::fromUTF8(scratchBuffer.data(), lengthNeeded);
        if (status == U_BUFFER_OVERFLOW_ERROR) {
            scratchBuffer.grow(lengthNeeded + 1);
            memset(scratchBuffer.data() + utf8Locale.length(), 0, scratchBuffer.size() - utf8Locale.length());
            status = U_ZERO_ERROR;
            int32_t lengthNeeded2 = uloc_setKeywordValue("lb", keywordValue, scratchBuffer.data(), scratchBuffer.size(), &status);
            if (!U_SUCCESS(status) || lengthNeeded != lengthNeeded2)
                return locale;
            return AtomicString::fromUTF8(scratchBuffer.data(), lengthNeeded);
        }
        return locale;
    }

    UBreakIterator* take(const AtomicString& locale, LineBreakIteratorMode mode)
    {
        auto localeWithOptionalBreakKeyword = makeLocaleWithBreakKeyword(locale, mode);

        UBreakIterator* iterator = nullptr;
        for (size_t i = 0; i < m_pool.size(); ++i) {
            if (m_pool[i].first == localeWithOptionalBreakKeyword) {
                iterator = m_pool[i].second;
                m_pool.remove(i);
                break;
            }
        }

        if (!iterator) {
            iterator = openLineBreakIterator(localeWithOptionalBreakKeyword);
            if (!iterator)
                return nullptr;
        }

        ASSERT(!m_vendedIterators.contains(iterator));
        m_vendedIterators.add(iterator, localeWithOptionalBreakKeyword);
        return iterator;
    }

    void put(UBreakIterator* iterator)
    {
        ASSERT(m_vendedIterators.contains(iterator));
        if (m_pool.size() == capacity) {
            closeLineBreakIterator(m_pool[0].second);
            m_pool.remove(0);
        }
        m_pool.uncheckedAppend({ m_vendedIterators.take(iterator), iterator });
    }

private:
    static constexpr size_t capacity = 4;

    Vector<std::pair<AtomicString, UBreakIterator*>, capacity> m_pool;
    HashMap<UBreakIterator*, AtomicString> m_vendedIterators;

    friend WTF::ThreadSpecific<LineBreakIteratorPool>::operator LineBreakIteratorPool*();
};

}
