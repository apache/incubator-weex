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

#include "config.h"
#include "DFGCriticalEdgeBreakingPhase.h"

#if ENABLE(DFG_JIT)

#include "DFGBasicBlockInlines.h"
#include "DFGBlockInsertionSet.h"
#include "DFGGraph.h"
#include "DFGPhase.h"
#include "JSCInlines.h"

namespace JSC { namespace DFG {

class CriticalEdgeBreakingPhase : public Phase {
public:
    CriticalEdgeBreakingPhase(Graph& graph)
        : Phase(graph, "critical edge breaking")
        , m_insertionSet(graph)
    {
    }
    
    bool run()
    {
        for (BlockIndex blockIndex = 0; blockIndex < m_graph.numBlocks(); ++blockIndex) {
            BasicBlock* block = m_graph.block(blockIndex);
            if (!block)
                continue;
            
            // An edge A->B is critical if A has multiple successor and B has multiple
            // predecessors. Thus we fail early if we don't have multiple successors.
            
            if (block->numSuccessors() <= 1)
                continue;
            
            for (unsigned i = block->numSuccessors(); i--;) {
                BasicBlock** successor = &block->successor(i);
                if ((*successor)->predecessors.size() <= 1)
                    continue;
                
                breakCriticalEdge(block, successor); 
            }
        }
        
        return m_insertionSet.execute();
    }

private:
    void breakCriticalEdge(BasicBlock* predecessor, BasicBlock** successor)
    {
        BasicBlock* pad = m_insertionSet.insertBefore(*successor, (*successor)->executionCount);
        pad->appendNode(
            m_graph, SpecNone, Jump, (*successor)->at(0)->origin, OpInfo(*successor));
        pad->predecessors.append(predecessor);
        (*successor)->replacePredecessor(predecessor, pad);
        
        *successor = pad;
    }
    
    BlockInsertionSet m_insertionSet;
};

bool performCriticalEdgeBreaking(Graph& graph)
{
    return runPhase<CriticalEdgeBreakingPhase>(graph);
}

} } // namespace JSC::DFG

#endif // ENABLE(DFG_JIT)


