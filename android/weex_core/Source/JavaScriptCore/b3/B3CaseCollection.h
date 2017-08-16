/*
 * Copyright (C) 2016 Apple Inc. All rights reserved.
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

#include "B3SwitchCase.h"

namespace JSC { namespace B3 {

class BasicBlock;
class SwitchValue;

// NOTE: You'll always want to include B3CaseCollectionInlines.h when you use this.

class CaseCollection {
public:
    CaseCollection()
    {
    }
    
    CaseCollection(const SwitchValue* terminal, const BasicBlock* owner)
        : m_switch(terminal)
        , m_owner(owner)
    {
    }
    
    const FrequentedBlock& fallThrough() const;

    unsigned size() const;
    SwitchCase at(unsigned index) const;
    
    SwitchCase operator[](unsigned index) const
    {
        return at(index);
    }

    class iterator {
    public:
        iterator()
            : m_collection(nullptr)
            , m_index(0)
        {
        }

        iterator(const CaseCollection& collection, unsigned index)
            : m_collection(&collection)
            , m_index(index)
        {
        }

        SwitchCase operator*()
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
        const CaseCollection* m_collection;
        unsigned m_index;
    };

    typedef iterator const_iterator;

    iterator begin() const { return iterator(*this, 0); }
    iterator end() const { return iterator(*this, size()); }
    
    void dump(PrintStream&) const;
    
private:
    const SwitchValue* m_switch { nullptr };
    const BasicBlock* m_owner { nullptr };
};

} } // namespace JSC::B3

#endif // ENABLE(B3_JIT)
