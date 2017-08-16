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
#include "DFGStaticExecutionCountEstimationPhase.h"

#if ENABLE(DFG_JIT)

#include "DFGBasicBlockInlines.h"
#include "DFGGraph.h"
#include "DFGNaturalLoops.h"
#include "DFGPhase.h"
#include "JSCInlines.h"

namespace JSC { namespace DFG {

class StaticExecutionCountEstimationPhase : public Phase {
public:
    StaticExecutionCountEstimationPhase(Graph& graph)
        : Phase(graph, "static execution count estimation")
    {
    }
    
    bool run()
    {
        m_graph.ensureNaturalLoops();
        
        // Estimate basic block execution counts based on loop depth.
        for (BlockIndex blockIndex = m_graph.numBlocks(); blockIndex--;) {
            BasicBlock* block = m_graph.block(blockIndex);
            if (!block)
                continue;

            block->executionCount = pow(10, m_graph.m_naturalLoops->loopDepth(block));
        }
        
        // Estimate branch weights based on execution counts. This isn't quite correct. It'll
        // assume that each block's conditional successor only has that block as its
        // predecessor.
        for (BlockIndex blockIndex = m_graph.numBlocks(); blockIndex--;) {
            BasicBlock* block = m_graph.block(blockIndex);
            if (!block)
                continue;
            
            Node* terminal = block->terminal();
            switch (terminal->op()) {
            case Branch: {
                BranchData* data = terminal->branchData();
                applyCounts(data->taken);
                applyCounts(data->notTaken);
                break;
            }
                
            case Switch: {
                SwitchData* data = terminal->switchData();
                for (unsigned i = data->cases.size(); i--;)
                    applyCounts(data->cases[i].target);
                applyCounts(data->fallThrough);
                break;
            }
                
            default:
                break;
            }
        }
        
        return true;
    }

private:
    void applyCounts(BranchTarget& target)
    {
        target.count = target.block->executionCount;
    }
};

bool performStaticExecutionCountEstimation(Graph& graph)
{
    return runPhase<StaticExecutionCountEstimationPhase>(graph);
}

} } // namespace JSC::DFG

#endif // ENABLE(DFG_JIT)


