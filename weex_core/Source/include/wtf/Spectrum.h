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
#ifndef Spectrum_h
#define Spectrum_h

#include <wtf/HashMap.h>
#include <wtf/Vector.h>
#include <algorithm>

namespace WTF {

template<typename T, typename CounterType = unsigned>
class Spectrum {
public:
    typedef typename HashMap<T, CounterType>::iterator iterator;
    typedef typename HashMap<T, CounterType>::const_iterator const_iterator;
    
    Spectrum() { }
    
    void add(const T& key, CounterType count = 1)
    {
        if (!count)
            return;
        typename HashMap<T, CounterType>::AddResult result = m_map.add(key, count);
        if (!result.isNewEntry)
            result.iterator->value += count;
    }
    
    template<typename U>
    void addAll(const Spectrum<T, U>& otherSpectrum)
    {
        for (auto& entry : otherSpectrum)
            add(entry.key, entry.count);
    }
    
    CounterType get(const T& key) const
    {
        const_iterator iter = m_map.find(key);
        if (iter == m_map.end())
            return 0;
        return iter->value;
    }
    
    size_t size() const { return m_map.size(); }
    
    iterator begin() { return m_map.begin(); }
    iterator end() { return m_map.end(); }
    const_iterator begin() const { return m_map.begin(); }
    const_iterator end() const { return m_map.end(); }
    
    struct KeyAndCount {
        KeyAndCount() { }
        
        KeyAndCount(const T& key, CounterType count)
            : key(key)
            , count(count)
        {
        }
        
        bool operator<(const KeyAndCount& other) const
        {
            if (count != other.count)
                return count < other.count;
            // This causes lower-ordered keys being returned first; this is really just
            // here to make sure that the order is somewhat deterministic rather than being
            // determined by hashing.
            return key > other.key;
        }

        T key;
        CounterType count;
    };
    
    // Returns a list ordered from lowest-count to highest-count.
    Vector<KeyAndCount> buildList() const
    {
        Vector<KeyAndCount> list;
        for (const_iterator iter = begin(); iter != end(); ++iter)
            list.append(KeyAndCount(iter->key, iter->value));
        
        std::sort(list.begin(), list.end());
        return list;
    }
    
    void clear() { m_map.clear(); }
    
    template<typename Functor>
    void removeIf(const Functor& functor)
    {
        m_map.removeIf([&functor] (typename HashMap<T, CounterType>::KeyValuePairType& pair) {
                return functor(KeyAndCount(pair.key, pair.value));
            });
    }
    
private:
    HashMap<T, CounterType> m_map;
};

} // namespace WTF

using WTF::Spectrum;

#endif // Spectrum_h
