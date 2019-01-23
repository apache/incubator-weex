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
#ifndef Map_h
#define Map_h

#include "Inline.h"
#include "Sizes.h"
#include "Vector.h"

namespace bmalloc {

class SmallPage;

template<typename Key, typename Value, typename Hash> class Map {
    static_assert(std::is_trivially_destructible<Key>::value, "Map must have a trivial destructor.");
    static_assert(std::is_trivially_destructible<Value>::value, "Map must have a trivial destructor.");
public:
    struct Bucket {
        Key key;
        Value value;
    };
    
    size_t size() { return m_keyCount; }
    size_t capacity() { return m_table.size(); }

    // key must be in the map.
    Value& get(const Key& key)
    {
        auto& bucket = find(key, [&](const Bucket& bucket) { return bucket.key == key; });
        return bucket.value;
    }

    void set(const Key& key, const Value& value)
    {
        if (shouldGrow())
            rehash();

        auto& bucket = find(key, [&](const Bucket& bucket) { return !bucket.key || bucket.key == key; });
        if (!bucket.key) {
            bucket.key = key;
            ++m_keyCount;
        }
        bucket.value = value;
    }

    // key must be in the map.
    Value remove(const Key& key)
    {
        if (shouldShrink())
            rehash();

        auto& bucket = find(key, [&](const Bucket& bucket) { return bucket.key == key; });
        Value value = bucket.value;
        bucket.key = Key();
        --m_keyCount;
        return value;
    }

private:
    static const unsigned minCapacity = 16;
    static const unsigned maxLoad = 2;
    static const unsigned rehashLoad = 4;
    static const unsigned minLoad = 8;

    bool shouldGrow() { return m_keyCount * maxLoad >= capacity(); }
    bool shouldShrink() { return m_keyCount * minLoad <= capacity() && capacity() > minCapacity; }

    void rehash();

    template<typename Predicate>
    Bucket& find(const Key& key, const Predicate& predicate)
    {
        for (unsigned h = Hash::hash(key); ; ++h) {
            unsigned i = h & m_tableMask;

            Bucket& bucket = m_table[i];
            if (predicate(bucket))
                return bucket;
        }
    }

    unsigned m_keyCount;
    unsigned m_tableMask;
    Vector<Bucket> m_table;
};

template<typename Key, typename Value, typename Hash>
void Map<Key, Value, Hash>::rehash()
{
    auto oldTable = std::move(m_table);

    size_t newCapacity = std::max(minCapacity, m_keyCount * rehashLoad);
    m_table.resize(newCapacity);

    m_keyCount = 0;
    m_tableMask = newCapacity - 1;

    for (auto& bucket : oldTable) {
        if (!bucket.key)
            continue;

        BASSERT(!shouldGrow());
        set(bucket.key, bucket.value);
    }
}

template<typename Key, typename Value, typename Hash> const unsigned Map<Key, Value, Hash>::minCapacity;

} // namespace bmalloc

#endif // Map_h
