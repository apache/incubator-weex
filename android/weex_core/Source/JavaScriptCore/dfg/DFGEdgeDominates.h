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

#include "DFGDominators.h"
#include "DFGGraph.h"

namespace JSC { namespace DFG {

class EdgeDominates {
    static const bool verbose = false;
    
public:
    EdgeDominates(Graph& graph, BasicBlock* block)
        : m_graph(graph)
        , m_block(block)
        , m_result(true)
    {
    }
    
    void operator()(Node*, Edge edge)
    {
        bool result = m_graph.m_dominators->dominates(edge.node()->owner, m_block);
        if (verbose) {
            dataLog(
                "Checking if ", edge, " in ", *edge.node()->owner,
                " dominates ", *m_block, ": ", result, "\n");
        }
        m_result &= result;
    }
    
    bool result() const { return m_result; }

private:
    Graph& m_graph;
    BasicBlock* m_block;
    bool m_result;
};

inline bool edgesDominate(Graph& graph, Node* node, BasicBlock* block)
{
    EdgeDominates edgeDominates(graph, block);
    DFG_NODE_DO_TO_CHILDREN(graph, node, edgeDominates);
    return edgeDominates.result();
}

} } // namespace JSC::DFG

#endif // ENABLE(DFG_JIT)
