/*
 * Copyright (C) 2005, 2006, 2007, 2008, 2011, 2013 Apple Inc. All rights reserved.
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

#ifndef WTF_HashMap_h
#define WTF_HashMap_h

#include <initializer_list>
#include <wtf/DataLog.h>
#include <wtf/HashTable.h>
#include <wtf/IteratorRange.h>

namespace WTF {

template<typename T> struct KeyValuePairKeyExtractor {
    static const typename T::KeyType& extract(const T& p) { return p.key; }
};

template<typename KeyArg, typename MappedArg, typename HashArg = typename DefaultHash<KeyArg>::Hash,
    typename KeyTraitsArg = HashTraits<KeyArg>, typename MappedTraitsArg = HashTraits<MappedArg>>
class HashMap final {
    WTF_MAKE_FAST_ALLOCATED;
private:
    typedef KeyTraitsArg KeyTraits;
    typedef MappedTraitsArg MappedTraits;

    struct KeyValuePairTraits : KeyValuePairHashTraits<KeyTraits, MappedTraits> {
        static const bool hasIsEmptyValueFunction = true;
        static bool isEmptyValue(const typename KeyValuePairHashTraits<KeyTraits, MappedTraits>::TraitType& value)
        {
            return isHashTraitsEmptyValue<KeyTraits>(value.key);
        }
    };

public:
    typedef typename KeyTraits::TraitType KeyType;
    typedef typename MappedTraits::TraitType MappedType;
    typedef typename KeyValuePairTraits::TraitType KeyValuePairType;

private:
    typedef typename MappedTraits::PeekType MappedPeekType;
    typedef typename MappedTraits::TakeType MappedTakeType;

    typedef HashArg HashFunctions;

    typedef HashTable<KeyType, KeyValuePairType, KeyValuePairKeyExtractor<KeyValuePairType>,
        HashFunctions, KeyValuePairTraits, KeyTraits> HashTableType;

    class HashMapKeysProxy;
    class HashMapValuesProxy;

public:
    typedef HashTableIteratorAdapter<HashTableType, KeyValuePairType> iterator;
    typedef HashTableConstIteratorAdapter<HashTableType, KeyValuePairType> const_iterator;
    typedef typename HashTableType::AddResult AddResult;

public:
    HashMap()
    {
    }

    HashMap(std::initializer_list<KeyValuePairType> initializerList)
    {
        for (const auto& keyValuePair : initializerList)
            add(keyValuePair.key, keyValuePair.value);
    }
    
    void swap(HashMap&);

    unsigned size() const;
    unsigned capacity() const;
    bool isEmpty() const;

    // iterators iterate over pairs of keys and values
    iterator begin();
    iterator end();
    const_iterator begin() const;
    const_iterator end() const;

    IteratorRange<typename iterator::Keys> keys() { return makeIteratorRange(begin().keys(), end().keys()); }
    const IteratorRange<typename const_iterator::Keys> keys() const { return makeIteratorRange(begin().keys(), end().keys()); }

    IteratorRange<typename iterator::Values> values() { return makeIteratorRange(begin().values(), end().values()); }
    const IteratorRange<typename const_iterator::Values> values() const { return makeIteratorRange(begin().values(), end().values()); }

    iterator find(const KeyType&);
    const_iterator find(const KeyType&) const;
    bool contains(const KeyType&) const;
    MappedPeekType get(const KeyType&) const;

    // Same as get(), but aggressively inlined.
    MappedPeekType fastGet(const KeyType&) const;

    // Replaces the value but not the key if the key is already present.
    // Return value includes both an iterator to the key location,
    // and an isNewEntry boolean that's true if a new entry was added.
    template<typename V> AddResult set(const KeyType&, V&&);
    template<typename V> AddResult set(KeyType&&, V&&);

    // Does nothing if the key is already present.
    // Return value includes both an iterator to the key location,
    // and an isNewEntry boolean that's true if a new entry was added.
    template<typename V> AddResult add(const KeyType&, V&&);
    template<typename V> AddResult add(KeyType&&, V&&);

    // Same as add(), but aggressively inlined.
    template<typename V> AddResult fastAdd(const KeyType&, V&&);
    template<typename V> AddResult fastAdd(KeyType&&, V&&);

    template<typename Functor> AddResult ensure(const KeyType&, Functor&&);
    template<typename Functor> AddResult ensure(KeyType&&, Functor&&);

    bool remove(const KeyType&);
    bool remove(iterator);
    template<typename Functor>
    void removeIf(Functor&&);
    void clear();

    MappedTakeType take(const KeyType&); // efficient combination of get with remove

    // An alternate version of find() that finds the object by hashing and comparing
    // with some other type, to avoid the cost of type conversion. HashTranslator
    // must have the following function members:
    //   static unsigned hash(const T&);
    //   static bool equal(const ValueType&, const T&);
    template<typename HashTranslator, typename T> iterator find(const T&);
    template<typename HashTranslator, typename T> const_iterator find(const T&) const;
    template<typename HashTranslator, typename T> bool contains(const T&) const;

    // An alternate version of add() that finds the object by hashing and comparing
    // with some other type, to avoid the cost of type conversion if the object is already
    // in the table. HashTranslator must have the following function members:
    //   static unsigned hash(const T&);
    //   static bool equal(const ValueType&, const T&);
    //   static translate(ValueType&, const T&, unsigned hashCode);
    template<typename HashTranslator, typename K, typename V> AddResult add(K&&, V&&);

    // Overloads for smart pointer keys that take the raw pointer type as the parameter.
    template<typename K = KeyType> typename std::enable_if<IsSmartPtr<K>::value, iterator>::type find(typename GetPtrHelper<K>::PtrType);
    template<typename K = KeyType> typename std::enable_if<IsSmartPtr<K>::value, const_iterator>::type find(typename GetPtrHelper<K>::PtrType) const;
    template<typename K = KeyType> typename std::enable_if<IsSmartPtr<K>::value, bool>::type contains(typename GetPtrHelper<K>::PtrType) const;
    template<typename K = KeyType> typename std::enable_if<IsSmartPtr<K>::value, MappedPeekType>::type inlineGet(typename GetPtrHelper<K>::PtrType) const;
    template<typename K = KeyType> typename std::enable_if<IsSmartPtr<K>::value, MappedPeekType>::type get(typename GetPtrHelper<K>::PtrType) const;
    template<typename K = KeyType> typename std::enable_if<IsSmartPtr<K>::value, bool>::type remove(typename GetPtrHelper<K>::PtrType);
    template<typename K = KeyType> typename std::enable_if<IsSmartPtr<K>::value, MappedTakeType>::type take(typename GetPtrHelper<K>::PtrType);

    void checkConsistency() const;

    static bool isValidKey(const KeyType&);

private:
    template<typename K, typename V>
    AddResult inlineSet(K&&, V&&);

    template<typename K, typename V>
    AddResult inlineAdd(K&&, V&&);

    template<typename K, typename F>
    AddResult inlineEnsure(K&&, F&&);

    HashTableType m_impl;
};

template<typename ValueTraits, typename HashFunctions>
struct HashMapTranslator {
    template<typename T> static unsigned hash(const T& key) { return HashFunctions::hash(key); }
    template<typename T, typename U> static bool equal(const T& a, const U& b) { return HashFunctions::equal(a, b); }
    template<typename T, typename U, typename V> static void translate(T& location, U&& key, V&& mapped)
    {
        ValueTraits::KeyTraits::assignToEmpty(location.key, std::forward<U>(key));
        ValueTraits::ValueTraits::assignToEmpty(location.value, std::forward<V>(mapped));
    }
};

template<typename ValueTraits, typename HashFunctions>
struct HashMapEnsureTranslator {
    template<typename T> static unsigned hash(const T& key) { return HashFunctions::hash(key); }
    template<typename T, typename U> static bool equal(const T& a, const U& b) { return HashFunctions::equal(a, b); }
    template<typename T, typename U, typename Functor> static void translate(T& location, U&& key, Functor&& functor)
    {
        ValueTraits::KeyTraits::assignToEmpty(location.key, std::forward<U>(key));
        ValueTraits::ValueTraits::assignToEmpty(location.value, functor());
    }
};

template<typename ValueTraits, typename Translator>
struct HashMapTranslatorAdapter {
    template<typename T> static unsigned hash(const T& key) { return Translator::hash(key); }
    template<typename T, typename U> static bool equal(const T& a, const U& b) { return Translator::equal(a, b); }
    template<typename T, typename U, typename V> static void translate(T& location, U&& key, V&& mapped, unsigned hashCode)
    {
        Translator::translate(location.key, key, hashCode);
        location.value = std::forward<V>(mapped);
    }
};

template<typename T, typename U, typename V, typename W, typename X>
inline void HashMap<T, U, V, W, X>::swap(HashMap& other)
{
    m_impl.swap(other.m_impl);
}

template<typename T, typename U, typename V, typename W, typename X>
inline unsigned HashMap<T, U, V, W, X>::size() const
{
    return m_impl.size(); 
}

template<typename T, typename U, typename V, typename W, typename X>
inline unsigned HashMap<T, U, V, W, X>::capacity() const
{ 
    return m_impl.capacity(); 
}

template<typename T, typename U, typename V, typename W, typename X>
inline bool HashMap<T, U, V, W, X>::isEmpty() const
{
    return m_impl.isEmpty();
}

template<typename T, typename U, typename V, typename W, typename X>
inline auto HashMap<T, U, V, W, X>::begin() -> iterator
{
    return m_impl.begin();
}

template<typename T, typename U, typename V, typename W, typename X>
inline auto HashMap<T, U, V, W, X>::end() -> iterator
{
    return m_impl.end();
}

template<typename T, typename U, typename V, typename W, typename X>
inline auto HashMap<T, U, V, W, X>::begin() const -> const_iterator
{
    return m_impl.begin();
}

template<typename T, typename U, typename V, typename W, typename X>
inline auto HashMap<T, U, V, W, X>::end() const -> const_iterator
{
    return m_impl.end();
}

template<typename T, typename U, typename V, typename W, typename X>
inline auto HashMap<T, U, V, W, X>::find(const KeyType& key) -> iterator
{
    return m_impl.find(key);
}

template<typename T, typename U, typename V, typename W, typename X>
inline auto HashMap<T, U, V, W, X>::find(const KeyType& key) const -> const_iterator
{
    return m_impl.find(key);
}

template<typename T, typename U, typename V, typename W, typename X>
inline bool HashMap<T, U, V, W, X>::contains(const KeyType& key) const
{
    return m_impl.contains(key);
}

template<typename T, typename U, typename V, typename W, typename X>
template<typename HashTranslator, typename TYPE>
inline typename HashMap<T, U, V, W, X>::iterator
HashMap<T, U, V, W, X>::find(const TYPE& value)
{
    return m_impl.template find<HashMapTranslatorAdapter<KeyValuePairTraits, HashTranslator>>(value);
}

template<typename T, typename U, typename V, typename W, typename X>
template<typename HashTranslator, typename TYPE>
inline typename HashMap<T, U, V, W, X>::const_iterator 
HashMap<T, U, V, W, X>::find(const TYPE& value) const
{
    return m_impl.template find<HashMapTranslatorAdapter<KeyValuePairTraits, HashTranslator>>(value);
}

template<typename T, typename U, typename V, typename W, typename X>
template<typename HashTranslator, typename TYPE>
inline bool HashMap<T, U, V, W, X>::contains(const TYPE& value) const
{
    return m_impl.template contains<HashMapTranslatorAdapter<KeyValuePairTraits, HashTranslator>>(value);
}

template<typename KeyArg, typename MappedArg, typename HashArg, typename KeyTraitsArg, typename MappedTraitsArg>
template<typename K, typename V>
auto HashMap<KeyArg, MappedArg, HashArg, KeyTraitsArg, MappedTraitsArg>::inlineSet(K&& key, V&& value) -> AddResult
{
    AddResult result = inlineAdd(std::forward<K>(key), std::forward<V>(value));
    if (!result.isNewEntry) {
        // The inlineAdd call above found an existing hash table entry; we need to set the mapped value.
        result.iterator->value = std::forward<V>(value);
    }
    return result;
}

template<typename KeyArg, typename MappedArg, typename HashArg, typename KeyTraitsArg, typename MappedTraitsArg>
template<typename K, typename V>
ALWAYS_INLINE auto HashMap<KeyArg, MappedArg, HashArg, KeyTraitsArg, MappedTraitsArg>::inlineAdd(K&& key, V&& value) -> AddResult
{
    return m_impl.template add<HashMapTranslator<KeyValuePairTraits, HashFunctions>>(std::forward<K>(key), std::forward<V>(value));
}

template<typename KeyArg, typename MappedArg, typename HashArg, typename KeyTraitsArg, typename MappedTraitsArg>
template<typename K, typename F>
ALWAYS_INLINE auto HashMap<KeyArg, MappedArg, HashArg, KeyTraitsArg, MappedTraitsArg>::inlineEnsure(K&& key, F&& functor) -> AddResult
{
    return m_impl.template add<HashMapEnsureTranslator<KeyValuePairTraits, HashFunctions>>(std::forward<K>(key), std::forward<F>(functor));
}

template<typename KeyArg, typename MappedArg, typename HashArg, typename KeyTraitsArg, typename MappedTraitsArg>
template<typename T>
auto HashMap<KeyArg, MappedArg, HashArg, KeyTraitsArg, MappedTraitsArg>::set(const KeyType& key, T&& mapped) -> AddResult
{
    return inlineSet(key, std::forward<T>(mapped));
}

template<typename KeyArg, typename MappedArg, typename HashArg, typename KeyTraitsArg, typename MappedTraitsArg>
template<typename T>
auto HashMap<KeyArg, MappedArg, HashArg, KeyTraitsArg, MappedTraitsArg>::set(KeyType&& key, T&& mapped) -> AddResult
{
    return inlineSet(WTFMove(key), std::forward<T>(mapped));
}

template<typename KeyArg, typename MappedArg, typename HashArg, typename KeyTraitsArg, typename MappedTraitsArg>
template<typename HashTranslator, typename K, typename V>
auto HashMap<KeyArg, MappedArg, HashArg, KeyTraitsArg, MappedTraitsArg>::add(K&& key, V&& value) -> AddResult
{
    return m_impl.template addPassingHashCode<HashMapTranslatorAdapter<KeyValuePairTraits, HashTranslator>>(std::forward<K>(key), std::forward<V>(value));
}

template<typename KeyArg, typename MappedArg, typename HashArg, typename KeyTraitsArg, typename MappedTraitsArg>
template<typename T>
auto HashMap<KeyArg, MappedArg, HashArg, KeyTraitsArg, MappedTraitsArg>::add(const KeyType& key, T&& mapped) -> AddResult
{
    return inlineAdd(key, std::forward<T>(mapped));
}

template<typename KeyArg, typename MappedArg, typename HashArg, typename KeyTraitsArg, typename MappedTraitsArg>
template<typename T>
auto HashMap<KeyArg, MappedArg, HashArg, KeyTraitsArg, MappedTraitsArg>::add(KeyType&& key, T&& mapped) -> AddResult
{
    return inlineAdd(WTFMove(key), std::forward<T>(mapped));
}

template<typename KeyArg, typename MappedArg, typename HashArg, typename KeyTraitsArg, typename MappedTraitsArg>
template<typename T>
ALWAYS_INLINE auto HashMap<KeyArg, MappedArg, HashArg, KeyTraitsArg, MappedTraitsArg>::fastAdd(const KeyType& key, T&& mapped) -> AddResult
{
    return inlineAdd(key, std::forward<T>(mapped));
}

template<typename KeyArg, typename MappedArg, typename HashArg, typename KeyTraitsArg, typename MappedTraitsArg>
template<typename T>
ALWAYS_INLINE auto HashMap<KeyArg, MappedArg, HashArg, KeyTraitsArg, MappedTraitsArg>::fastAdd(KeyType&& key, T&& mapped) -> AddResult
{
    return inlineAdd(WTFMove(key), std::forward<T>(mapped));
}

template<typename KeyArg, typename MappedArg, typename HashArg, typename KeyTraitsArg, typename MappedTraitsArg>
template<typename Functor>
auto HashMap<KeyArg, MappedArg, HashArg, KeyTraitsArg, MappedTraitsArg>::ensure(const KeyType& key, Functor&& functor) -> AddResult
{
    return inlineEnsure(key, std::forward<Functor>(functor));
}

template<typename KeyArg, typename MappedArg, typename HashArg, typename KeyTraitsArg, typename MappedTraitsArg>
template<typename Functor>
auto HashMap<KeyArg, MappedArg, HashArg, KeyTraitsArg, MappedTraitsArg>::ensure(KeyType&& key, Functor&& functor) -> AddResult
{
    return inlineEnsure(std::forward<KeyType>(key), std::forward<Functor>(functor));
}
    
template<typename T, typename U, typename V, typename W, typename MappedTraits>
auto HashMap<T, U, V, W, MappedTraits>::get(const KeyType& key) const -> MappedPeekType
{
    KeyValuePairType* entry = const_cast<HashTableType&>(m_impl).lookup(key);
    if (!entry)
        return MappedTraits::peek(MappedTraits::emptyValue());
    return MappedTraits::peek(entry->value);
}

template<typename T, typename U, typename V, typename W, typename MappedTraits>
ALWAYS_INLINE auto HashMap<T, U, V, W, MappedTraits>::fastGet(const KeyType& key) const -> MappedPeekType
{
    KeyValuePairType* entry = const_cast<HashTableType&>(m_impl).template inlineLookup<typename HashTableType::IdentityTranslatorType>(key);
    if (!entry)
        return MappedTraits::peek(MappedTraits::emptyValue());
    return MappedTraits::peek(entry->value);
}

template<typename T, typename U, typename V, typename W, typename X>
inline bool HashMap<T, U, V, W, X>::remove(iterator it)
{
    if (it.m_impl == m_impl.end())
        return false;
    m_impl.internalCheckTableConsistency();
    m_impl.removeWithoutEntryConsistencyCheck(it.m_impl);
    return true;
}

template<typename T, typename U, typename V, typename W, typename X>
template<typename Functor>
inline void HashMap<T, U, V, W, X>::removeIf(Functor&& functor)
{
    m_impl.removeIf(std::forward<Functor>(functor));
}

template<typename T, typename U, typename V, typename W, typename X>
inline bool HashMap<T, U, V, W, X>::remove(const KeyType& key)
{
    return remove(find(key));
}

template<typename T, typename U, typename V, typename W, typename X>
inline void HashMap<T, U, V, W, X>::clear()
{
    m_impl.clear();
}

template<typename T, typename U, typename V, typename W, typename MappedTraits>
auto HashMap<T, U, V, W, MappedTraits>::take(const KeyType& key) -> MappedTakeType
{
    iterator it = find(key);
    if (it == end())
        return MappedTraits::take(MappedTraits::emptyValue());
    auto value = MappedTraits::take(WTFMove(it->value));
    remove(it);
    return value;
}

template<typename T, typename U, typename V, typename W, typename X>
template<typename K>
inline auto HashMap<T, U, V, W, X>::find(typename GetPtrHelper<K>::PtrType key) -> typename std::enable_if<IsSmartPtr<K>::value, iterator>::type
{
    return m_impl.template find<HashMapTranslator<KeyValuePairTraits, HashFunctions>>(key);
}

template<typename T, typename U, typename V, typename W, typename X>
template<typename K>
inline auto HashMap<T, U, V, W, X>::find(typename GetPtrHelper<K>::PtrType key) const -> typename std::enable_if<IsSmartPtr<K>::value, const_iterator>::type
{
    return m_impl.template find<HashMapTranslator<KeyValuePairTraits, HashFunctions>>(key);
}

template<typename T, typename U, typename V, typename W, typename X>
template<typename K>
inline auto HashMap<T, U, V, W, X>::contains(typename GetPtrHelper<K>::PtrType key) const -> typename std::enable_if<IsSmartPtr<K>::value, bool>::type
{
    return m_impl.template contains<HashMapTranslator<KeyValuePairTraits, HashFunctions>>(key);
}

template<typename T, typename U, typename V, typename W, typename X>
template<typename K>
inline auto HashMap<T, U, V, W, X>::inlineGet(typename GetPtrHelper<K>::PtrType key) const -> typename std::enable_if<IsSmartPtr<K>::value, MappedPeekType>::type
{
    KeyValuePairType* entry = const_cast<HashTableType&>(m_impl).template inlineLookup<HashMapTranslator<KeyValuePairTraits, HashFunctions>>(key);
    if (!entry)
        return MappedTraits::peek(MappedTraits::emptyValue());
    return MappedTraits::peek(entry->value);
}

template<typename T, typename U, typename V, typename W, typename X>
template<typename K>
auto HashMap<T, U, V, W, X>::get(typename GetPtrHelper<K>::PtrType key) const -> typename std::enable_if<IsSmartPtr<K>::value, MappedPeekType>::type
{
    return inlineGet(key);
}

template<typename T, typename U, typename V, typename W, typename X>
template<typename K>
inline auto HashMap<T, U, V, W, X>::remove(typename GetPtrHelper<K>::PtrType key) -> typename std::enable_if<IsSmartPtr<K>::value, bool>::type
{
    return remove(find(key));
}

template<typename T, typename U, typename V, typename W, typename X>
template<typename K>
inline auto HashMap<T, U, V, W, X>::take(typename GetPtrHelper<K>::PtrType key) -> typename std::enable_if<IsSmartPtr<K>::value, MappedTakeType>::type
{
    iterator it = find(key);
    if (it == end())
        return MappedTraits::take(MappedTraits::emptyValue());
    auto value = MappedTraits::take(WTFMove(it->value));
    remove(it);
    return value;
}

template<typename T, typename U, typename V, typename W, typename X>
inline void HashMap<T, U, V, W, X>::checkConsistency() const
{
    m_impl.checkTableConsistency();
}

template<typename T, typename U, typename V, typename W, typename X>
inline bool HashMap<T, U, V, W, X>::isValidKey(const KeyType& key)
{
    if (KeyTraits::isDeletedValue(key))
        return false;

    if (HashFunctions::safeToCompareToEmptyOrDeleted) {
        if (key == KeyTraits::emptyValue())
            return false;
    } else {
        if (isHashTraitsEmptyValue<KeyTraits>(key))
            return false;
    }

    return true;
}

template<typename T, typename U, typename V, typename W, typename X>
bool operator==(const HashMap<T, U, V, W, X>& a, const HashMap<T, U, V, W, X>& b)
{
    if (a.size() != b.size())
        return false;

    typedef typename HashMap<T, U, V, W, X>::const_iterator const_iterator;

    const_iterator end = a.end();
    const_iterator notFound = b.end();
    for (const_iterator it = a.begin(); it != end; ++it) {
        const_iterator bPos = b.find(it->key);
        if (bPos == notFound || it->value != bPos->value)
            return false;
    }

    return true;
}

template<typename T, typename U, typename V, typename W, typename X>
inline bool operator!=(const HashMap<T, U, V, W, X>& a, const HashMap<T, U, V, W, X>& b)
{
    return !(a == b);
}

template<typename T, typename U, typename V, typename W, typename X, typename Y>
inline void copyToVector(const HashMap<T, U, V, W, X>& collection, Y& vector)
{
    typedef typename HashMap<T, U, V, W, X>::const_iterator iterator;

    vector.resize(collection.size());

    iterator it = collection.begin();
    iterator end = collection.end();
    for (unsigned i = 0; it != end; ++it, ++i)
        vector[i] = { (*it).key, (*it).value };
}

template<typename T, typename U, typename V, typename W, typename X, typename Y>
inline void copyKeysToVector(const HashMap<T, U, V, W, X>& collection, Y& vector)
{
    typedef typename HashMap<T, U, V, W, X>::const_iterator::Keys iterator;
    
    vector.resize(collection.size());
    
    iterator it = collection.begin().keys();
    iterator end = collection.end().keys();
    for (unsigned i = 0; it != end; ++it, ++i)
        vector[i] = *it;
}  

template<typename T, typename U, typename V, typename W, typename X, typename Y>
inline void copyValuesToVector(const HashMap<T, U, V, W, X>& collection, Y& vector)
{
    typedef typename HashMap<T, U, V, W, X>::const_iterator::Values iterator;
    
    vector.resize(collection.size());
    
    iterator it = collection.begin().values();
    iterator end = collection.end().values();
    for (unsigned i = 0; it != end; ++it, ++i)
        vector[i] = *it;
}   

} // namespace WTF

using WTF::HashMap;

#endif /* WTF_HashMap_h */
