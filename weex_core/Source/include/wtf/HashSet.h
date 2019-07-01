/*
 * Copyright (C) 2005, 2006, 2007, 2008, 2011, 2013, 2017 Apple Inc. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#ifndef WTF_HashSet_h
#define WTF_HashSet_h

#include <initializer_list>
#include <wtf/FastMalloc.h>
#include <wtf/GetPtr.h>
#include <wtf/HashTable.h>

namespace WTF {

    struct IdentityExtractor;
    
    template<typename Value, typename HashFunctions, typename Traits> class HashSet;

    template<typename ValueArg, typename HashArg = typename DefaultHash<ValueArg>::Hash,
        typename TraitsArg = HashTraits<ValueArg>> class HashSet final {
        WTF_MAKE_FAST_ALLOCATED;
    private:
        typedef HashArg HashFunctions;
        typedef TraitsArg ValueTraits;
        typedef typename ValueTraits::TakeType TakeType;

    public:
        typedef typename ValueTraits::TraitType ValueType;

    private:
        typedef HashTable<ValueType, ValueType, IdentityExtractor,
            HashFunctions, ValueTraits, ValueTraits> HashTableType;

    public:
        typedef HashTableConstIteratorAdapter<HashTableType, ValueType> iterator;
        typedef HashTableConstIteratorAdapter<HashTableType, ValueType> const_iterator;
        typedef typename HashTableType::AddResult AddResult;

        HashSet()
        {
        }

        HashSet(std::initializer_list<ValueArg> initializerList)
        {
            for (const auto& value : initializerList)
                add(value);
        }

        void swap(HashSet&);

        unsigned size() const;
        unsigned capacity() const;
        bool isEmpty() const;

        iterator begin() const;
        iterator end() const;

        iterator find(const ValueType&) const;
        bool contains(const ValueType&) const;

        // An alternate version of find() that finds the object by hashing and comparing
        // with some other type, to avoid the cost of type conversion. HashTranslator
        // must have the following function members:
        //   static unsigned hash(const T&);
        //   static bool equal(const ValueType&, const T&);
        template<typename HashTranslator, typename T> iterator find(const T&) const;
        template<typename HashTranslator, typename T> bool contains(const T&) const;

        // The return value includes both an iterator to the added value's location,
        // and an isNewEntry bool that indicates if it is a new or existing entry in the set.
        AddResult add(const ValueType&);
        AddResult add(ValueType&&);
        
        void addVoid(const ValueType&);
        void addVoid(ValueType&&);

        // An alternate version of add() that finds the object by hashing and comparing
        // with some other type, to avoid the cost of type conversion if the object is already
        // in the table. HashTranslator must have the following function members:
        //   static unsigned hash(const T&);
        //   static bool equal(const ValueType&, const T&);
        //   static translate(ValueType&, const T&, unsigned hashCode);
        template<typename HashTranslator, typename T> AddResult add(const T&);
        
        // Attempts to add a list of things to the set. Returns true if any of
        // them are new to the set. Returns false if the set is unchanged.
        template<typename IteratorType>
        bool add(IteratorType begin, IteratorType end);

        bool remove(const ValueType&);
        bool remove(iterator);
        template<typename Functor>
        void removeIf(const Functor&);
        void clear();

        TakeType take(const ValueType&);
        TakeType take(iterator);
        TakeType takeAny();

        // Overloads for smart pointer values that take the raw pointer type as the parameter.
        template<typename V = ValueType> typename std::enable_if<IsSmartPtr<V>::value, iterator>::type find(typename GetPtrHelper<V>::PtrType) const;
        template<typename V = ValueType> typename std::enable_if<IsSmartPtr<V>::value, bool>::type contains(typename GetPtrHelper<V>::PtrType) const;
        template<typename V = ValueType> typename std::enable_if<IsSmartPtr<V>::value, bool>::type remove(typename GetPtrHelper<V>::PtrType);
        template<typename V = ValueType> typename std::enable_if<IsSmartPtr<V>::value, TakeType>::type take(typename GetPtrHelper<V>::PtrType);

        static bool isValidValue(const ValueType&);

        template<typename OtherCollection>
        bool operator==(const OtherCollection&) const;
        
        template<typename OtherCollection>
        bool operator!=(const OtherCollection&) const;

    private:
        HashTableType m_impl;
    };

    struct IdentityExtractor {
        template<typename T> static const T& extract(const T& t) { return t; }
    };

    template<typename ValueTraits, typename HashFunctions>
    struct HashSetTranslator {
        template<typename T> static unsigned hash(const T& key) { return HashFunctions::hash(key); }
        template<typename T, typename U> static bool equal(const T& a, const U& b) { return HashFunctions::equal(a, b); }
        template<typename T, typename U, typename V> static void translate(T& location, U&&, V&& value)
        { 
            ValueTraits::assignToEmpty(location, std::forward<V>(value));
        }
    };

    template<typename Translator>
    struct HashSetTranslatorAdapter {
        template<typename T> static unsigned hash(const T& key) { return Translator::hash(key); }
        template<typename T, typename U> static bool equal(const T& a, const U& b) { return Translator::equal(a, b); }
        template<typename T, typename U> static void translate(T& location, const U& key, const U&, unsigned hashCode)
        {
            Translator::translate(location, key, hashCode);
        }
    };

    template<typename T, typename U, typename V>
    inline void HashSet<T, U, V>::swap(HashSet& other)
    {
        m_impl.swap(other.m_impl); 
    }

    template<typename T, typename U, typename V>
    inline unsigned HashSet<T, U, V>::size() const
    {
        return m_impl.size(); 
    }

    template<typename T, typename U, typename V>
    inline unsigned HashSet<T, U, V>::capacity() const
    {
        return m_impl.capacity(); 
    }

    template<typename T, typename U, typename V>
    inline bool HashSet<T, U, V>::isEmpty() const
    {
        return m_impl.isEmpty(); 
    }

    template<typename T, typename U, typename V>
    inline auto HashSet<T, U, V>::begin() const -> iterator
    {
        return m_impl.begin(); 
    }

    template<typename T, typename U, typename V>
    inline auto HashSet<T, U, V>::end() const -> iterator
    {
        return m_impl.end(); 
    }

    template<typename T, typename U, typename V>
    inline auto HashSet<T, U, V>::find(const ValueType& value) const -> iterator
    {
        return m_impl.find(value); 
    }

    template<typename T, typename U, typename V>
    inline bool HashSet<T, U, V>::contains(const ValueType& value) const
    {
        return m_impl.contains(value); 
    }

    template<typename Value, typename HashFunctions, typename Traits>
    template<typename HashTranslator, typename T>
    inline auto HashSet<Value, HashFunctions, Traits>::find(const T& value) const -> iterator
    {
        return m_impl.template find<HashSetTranslatorAdapter<HashTranslator>>(value);
    }

    template<typename Value, typename HashFunctions, typename Traits>
    template<typename HashTranslator, typename T>
    inline bool HashSet<Value, HashFunctions, Traits>::contains(const T& value) const
    {
        return m_impl.template contains<HashSetTranslatorAdapter<HashTranslator>>(value);
    }

    template<typename T, typename U, typename V>
    inline auto HashSet<T, U, V>::add(const ValueType& value) -> AddResult
    {
        return m_impl.add(value);
    }

    template<typename T, typename U, typename V>
    inline auto HashSet<T, U, V>::add(ValueType&& value) -> AddResult
    {
        return m_impl.add(WTFMove(value));
    }

    template<typename T, typename U, typename V>
    inline void HashSet<T, U, V>::addVoid(const ValueType& value)
    {
        m_impl.add(value);
    }

    template<typename T, typename U, typename V>
    inline void HashSet<T, U, V>::addVoid(ValueType&& value)
    {
        m_impl.add(WTFMove(value));
    }

    template<typename Value, typename HashFunctions, typename Traits>
    template<typename HashTranslator, typename T>
    inline auto HashSet<Value, HashFunctions, Traits>::add(const T& value) -> AddResult
    {
        return m_impl.template addPassingHashCode<HashSetTranslatorAdapter<HashTranslator>>(value, value);
    }

    template<typename T, typename U, typename V>
    template<typename IteratorType>
    inline bool HashSet<T, U, V>::add(IteratorType begin, IteratorType end)
    {
        bool changed = false;
        for (IteratorType iter = begin; iter != end; ++iter)
            changed |= add(*iter).isNewEntry;
        return changed;
    }

    template<typename T, typename U, typename V>
    inline bool HashSet<T, U, V>::remove(iterator it)
    {
        if (it.m_impl == m_impl.end())
            return false;
        m_impl.internalCheckTableConsistency();
        m_impl.removeWithoutEntryConsistencyCheck(it.m_impl);
        return true;
    }

    template<typename T, typename U, typename V>
    inline bool HashSet<T, U, V>::remove(const ValueType& value)
    {
        return remove(find(value));
    }

    template<typename T, typename U, typename V>
    template<typename Functor>
    inline void HashSet<T, U, V>::removeIf(const Functor& functor)
    {
        m_impl.removeIf(functor);
    }

    template<typename T, typename U, typename V>
    inline void HashSet<T, U, V>::clear()
    {
        m_impl.clear(); 
    }

    template<typename T, typename U, typename V>
    inline auto HashSet<T, U, V>::take(iterator it) -> TakeType
    {
        if (it == end())
            return ValueTraits::take(ValueTraits::emptyValue());

        auto result = ValueTraits::take(WTFMove(const_cast<ValueType&>(*it)));
        remove(it);
        return result;
    }

    template<typename T, typename U, typename V>
    inline auto HashSet<T, U, V>::take(const ValueType& value) -> TakeType
    {
        return take(find(value));
    }

    template<typename T, typename U, typename V>
    inline auto HashSet<T, U, V>::takeAny() -> TakeType
    {
        return take(begin());
    }

    template<typename Value, typename HashFunctions, typename Traits>
    template<typename V>
    inline auto HashSet<Value, HashFunctions, Traits>::find(typename GetPtrHelper<V>::PtrType value) const -> typename std::enable_if<IsSmartPtr<V>::value, iterator>::type
    {
        return m_impl.template find<HashSetTranslator<Traits, HashFunctions>>(value);
    }

    template<typename Value, typename HashFunctions, typename Traits>
    template<typename V>
    inline auto HashSet<Value, HashFunctions, Traits>::contains(typename GetPtrHelper<V>::PtrType value) const -> typename std::enable_if<IsSmartPtr<V>::value, bool>::type
    {
        return m_impl.template contains<HashSetTranslator<Traits, HashFunctions>>(value);
    }

    template<typename Value, typename HashFunctions, typename Traits>
    template<typename V>
    inline auto HashSet<Value, HashFunctions, Traits>::remove(typename GetPtrHelper<V>::PtrType value) -> typename std::enable_if<IsSmartPtr<V>::value, bool>::type
    {
        return remove(find(value));
    }

    template<typename Value, typename HashFunctions, typename Traits>
    template<typename V>
    inline auto HashSet<Value, HashFunctions, Traits>::take(typename GetPtrHelper<V>::PtrType value) -> typename std::enable_if<IsSmartPtr<V>::value, TakeType>::type
    {
        return take(find(value));
    }

    template<typename T, typename U, typename V>
    inline bool HashSet<T, U, V>::isValidValue(const ValueType& value)
    {
        if (ValueTraits::isDeletedValue(value))
            return false;

        if (HashFunctions::safeToCompareToEmptyOrDeleted) {
            if (value == ValueTraits::emptyValue())
                return false;
        } else {
            if (isHashTraitsEmptyValue<ValueTraits>(value))
                return false;
        }

        return true;
    }

    template<typename C, typename W>
    inline void copyToVector(const C& collection, W& vector)
    {
        typedef typename C::const_iterator iterator;
        
        vector.resize(collection.size());
        
        iterator it = collection.begin();
        iterator end = collection.end();
        for (unsigned i = 0; it != end; ++it, ++i)
            vector[i] = *it;
    }  

    template<typename T, typename U, typename V>
    template<typename OtherCollection>
    inline bool HashSet<T, U, V>::operator==(const OtherCollection& otherCollection) const
    {
        if (size() != otherCollection.size())
            return false;
        for (const auto& other : otherCollection) {
            if (!contains(other))
                return false;
        }
        return true;
    }
    
    template<typename T, typename U, typename V>
    template<typename OtherCollection>
    inline bool HashSet<T, U, V>::operator!=(const OtherCollection& otherCollection) const
    {
        return !(*this == otherCollection);
    }

} // namespace WTF

using WTF::HashSet;

#endif /* WTF_HashSet_h */
