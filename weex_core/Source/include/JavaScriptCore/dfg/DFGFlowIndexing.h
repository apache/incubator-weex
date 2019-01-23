/**
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
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

