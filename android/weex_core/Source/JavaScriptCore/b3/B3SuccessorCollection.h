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

#if ENABLE(B3_JIT)

namespace JSC { namespace B3 {

// This is a generic wrapper around lists of frequented blocks, which gives you just the blocks.

template<typename BasicBlock, typename SuccessorList>
class SuccessorCollection {
public:
    SuccessorCollection(SuccessorList& list)
        : m_list(list)
    {
    }

    size_t size() const { return m_list.size(); }
    BasicBlock* at(size_t index) const { return m_list[index].block(); }
    BasicBlock*& at(size_t index) { return m_list[index].block(); }
    BasicBlock* operator[](size_t index) const { return at(index); }
    BasicBlock*& operator[](size_t index) { return at(index); }

    class iterator {
    public:
        iterator()
            : m_collection(nullptr)
            , m_index(0)
        {
        }

        iterator(SuccessorCollection& collection, size_t index)
            : m_collection(&collection)
            , m_index(index)
        {
        }

        BasicBlock*& operator*() const
        {
            return m_collection->at(m_index);
        }

        iterator& operator++()
        {
            m_index++;
            return *this;
        }

        bool operator==(const iterator& other) const
        {
            ASSERT(m_collection == other.m_collection);
            return m_index == other.m_index;
        }

        bool operator!=(const iterator& other) const
        {
            return !(*this == other);
        }

    private:
        SuccessorCollection* m_collection;
        size_t m_index;
    };

    iterator begin() { return iterator(*this, 0); }
    iterator end() { return iterator(*this, size()); }

    class const_iterator {
    public:
        const_iterator()
            : m_collection(nullptr)
            , m_index(0)
        {
        }

        const_iterator(const SuccessorCollection& collection, size_t index)
            : m_collection(&collection)
            , m_index(index)
        {
        }

        BasicBlock* operator*() const
        {
            return m_collection->at(m_index);
        }

        const_iterator& operator++()
        {
            m_index++;
            return *this;
        }

        bool operator==(const const_iterator& other) const
        {
            ASSERT(m_collection == other.m_collection);
            return m_index == other.m_index;
        }

        bool operator!=(const const_iterator& other) const
        {
            return !(*this == other);
        }

    private:
        const SuccessorCollection* m_collection;
        size_t m_index;
    };

    const_iterator begin() const { return const_iterator(*this, 0); }
    const_iterator end() const { return const_iterator(*this, size()); }

private:
    SuccessorList& m_list;
};

} } // namespace JSC::B3

#endif // ENABLE(B3_JIT)
