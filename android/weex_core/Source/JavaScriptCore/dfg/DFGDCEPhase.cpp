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
#include "DFGDCEPhase.h"

#if ENABLE(DFG_JIT)

#include "DFGBasicBlockInlines.h"
#include "DFGGraph.h"
#include "DFGInsertionSet.h"
#include "DFGPhase.h"
#include "JSCInlines.h"

namespace JSC { namespace DFG {

class DCEPhase : public Phase {
public:
    DCEPhase(Graph& graph)
        : Phase(graph, "dead code elimination")
        , m_insertionSet(graph)
    {
    }
    
    bool run()
    {
        ASSERT(m_graph.m_form == ThreadedCPS || m_graph.m_form == SSA);
        
        m_graph.computeRefCounts();
        
        for (BasicBlock* block : m_graph.blocksInPreOrder())
            fixupBlock(block);
        
        cleanVariables(m_graph.m_arguments);

        // Just do a basic Phantom/Check clean-up.
        for (BlockIndex blockIndex = m_graph.numBlocks(); blockIndex--;) {
            BasicBlock* block = m_graph.block(blockIndex);
            if (!block)
                continue;
            unsigned sourceIndex = 0;
            unsigned targetIndex = 0;
            while (sourceIndex < block->size()) {
                Node* node = block->at(sourceIndex++);
                switch (node->op()) {
                case Check:
                case Phantom:
                    if (node->children.isEmpty())
                        continue;
                    break;
                default:
                    break;
                }
                block->at(targetIndex++) = node;
            }
            block->resize(targetIndex);
        }
        
        m_graph.m_refCountState = ExactRefCount;
        
        return true;
    }

private:
    void fixupBlock(BasicBlock* block)
    {
        if (!block)
            return;

        if (m_graph.m_form == ThreadedCPS) {
            for (unsigned phiIndex = 0; phiIndex < block->phis.size(); ++phiIndex) {
                Node* phi = block->phis[phiIndex];
                if (!phi->shouldGenerate()) {
                    m_graph.deleteNode(phi);
                    block->phis[phiIndex--] = block->phis.last();
                    block->phis.removeLast();
                }
            }
            
            cleanVariables(block->variablesAtHead);
            cleanVariables(block->variablesAtTail);
        }

        // This has to be a forward loop because we are using the insertion set.
        for (unsigned indexInBlock = 0; indexInBlock < block->size(); ++indexInBlock) {
            Node* node = block->at(indexInBlock);
            if (node->shouldGenerate())
                continue;
                
            if (node->flags() & NodeHasVarArgs) {
                for (unsigned childIdx = node->firstChild(); childIdx < node->firstChild() + node->numChildren(); childIdx++) {
                    Edge edge = m_graph.m_varArgChildren[childIdx];
                    
                    if (!edge || edge.willNotHaveCheck())
                        continue;
                    
                    m_insertionSet.insertNode(indexInBlock, SpecNone, Check, node->origin, edge);
                }
                
                node->setOpAndDefaultFlags(Check);
                node->children.reset();
                node->setRefCount(1);
                continue;
            }
            
            node->remove();
            node->setRefCount(1);
        }

        m_insertionSet.execute(block);
    }
    
    template<typename VariablesVectorType>
    void cleanVariables(VariablesVectorType& variables)
    {
        for (unsigned i = variables.size(); i--;) {
            Node* node = variables[i];
            if (!node)
                continue;
            if (node->op() != Check && node->shouldGenerate())
                continue;
            variables[i] = nullptr;
        }
    }
    
    InsertionSet m_insertionSet;
};

bool performDCE(Graph& graph)
{
    return runPhase<DCEPhase>(graph);
}

} } // namespace JSC::DFG

#endif // ENABLE(DFG_JIT)

