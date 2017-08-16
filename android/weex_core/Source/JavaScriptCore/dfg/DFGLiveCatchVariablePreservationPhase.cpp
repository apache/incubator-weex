/*
 * Copyright (C) 2015-2016 Apple Inc. All rights reserved.
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
#include "DFGLiveCatchVariablePreservationPhase.h"

#if ENABLE(DFG_JIT)

#include "DFGBasicBlockInlines.h"
#include "DFGGraph.h"
#include "DFGInsertionSet.h"
#include "DFGPhase.h"
#include "FullBytecodeLiveness.h"
#include "JSCInlines.h"

namespace JSC { namespace DFG {

class LiveCatchVariablePreservationPhase : public Phase {
public:
    LiveCatchVariablePreservationPhase(Graph& graph)
        : Phase(graph, "live catch variable preservation phase")
    {
    }

    bool run()
    {
        if (!m_graph.m_hasExceptionHandlers)
            return true;

        DFG_ASSERT(m_graph, nullptr, m_graph.m_form == LoadStore);

        m_currentBlockLiveness.resize(m_graph.block(0)->variablesAtTail.numberOfLocals());

        InsertionSet insertionSet(m_graph);
        for (BasicBlock* block : m_graph.blocksInNaturalOrder()) {
            handleBlock(block, insertionSet);
            insertionSet.execute(block);
        }

        return true;
    }

    bool willCatchException(CodeOrigin origin)
    {
        unsigned bytecodeIndexToCheck = origin.bytecodeIndex;
        m_currentBlockLiveness.clearAll();

        while (1) {
            InlineCallFrame* inlineCallFrame = origin.inlineCallFrame;
            CodeBlock* codeBlock = m_graph.baselineCodeBlockFor(inlineCallFrame);
            if (HandlerInfo* handler = codeBlock->handlerForBytecodeOffset(bytecodeIndexToCheck)) {
                unsigned catchBytecodeIndex = handler->target;
                m_graph.forAllLocalsLiveInBytecode(CodeOrigin(catchBytecodeIndex, inlineCallFrame), [&] (VirtualRegister operand) {
                    m_currentBlockLiveness[operand.toLocal()] = true;
                });
                return true;
            }

            if (!inlineCallFrame)
                return false;

            bytecodeIndexToCheck = inlineCallFrame->directCaller.bytecodeIndex;
            origin = inlineCallFrame->directCaller;
        }
    }

    void handleBlock(BasicBlock* block, InsertionSet& insertionSet)
    {
        // Because precise jump targets ensures that the start of a "try" block is its
        // own basic block, we will never have two "try" statements in the same DFG
        // basic block. Therefore, checking the first node in the block is sufficient 
        // to checking if we're in a try block.
        if (!willCatchException(block->at(0)->origin.semantic))
            return;

        Operands<VariableAccessData*> currentBlockAccessData(block->variablesAtTail.numberOfArguments(), block->variablesAtTail.numberOfLocals(), nullptr);
        HashSet<InlineCallFrame*> seenInlineCallFrames;

        {
            for (unsigned i = 0; i < block->size(); i++) {
                Node* node = block->at(i);
                bool isPrimordialSetArgument = node->op() == SetArgument && node->local().isArgument() && node == m_graph.m_arguments[node->local().toArgument()];
                InlineCallFrame* inlineCallFrame = node->origin.semantic.inlineCallFrame;
                if (inlineCallFrame)
                    seenInlineCallFrames.add(inlineCallFrame);

                if (node->op() == SetLocal || (node->op() == SetArgument && !isPrimordialSetArgument)) {
                    VirtualRegister operand = node->local();

                    int stackOffset = inlineCallFrame ? inlineCallFrame->stackOffset : 0;
                    if ((operand.isLocal() && m_currentBlockLiveness[operand.toLocal()])
                        || (operand.offset() == stackOffset + CallFrame::thisArgumentOffset())) {

                        VariableAccessData* variableAccessData = currentBlockAccessData.operand(operand);
                        if (!variableAccessData)
                            variableAccessData = newVariableAccessData(operand);

                        insertionSet.insertNode(i, SpecNone, 
                            Flush, node->origin, OpInfo(variableAccessData));
                    }
                }

                if (node->accessesStack(m_graph))
                    currentBlockAccessData.operand(node->local()) = node->variableAccessData();
            }
        }

        // Insert Flush for everything at the end of the block.
        {
            NodeOrigin origin = block->at(block->size() - 1)->origin;
            auto preserveLivenessAtEndOfBlock = [&] (VirtualRegister operand, bool alwaysInsert) {
                if ((operand.isLocal() && m_currentBlockLiveness[operand.toLocal()]) 
                    || operand.isArgument()
                    || alwaysInsert) {
                    VariableAccessData* accessData = currentBlockAccessData.operand(operand);
                    if (!accessData)
                        accessData = newVariableAccessData(operand);

                    currentBlockAccessData.operand(operand) = accessData;

                    insertionSet.insertNode(block->size(), SpecNone, 
                        Flush, origin, OpInfo(accessData));
                }
            };
            for (unsigned local = 0; local < block->variablesAtTail.numberOfLocals(); local++)
                preserveLivenessAtEndOfBlock(virtualRegisterForLocal(local), false);
            for (InlineCallFrame* inlineCallFrame : seenInlineCallFrames)
                preserveLivenessAtEndOfBlock(VirtualRegister(inlineCallFrame->stackOffset + CallFrame::thisArgumentOffset()), true);
            preserveLivenessAtEndOfBlock(VirtualRegister(CallFrame::thisArgumentOffset()), true);
        }
    }

    VariableAccessData* newVariableAccessData(VirtualRegister operand)
    {
        ASSERT(!operand.isConstant());
        
        m_graph.m_variableAccessData.append(VariableAccessData(operand));
        return &m_graph.m_variableAccessData.last();
    }

    FastBitVector m_currentBlockLiveness;
};

bool performLiveCatchVariablePreservationPhase(Graph& graph)
{
    return runPhase<LiveCatchVariablePreservationPhase>(graph);
}

} } // namespace JSC::DFG

#endif // ENABLE(DFG_JIT)
