/*
 * Copyright (C) 2016 Apple Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
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
