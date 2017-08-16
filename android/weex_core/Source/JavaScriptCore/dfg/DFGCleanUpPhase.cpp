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
#include "DFGCleanUpPhase.h"

#if ENABLE(DFG_JIT)

#include "DFGGraph.h"
#include "DFGInsertionSet.h"
#include "DFGPhase.h"
#include "DFGPredictionPropagationPhase.h"
#include "DFGVariableAccessDataDump.h"
#include "JSCInlines.h"

namespace JSC { namespace DFG {

class CleanUpPhase : public Phase {
public:
    CleanUpPhase(Graph& graph)
        : Phase(graph, "clean up")
    {
    }
    
    bool run()
    {
        bool changed = false;
        
        for (BasicBlock* block : m_graph.blocksInNaturalOrder()) {
            unsigned sourceIndex = 0;
            unsigned targetIndex = 0;
            while (sourceIndex < block->size()) {
                Node* node = block->at(sourceIndex++);
                bool kill = false;
                
                if (node->op() == Check)
                    node->children = node->children.justChecks();
                
                switch (node->op()) {
                case Phantom:
                case Check:
                    if (node->children.isEmpty())
                        kill = true;
                    break;
                default:
                    break;
                }
                
                if (kill)
                    m_graph.deleteNode(node);
                else
                    block->at(targetIndex++) = node;
            }
            block->resize(targetIndex);
        }
        
        return changed;
    }
};
    
bool performCleanUp(Graph& graph)
{
    return runPhase<CleanUpPhase>(graph);
}

} } // namespace JSC::DFG

#endif // ENABLE(DFG_JIT)

