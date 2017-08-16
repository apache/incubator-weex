/*
 * Copyright (C) 2011, 2013 Apple Inc. All rights reserved.
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
#include "DFGVirtualRegisterAllocationPhase.h"

#if ENABLE(DFG_JIT)

#include "DFGGraph.h"
#include "DFGScoreBoard.h"
#include "JSCInlines.h"
#include "StackAlignment.h"
#include <wtf/StdLibExtras.h>

namespace JSC { namespace DFG {

class VirtualRegisterAllocationPhase : public Phase {
public:
    VirtualRegisterAllocationPhase(Graph& graph)
        : Phase(graph, "virtual register allocation")
    {
    }
    
    bool run()
    {
        DFG_ASSERT(m_graph, nullptr, m_graph.m_form == ThreadedCPS);
        
        ScoreBoard scoreBoard(m_graph.m_nextMachineLocal);
        scoreBoard.assertClear();
        for (size_t blockIndex = 0; blockIndex < m_graph.numBlocks(); ++blockIndex) {
            BasicBlock* block = m_graph.block(blockIndex);
            if (!block)
                continue;
            if (!block->isReachable)
                continue;
            if (!ASSERT_DISABLED) {
                // Force usage of highest-numbered virtual registers.
                scoreBoard.sortFree();
            }
            for (auto* node : *block) {
                if (!node->shouldGenerate())
                    continue;
                
                switch (node->op()) {
                case Phi:
                case Flush:
                case PhantomLocal:
                    continue;
                case GetLocal:
                    ASSERT(!node->child1()->hasResult());
                    break;
                default:
                    break;
                }
                
                // First, call use on all of the current node's children, then
                // allocate a VirtualRegister for this node. We do so in this
                // order so that if a child is on its last use, and a
                // VirtualRegister is freed, then it may be reused for node.
                if (node->flags() & NodeHasVarArgs) {
                    for (unsigned childIdx = node->firstChild(); childIdx < node->firstChild() + node->numChildren(); childIdx++)
                        scoreBoard.useIfHasResult(m_graph.m_varArgChildren[childIdx]);
                } else {
                    scoreBoard.useIfHasResult(node->child1());
                    scoreBoard.useIfHasResult(node->child2());
                    scoreBoard.useIfHasResult(node->child3());
                }

                if (!node->hasResult())
                    continue;

                VirtualRegister virtualRegister = scoreBoard.allocate();
                node->setVirtualRegister(virtualRegister);
                // 'mustGenerate' nodes have their useCount artificially elevated,
                // call use now to account for this.
                if (node->mustGenerate())
                    scoreBoard.use(node);
            }
            scoreBoard.assertClear();
        }
        
        // Record the number of virtual registers we're using. This is used by calls
        // to figure out where to put the parameters.
        m_graph.m_nextMachineLocal = scoreBoard.highWatermark();

        return true;
    }
};

bool performVirtualRegisterAllocation(Graph& graph)
{
    return runPhase<VirtualRegisterAllocationPhase>(graph);
}

} } // namespace JSC::DFG

#endif // ENABLE(DFG_JIT)
