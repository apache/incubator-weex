/*
 * Copyright (C) 2013, 2015 Apple Inc. All rights reserved.
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
#include "DFGLoopPreHeaderCreationPhase.h"

#if ENABLE(DFG_JIT)

#include "DFGBasicBlockInlines.h"
#include "DFGBlockInsertionSet.h"
#include "DFGDominators.h"
#include "DFGGraph.h"
#include "DFGNaturalLoops.h"
#include "DFGPhase.h"
#include "JSCInlines.h"
#include <wtf/HashMap.h>

namespace JSC { namespace DFG {

BasicBlock* createPreHeader(Graph& graph, BlockInsertionSet& insertionSet, BasicBlock* block)
{
    // FIXME: If we run this utility on SSA IR, then we may end up with a bizarre arrangement of
    // Upsilons and Phis, like:
    //
    // BB#1:
    //     Upsilon(@a, ^p)
    //     Jump(#3)
    //
    // BB#2:
    //     Upsilon(@b, ^p)
    //     Jump(#3)
    //
    // BB#3:
    //     Jump(#4)
    //
    // BB#4:
    //     p: Phi()
    //
    // Notice how the Upsilons are not in the predecessor of the Phi anymore. It's not clear if this
    // would be bad. Probably not, but it's weird anyway. We should add a validation rule, and we
    // should implement a Upsilon/Phi canonicalization that handles this by calling into the
    // SSACalculator and treating the Upsilons as Defs and rebuilding the Phis from scratch.
    //
    // https://bugs.webkit.org/show_bug.cgi?id=148587

    // Determine a good frequency for the pre-header. It's definitely not the frequency of the loop body.
    // Instead, we use the max of the frequencies of the loop body's non-loop predecessors.
    float frequency = 0;
    for (BasicBlock* predecessor : block->predecessors) {
        if (graph.m_dominators->dominates(block, predecessor))
            continue;
        frequency = std::max(frequency, predecessor->executionCount);
    }
    BasicBlock* preHeader = insertionSet.insertBefore(block, frequency);

    // FIXME: It would be great if we put some effort into enabling exitOK at this origin, if it
    // happens to be unset. It might not be set because the loop header (i.e. "block") has Phis in it.
    // Phis have to have exitOK=false. There are a few ways to try to set exitOK:
    //
    // - Regenerate an exit origin by proving that we are at an exit origin boundary. If all of the
    //   predecessors' terminals have different exit origins than the exit origin of head of block,
    //   then we can leverage the assumption that exit origin boundaries can always exit. We could
    //   extend this further, and say that we will set exitOK even if a predecessor's terminal has the
    //   same exit origin, but so long it hadn't done anything that clobbers exit since the start of
    //   the origin.
    //
    // - Analyze the Phi's and MovHint's at the head of block. If prior to the ExitOK there are only
    //   Phi's and MovHint's, we could "roll them back" by proving that for each of the MovHints, the
    //   referenced Phi has a child that dominates the pre-header, and that child is the node that is
    //   OSR-available at the local being MovHinted.
    //
    // Note that there are some obviously wrong ways to try to set exitOK. For example, we cannot
    // simply use the origin of our predecessors, since in bytecode that could be *any* kind of
    // instruction. It may not even be a control flow construct, if we had lowered some non-control
    // bytecode operation into DFG IR that has control flow. Hence, we really do need to try to use the
    // origin of the head of the loop header.
    //
    // https://bugs.webkit.org/show_bug.cgi?id=148586
    preHeader->appendNode(
        graph, SpecNone, Jump, block->at(0)->origin, OpInfo(block));
    
    for (unsigned predecessorIndex = 0; predecessorIndex < block->predecessors.size(); predecessorIndex++) {
        BasicBlock* predecessor = block->predecessors[predecessorIndex];
        if (graph.m_dominators->dominates(block, predecessor))
            continue;
        block->predecessors[predecessorIndex--] = block->predecessors.last();
        block->predecessors.removeLast();
        for (unsigned successorIndex = predecessor->numSuccessors(); successorIndex--;) {
            BasicBlock*& successor = predecessor->successor(successorIndex);
            if (successor != block)
                continue;
            successor = preHeader;
            preHeader->predecessors.append(predecessor);
        }
    }
    
    block->predecessors.append(preHeader);
    return preHeader;
}

class LoopPreHeaderCreationPhase : public Phase {
public:
    LoopPreHeaderCreationPhase(Graph& graph)
        : Phase(graph, "loop pre-header creation")
        , m_insertionSet(graph)
    {
    }
    
    bool run()
    {
        m_graph.ensureDominators();
        m_graph.ensureNaturalLoops();
        
        for (unsigned loopIndex = m_graph.m_naturalLoops->numLoops(); loopIndex--;) {
            const NaturalLoop& loop = m_graph.m_naturalLoops->loop(loopIndex);
            BasicBlock* existingPreHeader = 0;
            bool needsNewPreHeader = false;
            for (unsigned predecessorIndex = loop.header()->predecessors.size(); predecessorIndex--;) {
                BasicBlock* predecessor = loop.header()->predecessors[predecessorIndex];
                if (m_graph.m_dominators->dominates(loop.header(), predecessor))
                    continue;
                if (!existingPreHeader) {
                    existingPreHeader = predecessor;
                    continue;
                }
                // We won't have duplicate entries in the predecessors list.
                DFG_ASSERT(m_graph, nullptr, existingPreHeader != predecessor);
                needsNewPreHeader = true;
                break;
            }
            
            // This phase should only be run on a DFG where unreachable blocks have been pruned.
            // We also don't allow loops back to root. This means that every loop header has got
            // to have a pre-header.
            DFG_ASSERT(m_graph, nullptr, existingPreHeader);
            
            // We are looking at the predecessors of a loop header. A loop header has to have
            // some predecessor other than the pre-header. We must have broken critical edges
            // because that is the DFG SSA convention. Therefore, each predecessor of the loop
            // header must have only one successor.
            DFG_ASSERT(m_graph, nullptr, existingPreHeader->terminal()->op() == Jump);

            // A pre-header is most useful if it's possible to exit from its terminal. Hence
            // if the terminal of the existing pre-header doesn't allow for exit, but the first
            // origin of the loop header does, then we should create a new pre-header.
            if (!needsNewPreHeader && loop.header()->at(0)->origin.exitOK
                && !existingPreHeader->terminal()->origin.exitOK)
                needsNewPreHeader = true;
            
            if (!needsNewPreHeader)
                continue;
            
            createPreHeader(m_graph, m_insertionSet, loop.header());
        }
        
        return m_insertionSet.execute();
    }

    BlockInsertionSet m_insertionSet;
};

bool performLoopPreHeaderCreation(Graph& graph)
{
    return runPhase<LoopPreHeaderCreationPhase>(graph);
}

} } // namespace JSC::DFG

#endif // ENABLE(DFG_JIT)


