/*
 * Copyright (C) 2013-2015 Apple Inc. All rights reserved.
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
#include "DFGTierUpCheckInjectionPhase.h"

#if ENABLE(DFG_JIT)

#include "DFGGraph.h"
#include "DFGInsertionSet.h"
#include "DFGNaturalLoops.h"
#include "DFGPhase.h"
#include "FTLCapabilities.h"
#include "JSCInlines.h"

namespace JSC { namespace DFG {

class TierUpCheckInjectionPhase : public Phase {
public:
    TierUpCheckInjectionPhase(Graph& graph)
        : Phase(graph, "tier-up check injection")
    {
    }
    
    bool run()
    {
        RELEASE_ASSERT(m_graph.m_plan.mode == DFGMode);
        
        if (!Options::useFTLJIT())
            return false;
        
        if (m_graph.m_profiledBlock->m_didFailFTLCompilation)
            return false;

        if (!Options::bytecodeRangeToFTLCompile().isInRange(m_graph.m_profiledBlock->instructionCount()))
            return false;

#if ENABLE(FTL_JIT)
        FTL::CapabilityLevel level = FTL::canCompile(m_graph);
        if (level == FTL::CannotCompile)
            return false;
        
        if (!Options::useOSREntryToFTL())
            level = FTL::CanCompile;
        
        m_graph.ensureNaturalLoops();
        NaturalLoops& naturalLoops = *m_graph.m_naturalLoops;
        HashMap<const NaturalLoop*, unsigned> naturalLoopToLoopHint = buildNaturalLoopToLoopHintMap(naturalLoops);

        HashMap<unsigned, LoopHintDescriptor> tierUpHierarchy;

        InsertionSet insertionSet(m_graph);
        for (BlockIndex blockIndex = m_graph.numBlocks(); blockIndex--;) {
            BasicBlock* block = m_graph.block(blockIndex);
            if (!block)
                continue;

            for (unsigned nodeIndex = 0; nodeIndex < block->size(); ++nodeIndex) {
                Node* node = block->at(nodeIndex);
                if (node->op() != LoopHint)
                    continue;

                NodeOrigin origin = node->origin;
                bool canOSREnter = canOSREnterAtLoopHint(level, block, nodeIndex);

                NodeType tierUpType = CheckTierUpAndOSREnter;
                if (!canOSREnter)
                    tierUpType = CheckTierUpInLoop;
                insertionSet.insertNode(nodeIndex + 1, SpecNone, tierUpType, origin);

                unsigned bytecodeIndex = origin.semantic.bytecodeIndex;
                if (canOSREnter)
                    m_graph.m_plan.tierUpAndOSREnterBytecodes.append(bytecodeIndex);

                if (const NaturalLoop* loop = naturalLoops.innerMostLoopOf(block)) {
                    LoopHintDescriptor descriptor;
                    descriptor.canOSREnter = canOSREnter;

                    const NaturalLoop* outerLoop = loop;
                    while ((outerLoop = naturalLoops.innerMostOuterLoop(*outerLoop))) {
                        auto it = naturalLoopToLoopHint.find(outerLoop);
                        if (it != naturalLoopToLoopHint.end())
                            descriptor.osrEntryCandidates.append(it->value);
                    }
                    if (!descriptor.osrEntryCandidates.isEmpty())
                        tierUpHierarchy.add(bytecodeIndex, WTFMove(descriptor));
                }
                break;
            }

            NodeAndIndex terminal = block->findTerminal();
            if (terminal.node->isFunctionTerminal()) {
                insertionSet.insertNode(
                    terminal.index, SpecNone, CheckTierUpAtReturn, terminal.node->origin);
            }

            insertionSet.execute(block);
        }

        // Add all the candidates that can be OSR Entered.
        for (auto entry : tierUpHierarchy) {
            Vector<unsigned> tierUpCandidates;
            for (unsigned bytecodeIndex : entry.value.osrEntryCandidates) {
                auto descriptorIt = tierUpHierarchy.find(bytecodeIndex);
                if (descriptorIt != tierUpHierarchy.end()
                    && descriptorIt->value.canOSREnter)
                    tierUpCandidates.append(bytecodeIndex);
            }

            if (!tierUpCandidates.isEmpty())
                m_graph.m_plan.tierUpInLoopHierarchy.add(entry.key, WTFMove(tierUpCandidates));
        }
        m_graph.m_plan.willTryToTierUp = true;
        return true;
#else // ENABLE(FTL_JIT)
        RELEASE_ASSERT_NOT_REACHED();
        return false;
#endif // ENABLE(FTL_JIT)
    }

private:
#if ENABLE(FTL_JIT)
    struct LoopHintDescriptor {
        Vector<unsigned> osrEntryCandidates;
        bool canOSREnter;
    };

    bool canOSREnterAtLoopHint(FTL::CapabilityLevel level, const BasicBlock* block, unsigned nodeIndex)
    {
        Node* node = block->at(nodeIndex);
        ASSERT(node->op() == LoopHint);

        NodeOrigin origin = node->origin;
        if (level != FTL::CanCompileAndOSREnter || origin.semantic.inlineCallFrame)
            return false;

        // We only put OSR checks for the first LoopHint in the block. Note that
        // more than one LoopHint could happen in cases where we did a lot of CFG
        // simplification in the bytecode parser, but it should be very rare.
        for (unsigned subNodeIndex = nodeIndex; subNodeIndex--;) {
            if (!block->at(subNodeIndex)->isSemanticallySkippable())
                return false;
        }
        return true;
    }

    HashMap<const NaturalLoop*, unsigned> buildNaturalLoopToLoopHintMap(const NaturalLoops& naturalLoops)
    {
        HashMap<const NaturalLoop*, unsigned> naturalLoopsToLoopHint;

        for (BasicBlock* block : m_graph.blocksInNaturalOrder()) {
            for (unsigned nodeIndex = 0; nodeIndex < block->size(); ++nodeIndex) {
                Node* node = block->at(nodeIndex);
                if (node->op() != LoopHint)
                    continue;

                if (const NaturalLoop* loop = naturalLoops.innerMostLoopOf(block)) {
                    unsigned bytecodeIndex = node->origin.semantic.bytecodeIndex;
                    naturalLoopsToLoopHint.add(loop, bytecodeIndex);
                }
                break;
            }
        }
        return naturalLoopsToLoopHint;
    }
#endif
};

bool performTierUpCheckInjection(Graph& graph)
{
    return runPhase<TierUpCheckInjectionPhase>(graph);
}

} } // namespace JSC::DFG

#endif // ENABLE(DFG_JIT)


