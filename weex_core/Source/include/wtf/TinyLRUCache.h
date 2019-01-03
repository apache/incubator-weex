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
