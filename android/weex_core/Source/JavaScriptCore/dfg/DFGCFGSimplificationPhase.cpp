/*
 * Copyright (C) 2012-2015 Apple Inc. All rights reserved.
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
#include "DFGCFGSimplificationPhase.h"

#if ENABLE(DFG_JIT)

#include "DFGBasicBlockInlines.h"
#include "DFGGraph.h"
#include "DFGInsertionSet.h"
#include "DFGPhase.h"
#include "DFGValidate.h"
#include "JSCInlines.h"

namespace JSC { namespace DFG {

class CFGSimplificationPhase : public Phase {
public:
    CFGSimplificationPhase(Graph& graph)
        : Phase(graph, "CFG simplification")
    {
    }
    
    bool run()
    {
        // FIXME: We should make this work in SSA. https://bugs.webkit.org/show_bug.cgi?id=148260
        DFG_ASSERT(m_graph, nullptr, m_graph.m_form != SSA);
        
        const bool extremeLogging = false;

        bool outerChanged = false;
        bool innerChanged;
        
        do {
            innerChanged = false;
            for (BlockIndex blockIndex = 0; blockIndex < m_graph.numBlocks(); ++blockIndex) {
                BasicBlock* block = m_graph.block(blockIndex);
                if (!block)
                    continue;
                ASSERT(block->isReachable);
            
                switch (block->terminal()->op()) {
                case Jump: {
                    // Successor with one predecessor -> merge.
                    if (block->successor(0)->predecessors.size() == 1) {
                        ASSERT(block->successor(0)->predecessors[0] == block);
                        if (extremeLogging)
                            m_graph.dump();
                        m_graph.dethread();
                        mergeBlocks(block, block->successor(0), noBlocks());
                        innerChanged = outerChanged = true;
                        break;
                    }
                
                    // FIXME: Block only has a jump -> remove. This is tricky though because of
                    // liveness. What we really want is to slam in a phantom at the end of the
                    // block, after the terminal. But we can't right now. :-(
                    // Idea: what if I slam the ghosties into my successor? Nope, that's
                    // suboptimal, because if my successor has multiple predecessors then we'll
                    // be keeping alive things on other predecessor edges unnecessarily.
                    // What we really need is the notion of end-of-block ghosties!
                    // FIXME: Allow putting phantoms after terminals.
                    // https://bugs.webkit.org/show_bug.cgi?id=126778
                    break;
                }
                
                case Branch: {
                    // Branch on constant -> jettison the not-taken block and merge.
                    if (isKnownDirection(block->cfaBranchDirection)) {
                        bool condition = branchCondition(block->cfaBranchDirection);
                        BasicBlock* targetBlock = block->successorForCondition(condition);
                        BasicBlock* jettisonedBlock = block->successorForCondition(!condition);
                        if (targetBlock->predecessors.size() == 1) {
                            if (extremeLogging)
                                m_graph.dump();
                            m_graph.dethread();
                            mergeBlocks(block, targetBlock, oneBlock(jettisonedBlock));
                        } else {
                            if (extremeLogging)
                                m_graph.dump();
                            m_graph.dethread();
                            
                            Node* terminal = block->terminal();
                            ASSERT(terminal->isTerminal());
                            NodeOrigin boundaryNodeOrigin = terminal->origin;

                            jettisonBlock(block, jettisonedBlock, boundaryNodeOrigin);

                            block->replaceTerminal(
                                m_graph, SpecNone, Jump, boundaryNodeOrigin,
                                OpInfo(targetBlock));
                            
                            ASSERT(block->terminal());
                        
                        }
                        innerChanged = outerChanged = true;
                        break;
                    }
                    
                    if (block->successor(0) == block->successor(1)) {
                        convertToJump(block, block->successor(0));
                        innerChanged = outerChanged = true;
                        break;
                    }
                    
                    // Branch to same destination -> jump.
                    // FIXME: this will currently not be hit because of the lack of jump-only
                    // block simplification.
                    
                    break;
                }
                    
                case Switch: {
                    SwitchData* data = block->terminal()->switchData();
                    
                    // Prune out cases that end up jumping to default.
                    for (unsigned i = 0; i < data->cases.size(); ++i) {
                        if (data->cases[i].target.block == data->fallThrough.block) {
                            data->fallThrough.count += data->cases[i].target.count;
                            data->cases[i--] = data->cases.last();
                            data->cases.removeLast();
                        }
                    }
                    
                    // If there are no cases other than default then this turns
                    // into a jump.
                    if (data->cases.isEmpty()) {
                        convertToJump(block, data->fallThrough.block);
                        innerChanged = outerChanged = true;
                        break;
                    }
                    
                    // Switch on constant -> jettison all other targets and merge.
                    Node* terminal = block->terminal();
                    if (terminal->child1()->hasConstant()) {
                        FrozenValue* value = terminal->child1()->constant();
                        TriState found = FalseTriState;
                        BasicBlock* targetBlock = 0;
                        for (unsigned i = data->cases.size(); found == FalseTriState && i--;) {
                            found = data->cases[i].value.strictEqual(value);
                            if (found == TrueTriState)
                                targetBlock = data->cases[i].target.block;
                        }
                        
                        if (found == MixedTriState)
                            break;
                        if (found == FalseTriState)
                            targetBlock = data->fallThrough.block;
                        ASSERT(targetBlock);
                        
                        Vector<BasicBlock*, 1> jettisonedBlocks;
                        for (BasicBlock* successor : terminal->successors()) {
                            if (successor != targetBlock)
                                jettisonedBlocks.append(successor);
                        }
                        
                        if (targetBlock->predecessors.size() == 1) {
                            if (extremeLogging)
                                m_graph.dump();
                            m_graph.dethread();
                            
                            mergeBlocks(block, targetBlock, jettisonedBlocks);
                        } else {
                            if (extremeLogging)
                                m_graph.dump();
                            m_graph.dethread();
                            
                            NodeOrigin boundaryNodeOrigin = terminal->origin;

                            for (unsigned i = jettisonedBlocks.size(); i--;)
                                jettisonBlock(block, jettisonedBlocks[i], boundaryNodeOrigin);
                            
                            block->replaceTerminal(
                                m_graph, SpecNone, Jump, boundaryNodeOrigin, OpInfo(targetBlock));
                        }
                        innerChanged = outerChanged = true;
                        break;
                    }
                    break;
                }
                    
                default:
                    break;
                }
            }
            
            if (innerChanged) {
                // Here's the reason for this pass:
                // Blocks: A, B, C, D, E, F
                // A -> B, C
                // B -> F
                // C -> D, E
                // D -> F
                // E -> F
                //
                // Assume that A's branch is determined to go to B. Then the rest of this phase
                // is smart enough to simplify down to:
                // A -> B
                // B -> F
                // C -> D, E
                // D -> F
                // E -> F
                //
                // We will also merge A and B. But then we don't have any other mechanism to
                // remove D, E as predecessors for F. Worse, the rest of this phase does not
                // know how to fix the Phi functions of F to ensure that they no longer refer
                // to variables in D, E. In general, we need a way to handle Phi simplification
                // upon:
                // 1) Removal of a predecessor due to branch simplification. The branch
                //    simplifier already does that.
                // 2) Invalidation of a predecessor because said predecessor was rendered
                //    unreachable. We do this here.
                //
                // This implies that when a block is unreachable, we must inspect its
                // successors' Phi functions to remove any references from them into the
                // removed block.
                
                m_graph.invalidateCFG();
                m_graph.resetReachability();
                m_graph.killUnreachableBlocks();
            }
            
            if (Options::validateGraphAtEachPhase())
                validate();
        } while (innerChanged);
        
        return outerChanged;
    }

private:
    void convertToJump(BasicBlock* block, BasicBlock* targetBlock)
    {
        ASSERT(targetBlock);
        ASSERT(targetBlock->isReachable);
        if (targetBlock->predecessors.size() == 1) {
            m_graph.dethread();
            mergeBlocks(block, targetBlock, noBlocks());
        } else {
            Node* branch = block->terminal();
            ASSERT(branch->op() == Branch || branch->op() == Switch);

            block->replaceTerminal(
                m_graph, SpecNone, Jump, branch->origin, OpInfo(targetBlock));
        }
    }
    
    void keepOperandAlive(BasicBlock* block, BasicBlock* jettisonedBlock, NodeOrigin nodeOrigin, VirtualRegister operand)
    {
        Node* livenessNode = jettisonedBlock->variablesAtHead.operand(operand);
        if (!livenessNode)
            return;
        NodeType nodeType;
        if (livenessNode->flags() & NodeIsFlushed)
            nodeType = Flush;
        else {
            // This seems like it shouldn't be necessary because we could just rematerialize
            // PhantomLocals or something similar using bytecode liveness. However, in ThreadedCPS, it's
            // worth the sanity to maintain this eagerly. See
            // https://bugs.webkit.org/show_bug.cgi?id=144086
            nodeType = PhantomLocal;
        }
        block->appendNode(
            m_graph, SpecNone, nodeType, nodeOrigin, 
            OpInfo(livenessNode->variableAccessData()));
    }
    
    void jettisonBlock(BasicBlock* block, BasicBlock* jettisonedBlock, NodeOrigin boundaryNodeOrigin)
    {
        for (size_t i = 0; i < jettisonedBlock->variablesAtHead.numberOfArguments(); ++i)
            keepOperandAlive(block, jettisonedBlock, boundaryNodeOrigin, virtualRegisterForArgument(i));
        for (size_t i = 0; i < jettisonedBlock->variablesAtHead.numberOfLocals(); ++i)
            keepOperandAlive(block, jettisonedBlock, boundaryNodeOrigin, virtualRegisterForLocal(i));
        
        fixJettisonedPredecessors(block, jettisonedBlock);
    }
    
    void fixJettisonedPredecessors(BasicBlock* block, BasicBlock* jettisonedBlock)
    {
        jettisonedBlock->removePredecessor(block);
    }

    Vector<BasicBlock*, 1> noBlocks()
    {
        return Vector<BasicBlock*, 1>();
    }
    
    Vector<BasicBlock*, 1> oneBlock(BasicBlock* block)
    {
        Vector<BasicBlock*, 1> result;
        result.append(block);
        return result;
    }
    
    void mergeBlocks(
        BasicBlock* firstBlock, BasicBlock* secondBlock,
        Vector<BasicBlock*, 1> jettisonedBlocks)
    {
        // This will add all of the nodes in secondBlock to firstBlock, but in so doing
        // it will also ensure that any GetLocals from the second block that refer to
        // SetLocals in the first block are relinked. If jettisonedBlock is not NoBlock,
        // then Phantoms are inserted for anything that the jettisonedBlock would have
        // kept alive.
        
        // Remove the terminal of firstBlock since we don't need it anymore. Well, we don't
        // really remove it; we actually turn it into a check.
        Node* terminal = firstBlock->terminal();
        ASSERT(terminal->isTerminal());
        NodeOrigin boundaryNodeOrigin = terminal->origin;
        terminal->remove();
        ASSERT(terminal->refCount() == 1);
        
        for (unsigned i = jettisonedBlocks.size(); i--;) {
            BasicBlock* jettisonedBlock = jettisonedBlocks[i];
            
            // Time to insert ghosties for things that need to be kept alive in case we OSR
            // exit prior to hitting the firstBlock's terminal, and end up going down a
            // different path than secondBlock.
            
            for (size_t i = 0; i < jettisonedBlock->variablesAtHead.numberOfArguments(); ++i)
                keepOperandAlive(firstBlock, jettisonedBlock, boundaryNodeOrigin, virtualRegisterForArgument(i));
            for (size_t i = 0; i < jettisonedBlock->variablesAtHead.numberOfLocals(); ++i)
                keepOperandAlive(firstBlock, jettisonedBlock, boundaryNodeOrigin, virtualRegisterForLocal(i));
        }
        
        for (size_t i = 0; i < secondBlock->phis.size(); ++i)
            firstBlock->phis.append(secondBlock->phis[i]);

        for (size_t i = 0; i < secondBlock->size(); ++i)
            firstBlock->append(secondBlock->at(i));
        
        ASSERT(firstBlock->terminal()->isTerminal());
        
        // Fix the predecessors of my new successors. This is tricky, since we are going to reset
        // all predecessors anyway due to reachability analysis. But we need to fix the
        // predecessors eagerly to ensure that we know what they are in case the next block we
        // consider in this phase wishes to query the predecessors of one of the blocks we
        // affected.
        for (unsigned i = firstBlock->numSuccessors(); i--;) {
            BasicBlock* successor = firstBlock->successor(i);
            for (unsigned j = 0; j < successor->predecessors.size(); ++j) {
                if (successor->predecessors[j] == secondBlock)
                    successor->predecessors[j] = firstBlock;
            }
        }
        
        // Fix the predecessors of my former successors. Again, we'd rather not do this, but it's
        // an unfortunate necessity. See above comment.
        for (unsigned i = jettisonedBlocks.size(); i--;)
            fixJettisonedPredecessors(firstBlock, jettisonedBlocks[i]);
        
        firstBlock->valuesAtTail = secondBlock->valuesAtTail;
        firstBlock->cfaBranchDirection = secondBlock->cfaBranchDirection;
        
        m_graph.killBlock(secondBlock);
    }
};

bool performCFGSimplification(Graph& graph)
{
    return runPhase<CFGSimplificationPhase>(graph);
}

} } // namespace JSC::DFG

#endif // ENABLE(DFG_JIT)


