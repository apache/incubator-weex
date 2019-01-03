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

#include "DFGFlowIndexing.h"
#include "DFGGraph.h"
#include "DFGNode.h"

namespace JSC { namespace DFG {

// This is a mapping from nodes to values that is useful for flow-sensitive analysis. In such an
// analysis, at every point in the program we need to consider the values of nodes plus the shadow
// values of Phis. This makes it easy to do both of those things.
template<typename T>
class FlowMap {
public:
    FlowMap(Graph& graph)
        : m_graph(graph)
    {
        resize();
    }
    
    // Call this if the number of nodes in the graph has changed. Note that this does not reset any
    // entries.
    void resize()
    {
        m_map.resize(m_graph.maxNodeCount());
        m_shadowMap.resize(m_graph.maxNodeCount());
    }
    
    Graph& graph() const { return m_graph; }
    
    T& at(unsigned nodeIndex)
    {
        return m_map[nodeIndex];
    }
    
    T& at(Node* node)
    {
        return at(node->index());
    }
    
    T& atShadow(unsigned nodeIndex)
    {
        return m_shadowMap[nodeIndex];
    }
    
    T& atShadow(Node* node)
    {
        return atShadow(node->index());
    }
    
    T& at(unsigned nodeIndex, NodeFlowProjection::Kind kind)
    {
        switch (kind) {
        case NodeFlowProjection::Primary:
            return at(nodeIndex);
        case NodeFlowProjection::Shadow:
            return atShadow(nodeIndex);
        }
        RELEASE_ASSERT_NOT_REACHED();
        return *bitwise_cast<T*>(nullptr);
    }
    
    T& at(Node* node, NodeFlowProjection::Kind kind)
    {
        return at(node->index(), kind);
    }
    
    T& at(NodeFlowProjection projection)
    {
        return at(projection.node(), projection.kind());
    }
    
    const T& at(unsigned nodeIndex) const { return const_cast<FlowMap*>(this)->at(nodeIndex); }
    const T& at(Node* node) const { return const_cast<FlowMap*>(this)->at(node); }
    const T& atShadow(unsigned nodeIndex) const { return const_cast<FlowMap*>(this)->atShadow(nodeIndex); }
    const T& atShadow(Node* node) const { return const_cast<FlowMap*>(this)->atShadow(node); }
    const T& at(unsigned nodeIndex, NodeFlowProjection::Kind kind) const { return const_cast<FlowMap*>(this)->at(nodeIndex, kind); }
    const T& at(Node* node, NodeFlowProjection::Kind kind) const { return const_cast<FlowMap*>(this)->at(node, kind); }
    const T& at(NodeFlowProjection projection) const { return const_cast<FlowMap*>(this)->at(projection); }
    
private:
    Graph& m_graph;
    Vector<T, 0, UnsafeVectorOverflow> m_map;
    Vector<T, 0, UnsafeVectorOverflow> m_shadowMap;
};

} } // namespace JSC::DFG

namespace WTF {

template<typename T>
void printInternal(PrintStream& out, const JSC::DFG::FlowMap<T>& map)
{
    CommaPrinter comma;
    for (unsigned i = 0; i < map.graph().maxNodeCount(); ++i) {
        if (JSC::DFG::Node* node = map.graph().nodeAt(i)) {
            if (const T& value = map.at(node))
                out.print(comma, node, "=>", value);
        }
    }
    for (unsigned i = 0; i < map.graph().maxNodeCount(); ++i) {
        if (JSC::DFG::Node* node = map.graph().nodeAt(i)) {
            if (const T& value = map.atShadow(node))
                out.print(comma, "shadow(", node, ")=>", value);
        }
    }
}

} // namespace WTF

#endif // ENABLE(DFG_JIT)

