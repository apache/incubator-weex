/*
 * Copyright (C) 2013, 2016 Apple Inc. All rights reserved.
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
#include "DFGAtTailAbstractState.h"
#include "DFGBlockMapInlines.h"

#if ENABLE(DFG_JIT)

#include "JSCInlines.h"

namespace JSC { namespace DFG {

AtTailAbstractState::AtTailAbstractState(Graph& graph)
    : m_graph(graph)
    , m_valuesAtTailMap(m_graph)
{
    for (BasicBlock* block : graph.blocksInNaturalOrder()) {
        auto& valuesAtTail = m_valuesAtTailMap.at(block);
        valuesAtTail.clear();
        for (auto& valueAtTailPair : block->ssa->valuesAtTail)
            valuesAtTail.add(valueAtTailPair.node, valueAtTailPair.value);
    }
}

AtTailAbstractState::~AtTailAbstractState() { }

void AtTailAbstractState::createValueForNode(NodeFlowProjection node)
{
    m_valuesAtTailMap.at(m_block).add(node, AbstractValue());
}

AbstractValue& AtTailAbstractState::forNode(NodeFlowProjection node)
{
    auto& valuesAtTail = m_valuesAtTailMap.at(m_block);
    HashMap<NodeFlowProjection, AbstractValue>::iterator iter = valuesAtTail.find(node);
    DFG_ASSERT(m_graph, node.node(), iter != valuesAtTail.end());
    return iter->value;
}

} } // namespace JSC::DFG

#endif // ENABLE(DFG_JIT)

