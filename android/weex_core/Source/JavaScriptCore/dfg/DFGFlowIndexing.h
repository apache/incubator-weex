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

#pragma once

#if ENABLE(DFG_JIT)

#include "DFGGraph.h"
#include "DFGNodeFlowProjection.h"

namespace JSC { namespace DFG {

// This is a mapping from nodes to unique, dense indices that can be used for creating fast
// Node-keyed maps. The special part is that it also allocated indices for the shadow values of Phi
// nodes, which is needed for any flow-sensitive analysis.
class FlowIndexing {
public:
    FlowIndexing(Graph&);
    ~FlowIndexing();
    
    void recompute();
    
    Graph& graph() const { return m_graph; }
    
    unsigned numIndices() const { return m_numIndices; }
    
    unsigned index(unsigned nodeIndex) const { return nodeIndex; }
    
    unsigned index(Node* node) const { return index(node->index()); }
    
    unsigned shadowIndex(unsigned nodeIndex) const
    {
        return m_nodeIndexToShadowIndex[nodeIndex];
    }
    
    unsigned shadowIndex(Node* node) const
    {
        DFG_ASSERT(m_graph, node, node->op() == Phi);
        return shadowIndex(node->index());
    }
    
    unsigned index(unsigned nodeIndex, NodeFlowProjection::Kind kind) const
    {
        switch (kind) {
        case NodeFlowProjection::Primary:
            return index(nodeIndex);
        case NodeFlowProjection::Shadow:
            return shadowIndex(nodeIndex);
        }
        RELEASE_ASSERT_NOT_REACHED();
        return 0;
    }
    
    unsigned index(Node *node, NodeFlowProjection::Kind kind) const
    {
        switch (kind) {
        case NodeFlowProjection::Primary:
            return index(node);
        case NodeFlowProjection::Shadow:
            return shadowIndex(node);
        }
        RELEASE_ASSERT_NOT_REACHED();
        return 0;
    }
    
    unsigned index(NodeFlowProjection projection) const
    {
        return index(projection.node(), projection.kind());
    }
    
    NodeFlowProjection nodeProjection(unsigned index) const
    {
        if (index < m_nodeIndexToShadowIndex.size())
            return NodeFlowProjection(m_graph.nodeAt(index));
        return NodeFlowProjection(
            m_graph.nodeAt(m_shadowIndexToNodeIndex[index - m_nodeIndexToShadowIndex.size()]),
            NodeFlowProjection::Shadow);
    }
    
private:
    Graph& m_graph;
    unsigned m_numIndices;
    Vector<unsigned, 0, UnsafeVectorOverflow> m_nodeIndexToShadowIndex;
    Vector<unsigned, 0, UnsafeVectorOverflow> m_shadowIndexToNodeIndex;
};

} } // namespace JSC::DFG

#endif // ENABLE(DFG_JIT)

