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
#include "DFGUnificationPhase.h"

#if ENABLE(DFG_JIT)

#include "DFGBasicBlockInlines.h"
#include "DFGGraph.h"
#include "DFGPhase.h"
#include "JSCInlines.h"

namespace JSC { namespace DFG {

class UnificationPhase : public Phase {
public:
    UnificationPhase(Graph& graph)
        : Phase(graph, "unification")
    {
    }
    
    bool run()
    {
        ASSERT(m_graph.m_form == ThreadedCPS);
        ASSERT(m_graph.m_unificationState == LocallyUnified);
        
        // Ensure that all Phi functions are unified.
        for (BlockIndex blockIndex = m_graph.numBlocks(); blockIndex--;) {
            BasicBlock* block = m_graph.block(blockIndex);
            if (!block)
                continue;
            ASSERT(block->isReachable);
            
            for (unsigned phiIndex = block->phis.size(); phiIndex--;) {
                Node* phi = block->phis[phiIndex];
                for (unsigned childIdx = 0; childIdx < AdjacencyList::Size; ++childIdx) {
                    if (!phi->children.child(childIdx))
                        break;

                    // FIXME: Consider reversing the order of this unification, since the other
                    // order will reveal more bugs. https://bugs.webkit.org/show_bug.cgi?id=154368
                    phi->variableAccessData()->unify(phi->children.child(childIdx)->variableAccessData());
                }
            }
        }
        
        // Ensure that all predictions are fixed up based on the unification.
        for (unsigned i = 0; i < m_graph.m_variableAccessData.size(); ++i) {
            VariableAccessData* data = &m_graph.m_variableAccessData[i];
            data->find()->predict(data->nonUnifiedPrediction());
            data->find()->mergeStructureCheckHoistingFailed(data->structureCheckHoistingFailed());
            data->find()->mergeCheckArrayHoistingFailed(data->checkArrayHoistingFailed());
            data->find()->mergeShouldNeverUnbox(data->shouldNeverUnbox());
            data->find()->mergeIsLoadedFrom(data->isLoadedFrom());
            data->find()->mergeIsProfitableToUnbox(data->isProfitableToUnbox());
        }
        
        m_graph.m_unificationState = GloballyUnified;
        return true;
    }
};

bool performUnification(Graph& graph)
{
    return runPhase<UnificationPhase>(graph);
}

} } // namespace JSC::DFG

#endif // ENABLE(DFG_JIT)

