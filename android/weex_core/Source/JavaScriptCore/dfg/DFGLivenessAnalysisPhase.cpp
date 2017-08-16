/*
 * Copyright (C) 2013-2017 Apple Inc. All rights reserved.
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
#include "DFGLivenessAnalysisPhase.h"

#if ENABLE(DFG_JIT)

#include "DFGBasicBlockInlines.h"
#include "DFGBlockMapInlines.h"
#include "DFGFlowIndexing.h"
#include "DFGGraph.h"
#include "DFGInsertionSet.h"
#include "DFGPhase.h"
#include "JSCInlines.h"
#include <wtf/BitVector.h>
#include <wtf/IndexSparseSet.h>
#include <wtf/LoggingHashSet.h>

namespace JSC { namespace DFG {

// Uncomment this to log hashtable operations.
// static const char templateString[] = "unsigned, DefaultHash<unsigned>::Hash, WTF::UnsignedWithZeroKeyHashTraits<unsigned>";
// typedef LoggingHashSet<templateString, unsigned, DefaultHash<unsigned>::Hash, WTF::UnsignedWithZeroKeyHashTraits<unsigned>> LiveSet;

typedef HashSet<unsigned, DefaultHash<unsigned>::Hash, WTF::UnsignedWithZeroKeyHashTraits<unsigned>> LiveSet;

class LivenessAnalysisPhase : public Phase {
public:
    LivenessAnalysisPhase(Graph& graph)
        : Phase(graph, "liveness analysis")
        , m_dirtyBlocks(m_graph.numBlocks())
        , m_indexing(*m_graph.m_indexingCache)
        , m_liveAtHead(m_graph)
        , m_liveAtTail(m_graph)
    {
        m_graph.m_indexingCache->recompute();
        m_workset = std::make_unique<IndexSparseSet<UnsafeVectorOverflow>>(m_graph.m_indexingCache->numIndices());
    }

    bool run()
    {
        // Start with all valid block dirty.
        BlockIndex numBlock = m_graph.numBlocks();
        m_dirtyBlocks.ensureSize(numBlock);
        for (BlockIndex blockIndex = 0; blockIndex < numBlock; ++blockIndex) {
            if (m_graph.block(blockIndex))
                m_dirtyBlocks.quickSet(blockIndex);
        }

        // Fixpoint until we do not add any new live values at tail.
        bool changed;
        do {
            changed = false;

            for (BlockIndex blockIndex = numBlock; blockIndex--;) {
                if (!m_dirtyBlocks.quickClear(blockIndex))
                    continue;

                changed |= processBlock(blockIndex);
            }
        } while (changed);

        // Update the per-block node list for live and tail.
        for (BlockIndex blockIndex = numBlock; blockIndex--;) {
            BasicBlock* block = m_graph.block(blockIndex);
            if (!block)
                continue;

            {
                const Vector<unsigned, 0, UnsafeVectorOverflow, 1>& liveAtHeadIndices = m_liveAtHead[blockIndex];
                Vector<NodeFlowProjection>& liveAtHead = block->ssa->liveAtHead;
                liveAtHead.resize(0);
                liveAtHead.reserveCapacity(liveAtHeadIndices.size());
                for (unsigned index : liveAtHeadIndices)
                    liveAtHead.uncheckedAppend(m_indexing.nodeProjection(index));
            }
            {
                const LiveSet& liveAtTailIndices = m_liveAtTail[blockIndex];
                Vector<NodeFlowProjection>& liveAtTail = block->ssa->liveAtTail;
                liveAtTail.resize(0);
                liveAtTail.reserveCapacity(liveAtTailIndices.size());
                for (unsigned index : m_liveAtTail[blockIndex])
                    liveAtTail.uncheckedAppend(m_indexing.nodeProjection(index));
            }
        }

        return true;
    }

private:
    bool processBlock(BlockIndex blockIndex)
    {
        BasicBlock* block = m_graph.block(blockIndex);
        ASSERT_WITH_MESSAGE(block, "Only dirty blocks needs updates. A null block should never be dirty.");

        m_workset->clear();
        for (unsigned index : m_liveAtTail[blockIndex])
            m_workset->add(index);

        for (unsigned nodeIndex = block->size(); nodeIndex--;) {
            Node* node = block->at(nodeIndex);

            auto handleEdge = [&] (Edge& edge) {
                bool newEntry = m_workset->add(m_indexing.index(edge.node()));
                edge.setKillStatus(newEntry ? DoesKill : DoesNotKill);
            };
            
            switch (node->op()) {
            case Upsilon: {
                ASSERT_WITH_MESSAGE(!m_workset->contains(node->index()), "Upsilon should not be used as defs by other nodes.");

                Node* phi = node->phi();
                m_workset->remove(m_indexing.shadowIndex(phi));
                handleEdge(node->child1());
                break;
            }
            case Phi: {
                m_workset->remove(m_indexing.index(node));
                m_workset->add(m_indexing.shadowIndex(node));
                break;
            }
            default:
                m_workset->remove(m_indexing.index(node));
                m_graph.doToChildren(node, handleEdge);
                break;
            }
        }

        // Update live at head.
        Vector<unsigned, 0, UnsafeVectorOverflow, 1>& liveAtHead = m_liveAtHead[blockIndex];
        if (m_workset->size() == liveAtHead.size())
            return false;

        for (unsigned liveIndexAtHead : liveAtHead)
            m_workset->remove(liveIndexAtHead);
        ASSERT(!m_workset->isEmpty());

        liveAtHead.reserveCapacity(liveAtHead.size() + m_workset->size());
        for (unsigned newValue : *m_workset)
            liveAtHead.uncheckedAppend(newValue);

        bool changedPredecessor = false;
        for (BasicBlock* predecessor : block->predecessors) {
            LiveSet& liveAtTail = m_liveAtTail[predecessor];
            for (unsigned newValue : *m_workset) {
                if (liveAtTail.add(newValue)) {
                    if (!m_dirtyBlocks.quickSet(predecessor->index))
                        changedPredecessor = true;
                }
            }
        }
        return changedPredecessor;
    }

    // Blocks with new live values at tail.
    BitVector m_dirtyBlocks;
    
    FlowIndexing& m_indexing;

    // Live values per block edge.
    BlockMap<Vector<unsigned, 0, UnsafeVectorOverflow, 1>> m_liveAtHead;
    BlockMap<LiveSet> m_liveAtTail;

    // Single sparse set allocated once and used by every basic block.
    std::unique_ptr<IndexSparseSet<UnsafeVectorOverflow>> m_workset;
};

bool performLivenessAnalysis(Graph& graph)
{
    graph.packNodeIndices();

    return runPhase<LivenessAnalysisPhase>(graph);
}

} } // namespace JSC::DFG

#endif // ENABLE(DFG_JIT)

