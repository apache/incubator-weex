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

#include "Weak.h"
#include <wtf/HashMap.h>

namespace JSC {

// A HashMap with Weak<JSCell> values, which automatically removes values once they're garbage collected.

template<typename KeyArg, typename ValueArg, typename HashArg = typename DefaultHash<KeyArg>::Hash,
    typename KeyTraitsArg = HashTraits<KeyArg>>
class WeakGCMap {
    WTF_MAKE_FAST_ALLOCATED;
    typedef Weak<ValueArg> ValueType;
    typedef HashMap<KeyArg, ValueType, HashArg, KeyTraitsArg> HashMapType;

public:
    typedef typename HashMapType::KeyType KeyType;
    typedef typename HashMapType::AddResult AddResult;
    typedef typename HashMapType::iterator iterator;
    typedef typename HashMapType::const_iterator const_iterator;

    explicit WeakGCMap(VM&);
    ~WeakGCMap();

    ValueArg* get(const KeyType& key) const
    {
        return m_map.get(key);
    }

    AddResult set(const KeyType& key, ValueType value)
    {
        return m_map.set(key, WTFMove(value));
    }

    bool remove(const KeyType& key)
    {
        return m_map.remove(key);
    }

    void clear()
    {
        m_map.clear();
    }

    bool isEmpty() const
    {
        const_iterator it = m_map.begin();
        const_iterator end = m_map.end();
        while (it != end) {
            if (it->value)
                return true;
        }
        return false;
    }

    inline iterator find(const KeyType& key);

    inline const_iterator find(const KeyType& key) const;

    template<typename Functor>
    void forEach(Functor functor)
    {
        for (auto& pair : m_map) {
            if (pair.value)
                functor(pair.key, pair.value.get());
        }
    }

    inline bool contains(const KeyType& key) const;

    void pruneStaleEntries();

private:
    HashMapType m_map;
    VM& m_vm;
};

} // namespace JSC
