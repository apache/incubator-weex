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

#include <iterator>

namespace WTF {

    template<typename HashTableType, typename KeyType, typename MappedType> struct HashTableConstKeysIterator;
    template<typename HashTableType, typename KeyType, typename MappedType> struct HashTableConstValuesIterator;
    template<typename HashTableType, typename KeyType, typename MappedType> struct HashTableKeysIterator;
    template<typename HashTableType, typename KeyType, typename MappedType> struct HashTableValuesIterator;

    template<typename HashTableType, typename KeyType, typename MappedType> struct HashTableConstIteratorAdapter<HashTableType, KeyValuePair<KeyType, MappedType>> : public std::iterator<std::forward_iterator_tag, KeyValuePair<KeyType, MappedType>, std::ptrdiff_t, const KeyValuePair<KeyType, MappedType>*, const KeyValuePair<KeyType, MappedType>&> {
    private:
        typedef KeyValuePair<KeyType, MappedType> ValueType;
    public:
        typedef HashTableConstKeysIterator<HashTableType, KeyType, MappedType> Keys;
        typedef HashTableConstValuesIterator<HashTableType, KeyType, MappedType> Values;

        HashTableConstIteratorAdapter() {}
        HashTableConstIteratorAdapter(const typename HashTableType::const_iterator& impl) : m_impl(impl) {}

        const ValueType* get() const { return (const ValueType*)m_impl.get(); }
        const ValueType& operator*() const { return *get(); }
        const ValueType* operator->() const { return get(); }

        HashTableConstIteratorAdapter& operator++() { ++m_impl; return *this; }
        // postfix ++ intentionally omitted

        Keys keys() { return Keys(*this); }
        Values values() { return Values(*this); }

        typename HashTableType::const_iterator m_impl;
    };

    template<typename HashTableType, typename KeyType, typename MappedType> struct HashTableIteratorAdapter<HashTableType, KeyValuePair<KeyType, MappedType>> : public std::iterator<std::forward_iterator_tag, KeyValuePair<KeyType, MappedType>, std::ptrdiff_t, KeyValuePair<KeyType, MappedType>*, KeyValuePair<KeyType, MappedType>&> {
    private:
        typedef KeyValuePair<KeyType, MappedType> ValueType;
    public:
        typedef HashTableKeysIterator<HashTableType, KeyType, MappedType> Keys;
        typedef HashTableValuesIterator<HashTableType, KeyType, MappedType> Values;

        HashTableIteratorAdapter() {}
        HashTableIteratorAdapter(const typename HashTableType::iterator& impl) : m_impl(impl) {}

        ValueType* get() const { return (ValueType*)m_impl.get(); }
        ValueType& operator*() const { return *get(); }
        ValueType* operator->() const { return get(); }

        HashTableIteratorAdapter& operator++() { ++m_impl; return *this; }
        // postfix ++ intentionally omitted

        operator HashTableConstIteratorAdapter<HashTableType, ValueType>() {
            typename HashTableType::const_iterator i = m_impl;
            return i;
        }

        Keys keys() { return Keys(*this); }
        Values values() { return Values(*this); }

        typename HashTableType::iterator m_impl;
    };

    template<typename HashTableType, typename KeyType, typename MappedType> struct HashTableConstKeysIterator : public std::iterator<std::forward_iterator_tag, KeyType, std::ptrdiff_t, const KeyType*, const KeyType&> {
    private:
        typedef HashTableConstIteratorAdapter<HashTableType, KeyValuePair<KeyType, MappedType>> ConstIterator;

    public:
        HashTableConstKeysIterator(const ConstIterator& impl) : m_impl(impl) {}
        
        const KeyType* get() const { return &(m_impl.get()->key); }
        const KeyType& operator*() const { return *get(); }
        const KeyType* operator->() const { return get(); }

        HashTableConstKeysIterator& operator++() { ++m_impl; return *this; }
        // postfix ++ intentionally omitted

        ConstIterator m_impl;
    };

    template<typename HashTableType, typename KeyType, typename MappedType> struct HashTableConstValuesIterator : public std::iterator<std::forward_iterator_tag, MappedType, std::ptrdiff_t, const MappedType*, const MappedType&> {
    private:
        typedef HashTableConstIteratorAdapter<HashTableType, KeyValuePair<KeyType, MappedType>> ConstIterator;

    public:
        HashTableConstValuesIterator(const ConstIterator& impl) : m_impl(impl) {}
        
        const MappedType* get() const { return &(m_impl.get()->value); }
        const MappedType& operator*() const { return *get(); }
        const MappedType* operator->() const { return get(); }

        HashTableConstValuesIterator& operator++() { ++m_impl; return *this; }
        // postfix ++ intentionally omitted

        ConstIterator m_impl;
    };

    template<typename HashTableType, typename KeyType, typename MappedType> struct HashTableKeysIterator : public std::iterator<std::forward_iterator_tag, KeyType, std::ptrdiff_t, KeyType*, KeyType&> {
    private:
        typedef HashTableIteratorAdapter<HashTableType, KeyValuePair<KeyType, MappedType>> Iterator;
        typedef HashTableConstIteratorAdapter<HashTableType, KeyValuePair<KeyType, MappedType>> ConstIterator;

    public:
        HashTableKeysIterator(const Iterator& impl) : m_impl(impl) {}
        
        KeyType* get() const { return &(m_impl.get()->key); }
        KeyType& operator*() const { return *get(); }
        KeyType* operator->() const { return get(); }

        HashTableKeysIterator& operator++() { ++m_impl; return *this; }
        // postfix ++ intentionally omitted

        operator HashTableConstKeysIterator<HashTableType, KeyType, MappedType>() {
            ConstIterator i = m_impl;
            return i;
        }

        Iterator m_impl;
    };

    template<typename HashTableType, typename KeyType, typename MappedType> struct HashTableValuesIterator : public std::iterator<std::forward_iterator_tag, MappedType, std::ptrdiff_t, MappedType*, MappedType&> {
    private:
        typedef HashTableIteratorAdapter<HashTableType, KeyValuePair<KeyType, MappedType>> Iterator;
        typedef HashTableConstIteratorAdapter<HashTableType, KeyValuePair<KeyType, MappedType>> ConstIterator;

    public:
        HashTableValuesIterator(const Iterator& impl) : m_impl(impl) {}
        
        MappedType* get() const { return std::addressof(m_impl.get()->value); }
        MappedType& operator*() const { return *get(); }
        MappedType* operator->() const { return get(); }

        HashTableValuesIterator& operator++() { ++m_impl; return *this; }
        // postfix ++ intentionally omitted

        operator HashTableConstValuesIterator<HashTableType, KeyType, MappedType>() {
            ConstIterator i = m_impl;
            return i;
        }

        Iterator m_impl;
    };

    template<typename T, typename U, typename V>
        inline bool operator==(const HashTableConstKeysIterator<T, U, V>& a, const HashTableConstKeysIterator<T, U, V>& b)
    {
        return a.m_impl == b.m_impl;
    }

    template<typename T, typename U, typename V>
        inline bool operator!=(const HashTableConstKeysIterator<T, U, V>& a, const HashTableConstKeysIterator<T, U, V>& b)
    {
        return a.m_impl != b.m_impl;
    }

    template<typename T, typename U, typename V>
        inline bool operator==(const HashTableConstValuesIterator<T, U, V>& a, const HashTableConstValuesIterator<T, U, V>& b)
    {
        return a.m_impl == b.m_impl;
    }

    template<typename T, typename U, typename V>
        inline bool operator!=(const HashTableConstValuesIterator<T, U, V>& a, const HashTableConstValuesIterator<T, U, V>& b)
    {
        return a.m_impl != b.m_impl;
    }

    template<typename T, typename U, typename V>
        inline bool operator==(const HashTableKeysIterator<T, U, V>& a, const HashTableKeysIterator<T, U, V>& b)
    {
        return a.m_impl == b.m_impl;
    }

    template<typename T, typename U, typename V>
        inline bool operator!=(const HashTableKeysIterator<T, U, V>& a, const HashTableKeysIterator<T, U, V>& b)
    {
        return a.m_impl != b.m_impl;
    }

    template<typename T, typename U, typename V>
        inline bool operator==(const HashTableValuesIterator<T, U, V>& a, const HashTableValuesIterator<T, U, V>& b)
    {
        return a.m_impl == b.m_impl;
    }

    template<typename T, typename U, typename V>
        inline bool operator!=(const HashTableValuesIterator<T, U, V>& a, const HashTableValuesIterator<T, U, V>& b)
    {
        return a.m_impl != b.m_impl;
    }

} // namespace WTF
