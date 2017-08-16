/*
 * Copyright (C) 2016 Apple Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "DFGStoreBarrierClusteringPhase.h"

#if ENABLE(DFG_JIT)

#include "DFGDoesGC.h"
#include "DFGGraph.h"
#include "DFGInsertionSet.h"
#include "DFGMayExit.h"
#include "DFGPhase.h"
#include "JSCInlines.h"
#include <wtf/FastBitVector.h>

namespace JSC { namespace DFG {

namespace {

class StoreBarrierClusteringPhase : public Phase {
public:
    StoreBarrierClusteringPhase(Graph& graph)
        : Phase(graph, "store barrier fencing")
        , m_insertionSet(graph)
    {
    }
    
    bool run()
    {
        size_t maxSize = 0;
        for (BasicBlock* block : m_graph.blocksInNaturalOrder())
            maxSize = std::max(maxSize, block->size());
        m_barrierPoints.resize(maxSize);
        
        for (BasicBlock* block : m_graph.blocksInNaturalOrder()) {
            size_t blockSize = block->size();
            doBlock(block);
            m_barrierPoints.clearRange(0, blockSize);
        }
        
        return true;
    }

private:
    // This summarizes everything we need to remember about a barrier.
    struct ChildAndOrigin {
        ChildAndOrigin() { }
        
        ChildAndOrigin(Node* child, CodeOrigin semanticOrigin)
            : child(child)
            , semanticOrigin(semanticOrigin)
        {
        }
        
        Node* child { nullptr };
        CodeOrigin semanticOrigin;
    };
    
    void doBlock(BasicBlock* block)
    {
        ASSERT(m_barrierPoints.isEmpty());
        
        // First identify the places where we would want to place all of the barriers based on a
        // backwards analysis. We use the futureGC flag to tell us if we had seen a GC. Since this
        // is a backwards analysis, when we get to a node, futureGC tells us if a GC will happen
        // in the future after that node.
        bool futureGC = true;
        for (unsigned nodeIndex = block->size(); nodeIndex--;) {
            Node* node = block->at(nodeIndex);
            
            // This is a backwards analysis, so exits require conservatism. If we exit, then there
            // probably will be a GC in the future! If we needed to then we could lift that
            // requirement by either (1) having a StoreBarrierHint that tells OSR exit to barrier that
            // value or (2) automatically barriering any DFG-live Node on OSR exit. Either way, it
            // would be weird because it would create a new root for OSR availability analysis. I
            // don't have evidence that it would be worth it.
            if (doesGC(m_graph, node) || mayExit(m_graph, node) != DoesNotExit) {
                futureGC = true;
                continue;
            }
            
            if (node->isStoreBarrier() && futureGC) {
                m_barrierPoints[nodeIndex] = true;
                futureGC = false;
            }
        }
        
        // Now we run forward and collect the barriers. When we hit a barrier point, insert all of
        // them with a fence.
        for (unsigned nodeIndex = 0; nodeIndex < block->size(); ++nodeIndex) {
            Node* node = block->at(nodeIndex);
            if (!node->isStoreBarrier())
                continue;
            
            DFG_ASSERT(m_graph, node, !node->origin.wasHoisted);
            DFG_ASSERT(m_graph, node, node->child1().useKind() == KnownCellUse);
            
            NodeOrigin origin = node->origin;
            m_neededBarriers.append(ChildAndOrigin(node->child1().node(), origin.semantic));
            node->remove();
            
            if (!m_barrierPoints[nodeIndex])
                continue;
            
            std::sort(
                m_neededBarriers.begin(), m_neededBarriers.end(),
                [&] (const ChildAndOrigin& a, const ChildAndOrigin& b) -> bool {
                    return a.child < b.child;
                });
            auto end = std::unique(
                m_neededBarriers.begin(), m_neededBarriers.end(),
                [&] (const ChildAndOrigin& a, const ChildAndOrigin& b) -> bool{
                    return a.child == b.child;
                });
            for (auto iter = m_neededBarriers.begin(); iter != end; ++iter) {
                Node* child = iter->child;
                CodeOrigin semanticOrigin = iter->semanticOrigin;
                
                NodeType type;
                if (Options::useConcurrentBarriers() && iter == m_neededBarriers.begin())
                    type = FencedStoreBarrier;
                else
                    type = StoreBarrier;
                
                m_insertionSet.insertNode(
                    nodeIndex, SpecNone, type, origin.withSemantic(semanticOrigin),
                    Edge(child, KnownCellUse));
            }
            m_neededBarriers.resize(0);
        }
        
        m_insertionSet.execute(block);
    }
    
    InsertionSet m_insertionSet;
    FastBitVector m_barrierPoints;
    Vector<ChildAndOrigin> m_neededBarriers;
};

} // anonymous namespace

bool performStoreBarrierClustering(Graph& graph)
{
    return runPhase<StoreBarrierClusteringPhase>(graph);
}

} } // namespace JSC::DFG

#endif // ENABLE(DFG_JIT)

