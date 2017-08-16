/*
 * Copyright (C) 2013 Apple Inc. All rights reserved.
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

#include "DFGAbstractHeap.h"
#include <wtf/HashMap.h>
#include <wtf/HashSet.h>
#include <wtf/PrintStream.h>

namespace JSC { namespace DFG {

class Graph;
struct Node;

// FIXME: If we ever want to compare if two nodes clobber each other, we should
// have a SmallClobberSet, which just keeps an array of the AbstractHeaps and
// satisfies overlaps() requests by looping over all of them. This will probably
// be faster than a full HashMap in a lot of cases. Or, maybe, we could have
// ClobberSet be smart and use a vector so long as it was small.

class ClobberSet {
public:
    ClobberSet();
    ~ClobberSet();
    
    bool isEmpty() const { return m_clobbers.isEmpty(); }
    
    void add(AbstractHeap);
    void addAll(const ClobberSet&);
    bool overlaps(AbstractHeap) const;
    void clear();
    
    // Calls useful for debugging the ClobberSet.
    // Do not call for non debugging purpose. Otherwise, you must handle DOMState hierarchy carefully.
    
    HashSet<AbstractHeap> direct() const;
    HashSet<AbstractHeap> super() const;
    
    void dump(PrintStream&) const;
    
private:
    bool contains(AbstractHeap) const;

    HashSet<AbstractHeap> setOf(bool direct) const;
    
    // Maps heap to:
    // true --> it's a direct clobber
    // false --> it's just a supertype of a direct clobber
    HashMap<AbstractHeap, bool> m_clobbers;
};

class ClobberSetAdd {
public:
    ClobberSetAdd(ClobberSet& set)
        : m_set(set)
    {
    }
    
    void operator()(AbstractHeap heap) const
    {
        m_set.add(heap);
    }
private:
    ClobberSet& m_set;
};

class ClobberSetOverlaps {
public:
    ClobberSetOverlaps(const ClobberSet& set)
        : m_set(set)
        , m_result(false)
    {
    }
    
    void operator()(AbstractHeap heap) const
    {
        m_result |= m_set.overlaps(heap);
    }
    
    bool result() const { return m_result; }
    
private:
    const ClobberSet& m_set;
    mutable bool m_result;
};

void addReads(Graph&, Node*, ClobberSet&);
void addWrites(Graph&, Node*, ClobberSet&);
void addReadsAndWrites(Graph&, Node*, ClobberSet& reads, ClobberSet& writes);

bool readsOverlap(Graph&, Node*, ClobberSet&);
bool writesOverlap(Graph&, Node*, ClobberSet&);

} } // namespace JSC::DFG

#endif // ENABLE(DFG_JIT)
