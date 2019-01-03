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
#ifndef WTF_HashCountedSet_h
#define WTF_HashCountedSet_h

#include <initializer_list>
#include <wtf/Assertions.h>
#include <wtf/HashMap.h>
#include <wtf/Vector.h>

namespace WTF {

    template<typename Value, typename HashFunctions = typename DefaultHash<Value>::Hash, typename Traits = HashTraits<Value>>
    class HashCountedSet final {
        WTF_MAKE_FAST_ALLOCATED;
    private:
        typedef HashMap<Value, unsigned, HashFunctions, Traits> ImplType;
    public:
        typedef Value ValueType;
        typedef typename ImplType::iterator iterator;
        typedef typename ImplType::const_iterator const_iterator;
        typedef typename ImplType::AddResult AddResult;

        HashCountedSet()
        {
        }

        HashCountedSet(std::initializer_list<typename ImplType::KeyValuePairType> initializerList)
        {
            for (const auto& keyValuePair : initializerList)
                add(keyValuePair.key, keyValuePair.value);
        }

        HashCountedSet(std::initializer_list<typename ImplType::KeyType> initializerList)
        {
            for (const auto& value : initializerList)
                add(value);
        }
        
        void swap(HashCountedSet&);
        
        unsigned size() const;
        unsigned capacity() const;
        bool isEmpty() const;
        
        // Iterators iterate over pairs of values and counts.
        iterator begin();
        iterator end();
        const_iterator begin() const;
        const_iterator end() const;
        
        iterator find(const ValueType&);
        const_iterator find(const ValueType&) const;
        bool contains(const ValueType&) const;
        unsigned count(const ValueType&) const;

        // Increments the count if an equal value is already present.
        // The return value includes both an iterator to the value's location,
        // and an isNewEntry bool that indicates whether it is a new or existing entry.
        AddResult add(const ValueType&);
        AddResult add(ValueType&&);

        // Increments the count of a value by the passed amount.
        AddResult add(const ValueType&, unsigned);
        AddResult add(ValueType&&, unsigned);

        // Decrements the count of the value, and removes it if count goes down to zero.
        // Returns true if the value is removed.
        bool remove(const ValueType&);
        bool remove(iterator);
 
        // Removes the value, regardless of its count.
        // Returns true if a value was removed.
        bool removeAll(iterator);
        bool removeAll(const ValueType&);

        // Clears the whole set.
        void clear();

        // Overloads for smart pointer keys that take the raw pointer type as the parameter.
        template<typename V = ValueType> typename std::enable_if<IsSmartPtr<V>::value, iterator>::type find(typename GetPtrHelper<V>::PtrType);
        template<typename V = ValueType> typename std::enable_if<IsSmartPtr<V>::value, const_iterator>::type find(typename GetPtrHelper<V>::PtrType) const;
        template<typename V = ValueType> typename std::enable_if<IsSmartPtr<V>::value, bool>::type contains(typename GetPtrHelper<V>::PtrType) const;
        template<typename V = ValueType> typename std::enable_if<IsSmartPtr<V>::value, unsigned>::type count(typename GetPtrHelper<V>::PtrType) const;
        template<typename V = ValueType> typename std::enable_if<IsSmartPtr<V>::value, bool>::type remove(typename GetPtrHelper<V>::PtrType);

    private:
        ImplType m_impl;
    };


    template<typename Value, typename HashFunctions, typename Traits>
    inline void HashCountedSet<Value, HashFunctions, Traits>::swap(HashCountedSet& other)
    {
        m_impl.swap(other.m_impl);
    }
    
    template<typename Value, typename HashFunctions, typename Traits>
    inline unsigned HashCountedSet<Value, HashFunctions, Traits>::size() const
    {
        return m_impl.size(); 
    }
    
    template<typename Value, typename HashFunctions, typename Traits>
    inline unsigned HashCountedSet<Value, HashFunctions, Traits>::capacity() const
    {
        return m_impl.capacity(); 
    }
    
    template<typename Value, typename HashFunctions, typename Traits>
    inline bool HashCountedSet<Value, HashFunctions, Traits>::isEmpty() const
    {
        return size() == 0; 
    }
    
    template<typename Value, typename HashFunctions, typename Traits>
    inline typename HashCountedSet<Value, HashFunctions, Traits>::iterator HashCountedSet<Value, HashFunctions, Traits>::begin()
    {
        return m_impl.begin(); 
    }

    template<typename Value, typename HashFunctions, typename Traits>
    inline typename HashCountedSet<Value, HashFunctions, Traits>::iterator HashCountedSet<Value, HashFunctions, Traits>::end()
    {
        return m_impl.end(); 
    }
    
    template<typename Value, typename HashFunctions, typename Traits>
    inline typename HashCountedSet<Value, HashFunctions, Traits>::const_iterator HashCountedSet<Value, HashFunctions, Traits>::begin() const
    {
        return m_impl.begin(); 
    }
    
    template<typename Value, typename HashFunctions, typename Traits>
    inline typename HashCountedSet<Value, HashFunctions, Traits>::const_iterator HashCountedSet<Value, HashFunctions, Traits>::end() const
    {
        return m_impl.end(); 
    }
    
    template<typename Value, typename HashFunctions, typename Traits>
    inline typename HashCountedSet<Value, HashFunctions, Traits>::iterator HashCountedSet<Value, HashFunctions, Traits>::find(const ValueType& value)
    {
        return m_impl.find(value); 
    }
    
    template<typename Value, typename HashFunctions, typename Traits>
    inline typename HashCountedSet<Value, HashFunctions, Traits>::const_iterator HashCountedSet<Value, HashFunctions, Traits>::find(const ValueType& value) const
    {
        return m_impl.find(value); 
    }
    
    template<typename Value, typename HashFunctions, typename Traits>
    inline bool HashCountedSet<Value, HashFunctions, Traits>::contains(const ValueType& value) const
    {
        return m_impl.contains(value); 
    }

    template<typename Value, typename HashFunctions, typename Traits>
    inline unsigned HashCountedSet<Value, HashFunctions, Traits>::count(const ValueType& value) const
    {
        return m_impl.get(value);
    }
    
    template<typename Value, typename HashFunctions, typename Traits>
    inline typename HashCountedSet<Value, HashFunctions, Traits>::AddResult HashCountedSet<Value, HashFunctions, Traits>::add(const ValueType &value)
    {
        AddResult result = m_impl.add(value, 0);
        ++result.iterator->value;
        return result;
    }

    template<typename Value, typename HashFunctions, typename Traits>
    inline typename HashCountedSet<Value, HashFunctions, Traits>::AddResult HashCountedSet<Value, HashFunctions, Traits>::add(ValueType&& value)
    {
        AddResult result = m_impl.add(std::forward<Value>(value), 0);
        ++result.iterator->value;
        return result;
    }

    template<typename Value, typename HashFunctions, typename Traits>
    inline typename HashCountedSet<Value, HashFunctions, Traits>::AddResult HashCountedSet<Value, HashFunctions, Traits>::add(const ValueType& value, unsigned count)
    {
        AddResult result = m_impl.add(value, 0);
        result.iterator->value += count;
        return result;
    }
    
    template<typename Value, typename HashFunctions, typename Traits>
    inline typename HashCountedSet<Value, HashFunctions, Traits>::AddResult HashCountedSet<Value, HashFunctions, Traits>::add(ValueType&& value, unsigned count)
    {
        AddResult result = m_impl.add(std::forward<Value>(value), 0);
        result.iterator->value += count;
        return result;
    }
    
    template<typename Value, typename HashFunctions, typename Traits>
    inline bool HashCountedSet<Value, HashFunctions, Traits>::remove(const ValueType& value)
    {
        return remove(find(value));
    }
    
    template<typename Value, typename HashFunctions, typename Traits>
    inline bool HashCountedSet<Value, HashFunctions, Traits>::remove(iterator it)
    {
        if (it == end())
            return false;

        unsigned oldVal = it->value;
        ASSERT(oldVal);
        unsigned newVal = oldVal - 1;
        if (newVal) {
            it->value = newVal;
            return false;
        }

        m_impl.remove(it);
        return true;
    }
    
    template<typename Value, typename HashFunctions, typename Traits>
    inline bool HashCountedSet<Value, HashFunctions, Traits>::removeAll(const ValueType& value)
    {
        return removeAll(find(value));
    }
    
    template<typename Value, typename HashFunctions, typename Traits>
    inline bool HashCountedSet<Value, HashFunctions, Traits>::removeAll(iterator it)
    {
        if (it == end())
            return false;

        m_impl.remove(it);
        return true;
    }
    
    template<typename Value, typename HashFunctions, typename Traits>
    inline void HashCountedSet<Value, HashFunctions, Traits>::clear()
    {
        m_impl.clear(); 
    }
    
    template<typename Value, typename HashFunctions, typename Traits, typename VectorType>
    inline void copyToVector(const HashCountedSet<Value, HashFunctions, Traits>& collection, VectorType& vector)
    {
        typedef typename HashCountedSet<Value, HashFunctions, Traits>::const_iterator iterator;
        
        vector.resize(collection.size());
        
        iterator it = collection.begin();
        iterator end = collection.end();
        for (unsigned i = 0; it != end; ++it, ++i)
            vector[i] = *it;
    }

    template<typename Value, typename HashFunctions, typename Traits>
    inline void copyToVector(const HashCountedSet<Value, HashFunctions, Traits>& collection, Vector<Value>& vector)
    {
        typedef typename HashCountedSet<Value, HashFunctions, Traits>::const_iterator iterator;
        
        vector.resize(collection.size());
        
        iterator it = collection.begin();
        iterator end = collection.end();
        for (unsigned i = 0; it != end; ++it, ++i)
            vector[i] = (*it).key;
    }

    template<typename Value, typename HashFunctions, typename Traits>
    template<typename V>
    inline auto HashCountedSet<Value, HashFunctions, Traits>::find(typename GetPtrHelper<V>::PtrType value) -> typename std::enable_if<IsSmartPtr<V>::value, iterator>::type
    {
        return m_impl.find(value);
    }
    
    template<typename Value, typename HashFunctions, typename Traits>
    template<typename V>
    inline auto HashCountedSet<Value, HashFunctions, Traits>::find(typename GetPtrHelper<V>::PtrType value) const -> typename std::enable_if<IsSmartPtr<V>::value, const_iterator>::type
    {
        return m_impl.find(value);
    }
    
    template<typename Value, typename HashFunctions, typename Traits>
    template<typename V>
    inline auto HashCountedSet<Value, HashFunctions, Traits>::contains(typename GetPtrHelper<V>::PtrType value) const -> typename std::enable_if<IsSmartPtr<V>::value, bool>::type
    {
        return m_impl.contains(value);
    }
    
    template<typename Value, typename HashFunctions, typename Traits>
    template<typename V>
    inline auto HashCountedSet<Value, HashFunctions, Traits>::count(typename GetPtrHelper<V>::PtrType value) const -> typename std::enable_if<IsSmartPtr<V>::value, unsigned>::type
    {
        return m_impl.get(value);
    }
    
    template<typename Value, typename HashFunctions, typename Traits>
    template<typename V>
    inline auto HashCountedSet<Value, HashFunctions, Traits>::remove(typename GetPtrHelper<V>::PtrType value) -> typename std::enable_if<IsSmartPtr<V>::value, bool>::type
    {
        return remove(find(value));
    }

} // namespace WTF

using WTF::HashCountedSet;

#endif /* WTF_HashCountedSet_h */
