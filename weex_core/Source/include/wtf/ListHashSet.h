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

#include <wtf/HashSet.h>

namespace WTF {

// ListHashSet: Just like HashSet, this class provides a Set
// interface - a collection of unique objects with O(1) insertion,
// removal and test for containership. However, it also has an
// order - iterating it will always give back values in the order
// in which they are added.

// Unlike iteration of most WTF Hash data structures, iteration is
// guaranteed safe against mutation of the ListHashSet, except for
// removal of the item currently pointed to by a given iterator.

template<typename Value, typename HashFunctions> class ListHashSet;

template<typename ValueArg, typename HashArg> class ListHashSetIterator;
template<typename ValueArg, typename HashArg> class ListHashSetConstIterator;

template<typename ValueArg> struct ListHashSetNode;

template<typename HashArg> struct ListHashSetNodeHashFunctions;
template<typename HashArg> struct ListHashSetTranslator;

template<typename ValueArg, typename HashArg = typename DefaultHash<ValueArg>::Hash> class ListHashSet {
    WTF_MAKE_FAST_ALLOCATED;
private:
    typedef ListHashSetNode<ValueArg> Node;

    typedef HashTraits<Node*> NodeTraits;
    typedef ListHashSetNodeHashFunctions<HashArg> NodeHash;
    typedef ListHashSetTranslator<HashArg> BaseTranslator;

    typedef HashArg HashFunctions;

public:
    typedef ValueArg ValueType;

    typedef ListHashSetIterator<ValueType, HashArg> iterator;
    typedef ListHashSetConstIterator<ValueType, HashArg> const_iterator;
    friend class ListHashSetConstIterator<ValueType, HashArg>;

    typedef std::reverse_iterator<iterator> reverse_iterator;
    typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

    typedef HashTableAddResult<iterator> AddResult;

    ListHashSet() = default;
    ListHashSet(const ListHashSet&);
    ListHashSet(ListHashSet&&);
    ListHashSet& operator=(const ListHashSet&);
    ListHashSet& operator=(ListHashSet&&);
    ~ListHashSet();

    void swap(ListHashSet&);

    unsigned size() const;
    unsigned capacity() const;
    bool isEmpty() const;

    iterator begin() { return makeIterator(m_head); }
    iterator end() { return makeIterator(nullptr); }
    const_iterator begin() const { return makeConstIterator(m_head); }
    const_iterator end() const { return makeConstIterator(nullptr); }

    reverse_iterator rbegin() { return reverse_iterator(end()); }
    reverse_iterator rend() { return reverse_iterator(begin()); }
    const_reverse_iterator rbegin() const { return const_reverse_iterator(end()); }
    const_reverse_iterator rend() const { return const_reverse_iterator(begin()); }

    ValueType& first();
    const ValueType& first() const;
    void removeFirst();
    ValueType takeFirst();

    ValueType& last();
    const ValueType& last() const;
    void removeLast();
    ValueType takeLast();

    iterator find(const ValueType&);
    const_iterator find(const ValueType&) const;
    bool contains(const ValueType&) const;

    // An alternate version of find() that finds the object by hashing and comparing
    // with some other type, to avoid the cost of type conversion.
    // The HashTranslator interface is defined in HashSet.
    // FIXME: We should reverse the order of the template arguments so that callers
    // can just pass the translator let the compiler deduce T.
    template<typename T, typename HashTranslator> iterator find(const T&);
    template<typename T, typename HashTranslator> const_iterator find(const T&) const;
    template<typename T, typename HashTranslator> bool contains(const T&) const;

    // The return value of add is a pair of an iterator to the new value's location, 
    // and a bool that is true if an new entry was added.
    AddResult add(const ValueType&);
    AddResult add(ValueType&&);

    // Add the value to the end of the collection. If the value was already in
    // the list, it is moved to the end.
    AddResult appendOrMoveToLast(const ValueType&);
    AddResult appendOrMoveToLast(ValueType&&);

    // Add the value to the beginning of the collection. If the value was already in
    // the list, it is moved to the beginning.
    AddResult prependOrMoveToFirst(const ValueType&);
    AddResult prependOrMoveToFirst(ValueType&&);

    AddResult insertBefore(const ValueType& beforeValue, const ValueType& newValue);
    AddResult insertBefore(const ValueType& beforeValue, ValueType&& newValue);
    AddResult insertBefore(iterator, const ValueType&);
    AddResult insertBefore(iterator, ValueType&&);

    bool remove(const ValueType&);
    bool remove(iterator);
    void clear();

private:
    void unlink(Node*);
    void unlinkAndDelete(Node*);
    void appendNode(Node*);
    void prependNode(Node*);
    void insertNodeBefore(Node* beforeNode, Node* newNode);
    void deleteAllNodes();
    
    iterator makeIterator(Node*);
    const_iterator makeConstIterator(Node*) const;

    HashTable<Node*, Node*, IdentityExtractor, NodeHash, NodeTraits, NodeTraits> m_impl;
    Node* m_head { nullptr };
    Node* m_tail { nullptr };
};

template<typename ValueArg> struct ListHashSetNode {
    WTF_MAKE_FAST_ALLOCATED;
public:
    template<typename T>
    ListHashSetNode(T&& value)
        : m_value(std::forward<T>(value))
    {
    }

    ValueArg m_value;
    ListHashSetNode* m_prev { nullptr };
    ListHashSetNode* m_next { nullptr };
};

template<typename HashArg> struct ListHashSetNodeHashFunctions {
    template<typename T> static unsigned hash(const T& key) { return HashArg::hash(key->m_value); }
    template<typename T> static bool equal(const T& a, const T& b) { return HashArg::equal(a->m_value, b->m_value); }
    static const bool safeToCompareToEmptyOrDeleted = false;
};

template<typename ValueArg, typename HashArg> class ListHashSetIterator {
private:
    typedef ListHashSet<ValueArg, HashArg> ListHashSetType;
    typedef ListHashSetIterator<ValueArg, HashArg> iterator;
    typedef ListHashSetConstIterator<ValueArg, HashArg> const_iterator;
    typedef ListHashSetNode<ValueArg> Node;
    typedef ValueArg ValueType;

    friend class ListHashSet<ValueArg, HashArg>;

    ListHashSetIterator(const ListHashSetType* set, Node* position) : m_iterator(set, position) { }

public:
    typedef ptrdiff_t difference_type;
    typedef ValueType value_type;
    typedef ValueType* pointer;
    typedef ValueType& reference;
    typedef std::bidirectional_iterator_tag iterator_category;

    ListHashSetIterator() { }

    // default copy, assignment and destructor are OK

    ValueType* get() const { return const_cast<ValueType*>(m_iterator.get()); }
    ValueType& operator*() const { return *get(); }
    ValueType* operator->() const { return get(); }

    iterator& operator++() { ++m_iterator; return *this; }

    // postfix ++ intentionally omitted

    iterator& operator--() { --m_iterator; return *this; }

    // postfix -- intentionally omitted

    // Comparison.
    bool operator==(const iterator& other) const { return m_iterator == other.m_iterator; }
    bool operator!=(const iterator& other) const { return m_iterator != other.m_iterator; }

    operator const_iterator() const { return m_iterator; }

private:
    Node* node() { return m_iterator.node(); }

    const_iterator m_iterator;
};

template<typename ValueArg, typename HashArg> class ListHashSetConstIterator {
private:
    typedef ListHashSet<ValueArg, HashArg> ListHashSetType;
    typedef ListHashSetIterator<ValueArg, HashArg> iterator;
    typedef ListHashSetConstIterator<ValueArg, HashArg> const_iterator;
    typedef ListHashSetNode<ValueArg> Node;
    typedef ValueArg ValueType;

    friend class ListHashSet<ValueArg, HashArg>;
    friend class ListHashSetIterator<ValueArg, HashArg>;

    ListHashSetConstIterator(const ListHashSetType* set, Node* position)
        : m_set(set)
        , m_position(position)
    {
    }

public:
    typedef ptrdiff_t difference_type;
    typedef const ValueType value_type;
    typedef const ValueType* pointer;
    typedef const ValueType& reference;
    typedef std::bidirectional_iterator_tag iterator_category;

    ListHashSetConstIterator()
    {
    }

    const ValueType* get() const
    {
        return std::addressof(m_position->m_value);
    }

    const ValueType& operator*() const { return *get(); }
    const ValueType* operator->() const { return get(); }

    const_iterator& operator++()
    {
        ASSERT(m_position);
        m_position = m_position->m_next;
        return *this;
    }

    // postfix ++ intentionally omitted

    const_iterator& operator--()
    {
        ASSERT(m_position != m_set->m_head);
        if (!m_position)
            m_position = m_set->m_tail;
        else
            m_position = m_position->m_prev;
        return *this;
    }

    // postfix -- intentionally omitted

    // Comparison.
    bool operator==(const const_iterator& other) const
    {
        return m_position == other.m_position;
    }
    bool operator!=(const const_iterator& other) const
    {
        return m_position != other.m_position;
    }

private:
    Node* node() { return m_position; }

    const ListHashSetType* m_set;
    Node* m_position;
};

template<typename HashFunctions>
struct ListHashSetTranslator {
    template<typename T> static unsigned hash(const T& key) { return HashFunctions::hash(key); }
    template<typename T, typename U> static bool equal(const T& a, const U& b) { return HashFunctions::equal(a->m_value, b); }
    template<typename T, typename U, typename V> static void translate(T*& location, U&& key, V&&)
    {
        location = new T(std::forward<U>(key));
    }
};

template<typename T, typename U>
inline ListHashSet<T, U>::ListHashSet(const ListHashSet& other)
{
    for (auto it = other.begin(), end = other.end(); it != end; ++it)
        add(*it);
}

template<typename T, typename U>
inline ListHashSet<T, U>& ListHashSet<T, U>::operator=(const ListHashSet& other)
{
    ListHashSet tmp(other);
    swap(tmp);
    return *this;
}

template<typename T, typename U>
inline ListHashSet<T, U>::ListHashSet(ListHashSet&& other)
    : m_impl(WTFMove(other.m_impl))
    , m_head(std::exchange(other.m_head, nullptr))
    , m_tail(std::exchange(other.m_tail, nullptr))
{
}

template<typename T, typename U>
inline ListHashSet<T, U>& ListHashSet<T, U>::operator=(ListHashSet&& other)
{
    m_impl = WTFMove(other.m_impl);
    m_head = std::exchange(other.m_head, nullptr);
    m_tail = std::exchange(other.m_tail, nullptr);
    return *this;
}

template<typename T, typename U>
inline void ListHashSet<T, U>::swap(ListHashSet& other)
{
    m_impl.swap(other.m_impl);
    std::swap(m_head, other.m_head);
    std::swap(m_tail, other.m_tail);
}

template<typename T, typename U>
inline ListHashSet<T, U>::~ListHashSet()
{
    deleteAllNodes();
}

template<typename T, typename U>
inline unsigned ListHashSet<T, U>::size() const
{
    return m_impl.size(); 
}

template<typename T, typename U>
inline unsigned ListHashSet<T, U>::capacity() const
{
    return m_impl.capacity(); 
}

template<typename T, typename U>
inline bool ListHashSet<T, U>::isEmpty() const
{
    return m_impl.isEmpty(); 
}

template<typename T, typename U>
inline T& ListHashSet<T, U>::first()
{
    ASSERT(!isEmpty());
    return m_head->m_value;
}

template<typename T, typename U>
inline void ListHashSet<T, U>::removeFirst()
{
    takeFirst();
}

template<typename T, typename U>
inline T ListHashSet<T, U>::takeFirst()
{
    ASSERT(!isEmpty());
    auto it = m_impl.find(m_head);

    T result = WTFMove((*it)->m_value);
    m_impl.remove(it);
    unlinkAndDelete(m_head);

    return result;
}

template<typename T, typename U>
inline const T& ListHashSet<T, U>::first() const
{
    ASSERT(!isEmpty());
    return m_head->m_value;
}

template<typename T, typename U>
inline T& ListHashSet<T, U>::last()
{
    ASSERT(!isEmpty());
    return m_tail->m_value;
}

template<typename T, typename U>
inline const T& ListHashSet<T, U>::last() const
{
    ASSERT(!isEmpty());
    return m_tail->m_value;
}

template<typename T, typename U>
inline void ListHashSet<T, U>::removeLast()
{
    takeLast();
}

template<typename T, typename U>
inline T ListHashSet<T, U>::takeLast()
{
    ASSERT(!isEmpty());
    auto it = m_impl.find(m_tail);

    T result = WTFMove((*it)->m_value);
    m_impl.remove(it);
    unlinkAndDelete(m_tail);

    return result;
}

template<typename T, typename U>
inline auto ListHashSet<T, U>::find(const ValueType& value) -> iterator
{
    auto it = m_impl.template find<BaseTranslator>(value);
    if (it == m_impl.end())
        return end();
    return makeIterator(*it); 
}

template<typename T, typename U>
inline auto ListHashSet<T, U>::find(const ValueType& value) const -> const_iterator
{
    auto it = m_impl.template find<BaseTranslator>(value);
    if (it == m_impl.end())
        return end();
    return makeConstIterator(*it);
}

template<typename Translator>
struct ListHashSetTranslatorAdapter {
    template<typename T> static unsigned hash(const T& key) { return Translator::hash(key); }
    template<typename T, typename U> static bool equal(const T& a, const U& b) { return Translator::equal(a->m_value, b); }
};

template<typename ValueType, typename U>
template<typename T, typename HashTranslator>
inline auto ListHashSet<ValueType, U>::find(const T& value) -> iterator
{
    auto it = m_impl.template find<ListHashSetTranslatorAdapter<HashTranslator>>(value);
    if (it == m_impl.end())
        return end();
    return makeIterator(*it);
}

template<typename ValueType, typename U>
template<typename T, typename HashTranslator>
inline auto ListHashSet<ValueType, U>::find(const T& value) const -> const_iterator
{
    auto it = m_impl.template find<ListHashSetTranslatorAdapter<HashTranslator>>(value);
    if (it == m_impl.end())
        return end();
    return makeConstIterator(*it);
}

template<typename ValueType, typename U>
template<typename T, typename HashTranslator>
inline bool ListHashSet<ValueType, U>::contains(const T& value) const
{
    return m_impl.template contains<ListHashSetTranslatorAdapter<HashTranslator>>(value);
}

template<typename T, typename U>
inline bool ListHashSet<T, U>::contains(const ValueType& value) const
{
    return m_impl.template contains<BaseTranslator>(value);
}

template<typename T, typename U>
auto ListHashSet<T, U>::add(const ValueType& value) -> AddResult
{
    auto result = m_impl.template add<BaseTranslator>(value, nullptr);
    if (result.isNewEntry)
        appendNode(*result.iterator);
    return AddResult(makeIterator(*result.iterator), result.isNewEntry);
}

template<typename T, typename U>
auto ListHashSet<T, U>::add(ValueType&& value) -> AddResult
{
    auto result = m_impl.template add<BaseTranslator>(WTFMove(value), nullptr);
    if (result.isNewEntry)
        appendNode(*result.iterator);
    return AddResult(makeIterator(*result.iterator), result.isNewEntry);
}

template<typename T, typename U>
auto ListHashSet<T, U>::appendOrMoveToLast(const ValueType& value) -> AddResult
{
    auto result = m_impl.template add<BaseTranslator>(value, nullptr);
    Node* node = *result.iterator;
    if (!result.isNewEntry)
        unlink(node);
    appendNode(node);

    return AddResult(makeIterator(*result.iterator), result.isNewEntry);
}

template<typename T, typename U>
auto ListHashSet<T, U>::appendOrMoveToLast(ValueType&& value) -> AddResult
{
    auto result = m_impl.template add<BaseTranslator>(WTFMove(value), nullptr);
    Node* node = *result.iterator;
    if (!result.isNewEntry)
        unlink(node);
    appendNode(node);

    return AddResult(makeIterator(*result.iterator), result.isNewEntry);
}

template<typename T, typename U>
auto ListHashSet<T, U>::prependOrMoveToFirst(const ValueType& value) -> AddResult
{
    auto result = m_impl.template add<BaseTranslator>(value, nullptr);
    Node* node = *result.iterator;
    if (!result.isNewEntry)
        unlink(node);
    prependNode(node);

    return AddResult(makeIterator(*result.iterator), result.isNewEntry);
}

template<typename T, typename U>
auto ListHashSet<T, U>::prependOrMoveToFirst(ValueType&& value) -> AddResult
{
    auto result = m_impl.template add<BaseTranslator>(WTFMove(value), nullptr);
    Node* node = *result.iterator;
    if (!result.isNewEntry)
        unlink(node);
    prependNode(node);

    return AddResult(makeIterator(*result.iterator), result.isNewEntry);
}

template<typename T, typename U>
auto ListHashSet<T, U>::insertBefore(const ValueType& beforeValue, const ValueType& newValue) -> AddResult
{
    return insertBefore(find(beforeValue), newValue);
}

template<typename T, typename U>
auto ListHashSet<T, U>::insertBefore(const ValueType& beforeValue, ValueType&& newValue) -> AddResult
{
    return insertBefore(find(beforeValue), WTFMove(newValue));
}

template<typename T, typename U>
auto ListHashSet<T, U>::insertBefore(iterator it, const ValueType& newValue) -> AddResult
{
    auto result = m_impl.template add<BaseTranslator>(newValue, nullptr);
    if (result.isNewEntry)
        insertNodeBefore(it.node(), *result.iterator);
    return AddResult(makeIterator(*result.iterator), result.isNewEntry);
}

template<typename T, typename U>
auto ListHashSet<T, U>::insertBefore(iterator it, ValueType&& newValue) -> AddResult
{
    auto result = m_impl.template add<BaseTranslator>(WTFMove(newValue), nullptr);
    if (result.isNewEntry)
        insertNodeBefore(it.node(), *result.iterator);
    return AddResult(makeIterator(*result.iterator), result.isNewEntry);
}

template<typename T, typename U>
inline bool ListHashSet<T, U>::remove(iterator it)
{
    if (it == end())
        return false;
    m_impl.remove(it.node());
    unlinkAndDelete(it.node());
    return true;
}

template<typename T, typename U>
inline bool ListHashSet<T, U>::remove(const ValueType& value)
{
    return remove(find(value));
}

template<typename T, typename U>
inline void ListHashSet<T, U>::clear()
{
    deleteAllNodes();
    m_impl.clear(); 
    m_head = nullptr;
    m_tail = nullptr;
}

template<typename T, typename U>
void ListHashSet<T, U>::unlink(Node* node)
{
    if (!node->m_prev) {
        ASSERT(node == m_head);
        m_head = node->m_next;
    } else {
        ASSERT(node != m_head);
        node->m_prev->m_next = node->m_next;
    }

    if (!node->m_next) {
        ASSERT(node == m_tail);
        m_tail = node->m_prev;
    } else {
        ASSERT(node != m_tail);
        node->m_next->m_prev = node->m_prev;
    }
}

template<typename T, typename U>
void ListHashSet<T, U>::unlinkAndDelete(Node* node)
{
    unlink(node);
    delete node;
}

template<typename T, typename U>
void ListHashSet<T, U>::appendNode(Node* node)
{
    node->m_prev = m_tail;
    node->m_next = nullptr;

    if (m_tail) {
        ASSERT(m_head);
        m_tail->m_next = node;
    } else {
        ASSERT(!m_head);
        m_head = node;
    }

    m_tail = node;
}

template<typename T, typename U>
void ListHashSet<T, U>::prependNode(Node* node)
{
    node->m_prev = nullptr;
    node->m_next = m_head;

    if (m_head)
        m_head->m_prev = node;
    else
        m_tail = node;

    m_head = node;
}

template<typename T, typename U>
void ListHashSet<T, U>::insertNodeBefore(Node* beforeNode, Node* newNode)
{
    if (!beforeNode)
        return appendNode(newNode);
    
    newNode->m_next = beforeNode;
    newNode->m_prev = beforeNode->m_prev;
    if (beforeNode->m_prev)
        beforeNode->m_prev->m_next = newNode;
    beforeNode->m_prev = newNode;

    if (!newNode->m_prev)
        m_head = newNode;
}

template<typename T, typename U>
void ListHashSet<T, U>::deleteAllNodes()
{
    if (!m_head)
        return;

    for (Node* node = m_head, *next = m_head->m_next; node; node = next, next = node ? node->m_next : nullptr)
        delete node;
}

template<typename T, typename U>
inline auto ListHashSet<T, U>::makeIterator(Node* position) -> iterator
{
    return iterator(this, position);
}

template<typename T, typename U>
inline auto ListHashSet<T, U>::makeConstIterator(Node* position) const -> const_iterator
{ 
    return const_iterator(this, position);
}

} // namespace WTF

using WTF::ListHashSet;
