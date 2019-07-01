/*
 * Copyright (C) 2014 Apple Inc. All rights reserved.
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

#if ENABLE(DFG_JIT)

#include "DFGBasicBlock.h"
#include <wtf/BitVector.h>

namespace JSC { namespace DFG {

class Graph;

class BlockSet {
public:
    BlockSet() { }
    
    // Return true if the block was added, false if it was already present.
    bool add(BasicBlock* block)
    {
        return !m_set.set(block->index);
    }
    
    bool contains(BasicBlock* block) const
    {
        if (!block)
            return false;
        return m_set.get(block->index);
    }
    
    class iterator {
    public:
        iterator()
            : m_graph(nullptr)
            , m_set(nullptr)
            , m_index(0)
        {
        }
        
        iterator& operator++()
        {
            m_index = m_set->m_set.findBit(m_index + 1, true);
            return *this;
        }
        
        BasicBlock* operator*() const;
        
        bool operator==(const iterator& other) const
        {
            return m_index == other.m_index;
        }
        
        bool operator!=(const iterator& other) const
        {
            return !(*this == other);
        }
        
    private:
        friend class BlockSet;
        
        Graph* m_graph;
        const BlockSet* m_set;
        size_t m_index;
    };
    
    class Iterable {
    public:
        Iterable(Graph& graph, const BlockSet& set)
            : m_graph(graph)
            , m_set(set)
        {
        }
        
        iterator begin() const
        {
            iterator result;
            result.m_graph = &m_graph;
            result.m_set = &m_set;
            result.m_index = m_set.m_set.findBit(0, true);
            return result;
        }
        
        iterator end() const
        {
            iterator result;
            result.m_graph = &m_graph;
            result.m_set = &m_set;
            result.m_index = m_set.m_set.size();
            return result;
        }
        
    private:
        Graph& m_graph;
        const BlockSet& m_set;
    };
    
    Iterable iterable(Graph& graph) const
    {
        return Iterable(graph, *this);
    }
    
    void dump(PrintStream&) const;
    
private:
    BitVector m_set;
};

class BlockAdder {
public:
    BlockAdder(BlockSet& set)
        : m_set(set)
    {
    }
    
    bool operator()(BasicBlock* block) const
    {
        return m_set.add(block);
    }
private:
    BlockSet& m_set;
};

} } // namespace JSC::DFG

#endif // ENABLE(DFG_JIT)
