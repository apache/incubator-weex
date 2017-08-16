/*
 * Copyright (C) 2010, 2016 Apple Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef TinyLRUCache_h
#define TinyLRUCache_h

#include <wtf/NeverDestroyed.h>
#include <wtf/Vector.h>

namespace WTF {

template<typename KeyType, typename ValueType>
struct TinyLRUCachePolicy {
    static bool isKeyNull(const KeyType&) { return false; }
    static ValueType createValueForNullKey() { return { }; }
    static ValueType createValueForKey(const KeyType&) { return { }; }
};

template<typename KeyType, typename ValueType, size_t capacity = 4, typename Policy = TinyLRUCachePolicy<KeyType, ValueType>>
class TinyLRUCache {
public:
    const ValueType& get(const KeyType& key)
    {
        if (Policy::isKeyNull(key)) {
            static NeverDestroyed<ValueType> valueForNull = Policy::createValueForNullKey();
            return valueForNull;
        }

        for (size_t i = 0; i < m_cache.size(); ++i) {
            if (m_cache[i].first != key)
                continue;

            if (i == m_cache.size() - 1)
                return m_cache[i].second;

            // If the entry is not the last one, move it to the end of the cache.
            Entry entry = WTFMove(m_cache[i]);
            m_cache.remove(i);
            m_cache.append(WTFMove(entry));
            return m_cache[m_cache.size() - 1].second;
        }

        // m_cache[0] is the LRU entry, so remove it.
        if (m_cache.size() == capacity)
            m_cache.remove(0);

        m_cache.append(std::make_pair(key, Policy::createValueForKey(key)));
        return m_cache.last().second;
    }

private:
    typedef std::pair<KeyType, ValueType> Entry;
    typedef Vector<Entry, capacity> Cache;
    Cache m_cache;
};

}

using WTF::TinyLRUCache;
using WTF::TinyLRUCachePolicy;

#endif // TinyLRUCache_h
