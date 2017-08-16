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
#include "DFGPredictionInjectionPhase.h"

#if ENABLE(DFG_JIT)

#include "DFGBasicBlockInlines.h"
#include "DFGGraph.h"
#include "DFGPhase.h"
#include "JSCInlines.h"

namespace JSC { namespace DFG {

class PredictionInjectionPhase : public Phase {
public:
    PredictionInjectionPhase(Graph& graph)
        : Phase(graph, "prediction injection")
    {
    }
    
    bool run()
    {
        ASSERT(m_graph.m_form == ThreadedCPS);
        ASSERT(m_graph.m_unificationState == GloballyUnified);
        
        ASSERT(codeBlock()->numParameters() >= 1);
        {
            ConcurrentJSLocker locker(profiledBlock()->m_lock);
            
            for (size_t arg = 0; arg < static_cast<size_t>(codeBlock()->numParameters()); ++arg) {
                ValueProfile* profile = profiledBlock()->valueProfileForArgument(arg);
                if (!profile)
                    continue;
            
                m_graph.m_arguments[arg]->variableAccessData()->predict(
                    profile->computeUpdatedPrediction(locker));
            }
        }
        
        for (BlockIndex blockIndex = 0; blockIndex < m_graph.numBlocks(); ++blockIndex) {
            BasicBlock* block = m_graph.block(blockIndex);
            if (!block)
                continue;
            if (!block->isOSRTarget)
                continue;
            if (block->bytecodeBegin != m_graph.m_plan.osrEntryBytecodeIndex)
                continue;
            for (size_t i = 0; i < m_graph.m_plan.mustHandleValues.size(); ++i) {
                int operand = m_graph.m_plan.mustHandleValues.operandForIndex(i);
                Node* node = block->variablesAtHead.operand(operand);
                if (!node)
                    continue;
                ASSERT(node->accessesStack(m_graph));
                node->variableAccessData()->predict(
                    speculationFromValue(m_graph.m_plan.mustHandleValues[i]));
            }
        }
        
        return true;
    }
};

bool performPredictionInjection(Graph& graph)
{
    return runPhase<PredictionInjectionPhase>(graph);
}

} } // namespace JSC::DFG

#endif // ENABLE(DFG_JIT)

