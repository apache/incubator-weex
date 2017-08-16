/*
 * Copyright (C) 2014, 2015 Apple Inc. All rights reserved.
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
#include "DFGAvailabilityMap.h"

#if ENABLE(DFG_JIT)

#include "DFGGraph.h"
#include "JSCInlines.h"
#include "OperandsInlines.h"
#include <wtf/ListDump.h>

namespace JSC { namespace DFG {

void AvailabilityMap::pruneHeap()
{
    if (m_heap.isEmpty())
        return;
    
    NodeSet possibleNodes;
    
    for (unsigned i = m_locals.size(); i--;) {
        if (m_locals[i].hasNode())
            possibleNodes.addVoid(m_locals[i].node());
    }

    closeOverNodes(
        [&] (Node* node) -> bool {
            return possibleNodes.contains(node);
        },
        [&] (Node* node) -> bool {
            return possibleNodes.add(node).isNewEntry;
        });
    
    HashMap<PromotedHeapLocation, Availability> newHeap;
    for (auto pair : m_heap) {
        if (possibleNodes.contains(pair.key.base()))
            newHeap.add(pair.key, pair.value);
    }
    m_heap = newHeap;
}

void AvailabilityMap::pruneByLiveness(Graph& graph, CodeOrigin where)
{
    Operands<Availability> localsCopy(OperandsLike, m_locals);
    graph.forAllLiveInBytecode(
        where,
        [&] (VirtualRegister reg) {
            localsCopy.operand(reg) = m_locals.operand(reg);
        });
    m_locals = localsCopy;
    pruneHeap();
}

void AvailabilityMap::clear()
{
    m_locals.fill(Availability());
    m_heap.clear();
}

void AvailabilityMap::dump(PrintStream& out) const
{
    out.print("{locals = ", m_locals, "; heap = ", mapDump(m_heap), "}");
}

bool AvailabilityMap::operator==(const AvailabilityMap& other) const
{
    return m_locals == other.m_locals
        && m_heap == other.m_heap;
}

void AvailabilityMap::merge(const AvailabilityMap& other)
{
    for (unsigned i = other.m_locals.size(); i--;)
        m_locals[i] = other.m_locals[i].merge(m_locals[i]);
    
    for (auto pair : other.m_heap) {
        auto result = m_heap.add(pair.key, Availability());
        result.iterator->value = pair.value.merge(result.iterator->value);
    }
}

} } // namespace JSC::DFG

#endif // ENABLE(DFG_JIT)

