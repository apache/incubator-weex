/*
 * Copyright (C) 2015-2016 Apple Inc. All rights reserved.
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
