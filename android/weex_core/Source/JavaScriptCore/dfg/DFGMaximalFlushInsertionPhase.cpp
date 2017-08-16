/*
 * Copyright (C) 2015 Apple Inc. All rights reserved.
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
#include "DFGMaximalFlushInsertionPhase.h"

#if ENABLE(DFG_JIT)

#include "DFGBasicBlockInlines.h"
#include "DFGGraph.h"
#include "DFGInsertionSet.h"
#include "DFGPhase.h"
#include "JSCInlines.h"

namespace JSC { namespace DFG {

class MaximalFlushInsertionPhase : public Phase {
public:
    MaximalFlushInsertionPhase(Graph& graph)
        : Phase(graph, "maximal flush insertion phase")
    {
    }
    
    bool run()
    {
        DFG_ASSERT(m_graph, nullptr, m_graph.m_form == LoadStore);

        InsertionSet insertionSet(m_graph);
        for (BasicBlock* block : m_graph.blocksInNaturalOrder()) {
            treatRegularBlock(block, insertionSet);
            insertionSet.execute(block);
        }

        treatRootBlock(m_graph.block(0), insertionSet);
        insertionSet.execute(m_graph.block(0));

        return true;
    }

    void treatRegularBlock(BasicBlock* block, InsertionSet& insertionSet)
    {
        Operands<VariableAccessData*> currentBlockAccessData(block->variablesAtTail.numberOfArguments(), block->variablesAtTail.numberOfLocals(), nullptr);
        // Insert a Flush before every SetLocal to properly pattern the graph such that 
        // any range between SetLocal and Flush has access to the local on the stack.
        {
            for (unsigned i = 0; i < block->size(); i++) {
                Node* node = block->at(i);
                bool isPrimordialSetArgument = node->op() == SetArgument && node->local().isArgument() && node == m_graph.m_arguments[node->local().toArgument()];
                if (node->op() == SetLocal || (node->op() == SetArgument && !isPrimordialSetArgument)) {
                    VirtualRegister operand = node->local();
                    VariableAccessData* flushAccessData = currentBlockAccessData.operand(operand);
                    if (!flushAccessData)
                        flushAccessData = newVariableAccessData(operand);

                    insertionSet.insertNode(i, SpecNone, 
                        Flush, node->origin, OpInfo(flushAccessData));
                }

                if (node->accessesStack(m_graph))
                    currentBlockAccessData.operand(node->local()) = node->variableAccessData();
            }
        }

        // Flush everything at the end of the block.
        {
            NodeOrigin origin = block->at(block->size() - 1)->origin;
            auto insertFlushAtEnd = [&] (VirtualRegister operand) {
                VariableAccessData* accessData = currentBlockAccessData.operand(operand);
                if (!accessData)
                    accessData = newVariableAccessData(operand);

                currentBlockAccessData.operand(operand) = accessData;

                insertionSet.insertNode(block->size(), SpecNone, 
                    Flush, origin, OpInfo(accessData));
            };

            for (unsigned i = 0; i < block->variablesAtTail.numberOfLocals(); i++)
                insertFlushAtEnd(virtualRegisterForLocal(i));
            for (unsigned i = 0; i < block->variablesAtTail.numberOfArguments(); i++)
                insertFlushAtEnd(virtualRegisterForArgument(i));
        }
    }

    void treatRootBlock(BasicBlock* block, InsertionSet& insertionSet)
    {
        Operands<VariableAccessData*> initialAccessData(block->variablesAtTail.numberOfArguments(), block->variablesAtTail.numberOfLocals(), nullptr);
        Operands<Node*> initialAccessNodes(block->variablesAtTail.numberOfArguments(), block->variablesAtTail.numberOfLocals(), nullptr);
        for (auto* node : *block) {
            if (!node->accessesStack(m_graph))
                continue;

            VirtualRegister operand = node->local();
            if (initialAccessData.operand(operand))
                continue;

            DFG_ASSERT(m_graph, node, node->op() != SetLocal); // We should have inserted a Flush before this!
            initialAccessData.operand(operand) = node->variableAccessData();
            initialAccessNodes.operand(operand) = node;
        }

        // We want every Flush to be able to reach backwards to
        // a SetLocal. Doing this in the root block achieves this goal.
        NodeOrigin origin = block->at(0)->origin;
        Node* undefined = insertionSet.insertConstant(0, origin, jsUndefined());

        for (unsigned i = 0; i < block->variablesAtTail.numberOfLocals(); i++) {
            VirtualRegister operand = virtualRegisterForLocal(i);
            VariableAccessData* accessData;
            DFG_ASSERT(m_graph, nullptr, initialAccessNodes.operand(operand)->op() == Flush); // We should have inserted a Flush before any SetLocal/SetArgument for the local that we are analyzing now.
            accessData = initialAccessData.operand(operand);
            DFG_ASSERT(m_graph, nullptr, accessData);
            insertionSet.insertNode(0, SpecNone, 
                SetLocal, origin, OpInfo(accessData), Edge(undefined));
        }
    }


    VariableAccessData* newVariableAccessData(VirtualRegister operand)
    {
        ASSERT(!operand.isConstant());
        
        m_graph.m_variableAccessData.append(VariableAccessData(operand));
        return &m_graph.m_variableAccessData.last();
    }
};

bool performMaximalFlushInsertion(Graph& graph)
{
    return runPhase<MaximalFlushInsertionPhase>(graph);
}

} } // namespace JSC::DFG

#endif // ENABLE(DFG_JIT)
