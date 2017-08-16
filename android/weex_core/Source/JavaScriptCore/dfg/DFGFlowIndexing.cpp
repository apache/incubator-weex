/*
 * Copyright (C) 2016 Apple Inc. All Rights Reserved.
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
#include "DFGFlowIndexing.h"

#if ENABLE(DFG_JIT)

namespace JSC { namespace DFG {

FlowIndexing::FlowIndexing(Graph& graph)
    : m_graph(graph)
    , m_numIndices(0)
{
    recompute();
}

FlowIndexing::~FlowIndexing()
{
}

void FlowIndexing::recompute()
{
    unsigned numNodeIndices = m_graph.maxNodeCount();
    
    m_nodeIndexToShadowIndex.resize(numNodeIndices);
    m_nodeIndexToShadowIndex.fill(UINT_MAX);
    
    m_shadowIndexToNodeIndex.resize(0);

    m_numIndices = numNodeIndices;

    for (BasicBlock* block : m_graph.blocksInNaturalOrder()) {
        for (Node* node : *block) {
            if (node->op() != Phi)
                continue;
            
            unsigned nodeIndex = node->index();
            unsigned shadowIndex = m_numIndices++;
            m_nodeIndexToShadowIndex[nodeIndex] = shadowIndex;
            m_shadowIndexToNodeIndex.append(nodeIndex);
            DFG_ASSERT(m_graph, nullptr, m_shadowIndexToNodeIndex.size() + numNodeIndices == m_numIndices);
            DFG_ASSERT(m_graph, nullptr, m_shadowIndexToNodeIndex[shadowIndex - numNodeIndices] == nodeIndex);
        }
    }
}

} } // namespace JSC::DFG

#endif // ENABLE(DFG_JIT)

