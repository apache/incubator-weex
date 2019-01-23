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

#include "DFGBasicBlock.h"
#include "DFGBlockMapInlines.h"
#include "DFGBlockSet.h"
#include "DFGGraph.h"

namespace JSC { namespace DFG {

class CFG {
    WTF_MAKE_NONCOPYABLE(CFG);
    WTF_MAKE_FAST_ALLOCATED;
public:
    typedef BasicBlock* Node;
    typedef BlockSet Set;
    template<typename T> using Map = BlockMap<T>;
    typedef BlockList List;

    CFG(Graph& graph)
        : m_graph(graph)
    {
    }

    Node root() { return m_graph.block(0); }

    template<typename T>
    Map<T> newMap() { return BlockMap<T>(m_graph); }

    DFG::Node::SuccessorsIterable successors(Node node) { return node->successors(); }
    PredecessorList& predecessors(Node node) { return node->predecessors; }

    unsigned index(Node node) const { return node->index; }
    Node node(unsigned index) const { return m_graph.block(index); }
    unsigned numNodes() const { return m_graph.numBlocks(); }

    PointerDump<BasicBlock> dump(Node node) const { return pointerDump(node); }

    void dump(PrintStream& out) const
    {
        m_graph.dump(out);
    }

private:
    Graph& m_graph;
};

} } // namespace JSC::DFG

#endif // ENABLE(DFG_JIT)
