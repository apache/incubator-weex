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

#include <wtf/BitVector.h>
#include <wtf/CommaPrinter.h>

namespace WTF {

// This is a set for things that have an index(). It's super efficient for BasicBlocks. It's only
// efficient for Values if you don't create too many of these sets, since Values can have very sparse
// indices and there are a lot of Values.

// If you want a set of BasicBlocks, you do IndexSet<BasicBlock>. So, T = BasicBlock.
template<typename T>
class IndexSet {
public:
    IndexSet()
    {
    }

    bool add(T* value)
    {
        return !m_set.set(value->index());
    }

    template<typename Iterable>
    bool addAll(const Iterable& iterable)
    {
        bool result = false;
        for (T* value : iterable)
            result |= add(value);
        return result;
    }

    bool remove(T* value)
    {
        return m_set.clear(value->index());
    }

    bool contains(T* value) const
    {
        if (!value)
            return false;
        return m_set.get(value->index());
    }

    size_t size() const
    {
        return m_set.bitCount();
    }

    bool isEmpty() const
    {
        return !size();
    }

    template<typename CollectionType>
    class Iterable {
    public:
        Iterable(const CollectionType& collection, const BitVector& set)
            : m_collection(collection)
            , m_set(set)
        {
        }

        class iterator {
        public:
            iterator()
                : m_collection(nullptr)
            {
            }

            iterator(const CollectionType& collection, BitVector::iterator iter)
                : m_collection(&collection)
                , m_iter(iter)
            {
            }

            T* operator*()
            {
                return m_collection->at(*m_iter);
            }

            iterator& operator++()
            {
                ++m_iter;
                return *this;
            }

            bool operator==(const iterator& other) const
            {
                return m_iter == other.m_iter;
            }

            bool operator!=(const iterator& other) const
            {
                return !(*this == other);
            }

        private:
            const CollectionType* m_collection;
            BitVector::iterator m_iter;
        };

        iterator begin() const { return iterator(m_collection, m_set.begin()); }
        iterator end() const { return iterator(m_collection, m_set.end()); }

    private:
        const CollectionType& m_collection;
        const BitVector& m_set;
    };

    // For basic blocks, you do:
    // indexSet.values(procedure);
    //
    // For values, you do:
    // indexSet.values(procedure.values());
    template<typename CollectionType>
    Iterable<CollectionType> values(const CollectionType& collection) const
    {
        return Iterable<CollectionType>(collection, indices());
    }

    const BitVector& indices() const { return m_set; }

    void dump(PrintStream& out) const
    {
        CommaPrinter comma;
        for (size_t index : indices())
            out.print(comma, T::dumpPrefix, index);
    }

private:
    BitVector m_set;
};

} // namespace WTF

using WTF::IndexSet;
