/*
 * Copyright (C) 2013, 2014 Apple Inc. All rights reserved.
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

#include "config.h"
#include "DFGClobberSet.h"

#if ENABLE(DFG_JIT)

#include "DFGClobberize.h"
#include "JSCInlines.h"
#include <wtf/ListDump.h>

namespace JSC { namespace DFG {

ClobberSet::ClobberSet() { }
ClobberSet::~ClobberSet() { }

void ClobberSet::add(AbstractHeap heap)
{
    HashMap<AbstractHeap, bool>::AddResult result = m_clobbers.add(heap, true);
    if (!result.isNewEntry) {
        if (result.iterator->value)
            return;
        result.iterator->value = true;
    }
    while (heap.kind() != World) {
        heap = heap.supertype();
        if (!m_clobbers.add(heap, false).isNewEntry)
            return;
    }
}

void ClobberSet::addAll(const ClobberSet& other)
{
    // If the other set has a direct heap, we make sure we have it and we set its
    // value to be true.
    //
    // If the other heap has a super heap, we make sure it's present but don't
    // modify its value - so we had it directly already then this doesn't change.
    
    if (this == &other)
        return;
    
    HashMap<AbstractHeap, bool>::const_iterator iter = other.m_clobbers.begin();
    HashMap<AbstractHeap, bool>::const_iterator end = other.m_clobbers.end();
    for (; iter != end; ++iter)
        m_clobbers.add(iter->key, iter->value).iterator->value |= iter->value;
}

bool ClobberSet::contains(AbstractHeap heap) const
{
    HashMap<AbstractHeap, bool>::const_iterator iter = m_clobbers.find(heap);
    if (iter == m_clobbers.end())
        return false;
    return iter->value;
}

bool ClobberSet::overlaps(AbstractHeap heap) const
{
    if (m_clobbers.find(heap) != m_clobbers.end())
        return true;
    if (heap.kind() == DOMState && !heap.payload().isTop()) {
        // DOMState heap has its own hierarchy. For direct heap clobbers that payload is not Top,
        // we should query whether the clobber overlaps with the given heap.
        DOMJIT::HeapRange range = DOMJIT::HeapRange::fromRaw(heap.payload().value32());
        for (auto pair : m_clobbers) {
            bool direct = pair.value;
            if (!direct)
                continue;
            AbstractHeap clobber = pair.key;
            if (clobber.kind() != DOMState)
                continue;
            if (clobber.payload().isTop())
                return true;
            if (DOMJIT::HeapRange::fromRaw(clobber.payload().value32()).overlaps(range))
                return true;
        }
    }
    while (heap.kind() != World) {
        heap = heap.supertype();
        if (contains(heap))
            return true;
    }
    return false;
}

void ClobberSet::clear()
{
    m_clobbers.clear();
}

HashSet<AbstractHeap> ClobberSet::direct() const
{
    return setOf(true);
}

HashSet<AbstractHeap> ClobberSet::super() const
{
    return setOf(false);
}

void ClobberSet::dump(PrintStream& out) const
{
    out.print("(Direct:[", sortedListDump(direct()), "], Super:[", sortedListDump(super()), "])");
}

HashSet<AbstractHeap> ClobberSet::setOf(bool direct) const
{
    HashSet<AbstractHeap> result;
    for (auto& clobber : m_clobbers) {
        if (clobber.value == direct)
            result.add(clobber.key);
    }
    return result;
}

void addReads(Graph& graph, Node* node, ClobberSet& readSet)
{
    ClobberSetAdd addRead(readSet);
    NoOpClobberize noOp;
    clobberize(graph, node, addRead, noOp, noOp);
}

void addWrites(Graph& graph, Node* node, ClobberSet& writeSet)
{
    NoOpClobberize noOp;
    ClobberSetAdd addWrite(writeSet);
    clobberize(graph, node, noOp, addWrite, noOp);
}

void addReadsAndWrites(Graph& graph, Node* node, ClobberSet& readSet, ClobberSet& writeSet)
{
    ClobberSetAdd addRead(readSet);
    ClobberSetAdd addWrite(writeSet);
    NoOpClobberize noOp;
    clobberize(graph, node, addRead, addWrite, noOp);
}

bool readsOverlap(Graph& graph, Node* node, ClobberSet& readSet)
{
    ClobberSetOverlaps addRead(readSet);
    NoOpClobberize noOp;
    clobberize(graph, node, addRead, noOp, noOp);
    return addRead.result();
}

bool writesOverlap(Graph& graph, Node* node, ClobberSet& writeSet)
{
    NoOpClobberize noOp;
    ClobberSetOverlaps addWrite(writeSet);
    clobberize(graph, node, noOp, addWrite, noOp);
    return addWrite.result();
}

} } // namespace JSC::DFG

#endif // ENABLE(DFG_JIT)

