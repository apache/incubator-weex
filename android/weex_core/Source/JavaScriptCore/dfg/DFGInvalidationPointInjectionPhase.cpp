/*
 * Copyright (C) 2013, 2014 Apple Inc. All rights reserved.
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
#include "DFGInvalidationPointInjectionPhase.h"

#if ENABLE(DFG_JIT)

#include "DFGBlockSetInlines.h"
#include "DFGClobberize.h"
#include "DFGGraph.h"
#include "DFGInsertionSet.h"
#include "DFGPhase.h"
#include "JSCInlines.h"

namespace JSC { namespace DFG {

class InvalidationPointInjectionPhase : public Phase {
    static const bool verbose = false;
    
public:
    InvalidationPointInjectionPhase(Graph& graph)
        : Phase(graph, "invalidation point injection")
        , m_insertionSet(graph)
    {
    }
    
    bool run()
    {
        ASSERT(m_graph.m_form != SSA);
        
        BlockSet blocksThatNeedInvalidationPoints;
        
        for (BlockIndex blockIndex = m_graph.numBlocks(); blockIndex--;) {
            BasicBlock* block = m_graph.block(blockIndex);
            if (!block)
                continue;
            
            m_originThatHadFire = CodeOrigin();
            
            for (unsigned nodeIndex = 0; nodeIndex < block->size(); ++nodeIndex)
                handle(nodeIndex, block->at(nodeIndex));
            
            // Note: this assumes that control flow occurs at bytecode instruction boundaries.
            if (m_originThatHadFire.isSet()) {
                for (unsigned i = block->numSuccessors(); i--;)
                    blocksThatNeedInvalidationPoints.add(block->successor(i));
            }
            
            m_insertionSet.execute(block);
        }

        for (BasicBlock* block : blocksThatNeedInvalidationPoints.iterable(m_graph)) {
            insertInvalidationCheck(0, block->at(0));
            m_insertionSet.execute(block);
        }
        
        return true;
    }

private:
    void handle(unsigned nodeIndex, Node* node)
    {
        if (m_originThatHadFire.isSet() && m_originThatHadFire != node->origin.forExit) {
            insertInvalidationCheck(nodeIndex, node);
            m_originThatHadFire = CodeOrigin();
        }
        
        if (writesOverlap(m_graph, node, Watchpoint_fire))
            m_originThatHadFire = node->origin.forExit;
    }
    
    void insertInvalidationCheck(unsigned nodeIndex, Node* node)
    {
        m_insertionSet.insertNode(nodeIndex, SpecNone, InvalidationPoint, node->origin);
    }
    
    CodeOrigin m_originThatHadFire;
    InsertionSet m_insertionSet;
};

bool performInvalidationPointInjection(Graph& graph)
{
    return runPhase<InvalidationPointInjectionPhase>(graph);
}

} } // namespace JSC::DFG

#endif // ENABLE(DFG_JIT)

