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
#include "DFGSSAConversionPhase.h"

#if ENABLE(DFG_JIT)

#include "DFGBasicBlockInlines.h"
#include "DFGGraph.h"
#include "DFGInsertionSet.h"
#include "DFGPhase.h"
#include "DFGSSACalculator.h"
#include "DFGVariableAccessDataDump.h"
#include "JSCInlines.h"

namespace JSC { namespace DFG {

class SSAConversionPhase : public Phase {
    static const bool verbose = false;
    
public:
    SSAConversionPhase(Graph& graph)
        : Phase(graph, "SSA conversion")
        , m_calculator(graph)
        , m_insertionSet(graph)
    {
    }
    
    bool run()
    {
        RELEASE_ASSERT(m_graph.m_form == ThreadedCPS);
        
        m_graph.clearReplacements();
        m_graph.ensureDominators();
        
        if (verbose) {
            dataLog("Graph before SSA transformation:\n");
            m_graph.dump();
        }

        // Create a SSACalculator::Variable for every root VariableAccessData.
        for (VariableAccessData& variable : m_graph.m_variableAccessData) {
            if (!variable.isRoot())
                continue;
            
            SSACalculator::Variable* ssaVariable = m_calculator.newVariable();
            ASSERT(ssaVariable->index() == m_variableForSSAIndex.size());
            m_variableForSSAIndex.append(&variable);
            m_ssaVariableForVariable.add(&variable, ssaVariable);
        }
        
        // Find all SetLocals and create Defs for them. We handle SetArgument by creating a
        // GetLocal, and recording the flush format.
        for (BlockIndex blockIndex = m_graph.numBlocks(); blockIndex--;) {
            BasicBlock* block = m_graph.block(blockIndex);
            if (!block)
                continue;
            
            // Must process the block in forward direction because we want to see the last
            // assignment for every local.
            for (unsigned nodeIndex = 0; nodeIndex < block->size(); ++nodeIndex) {
                Node* node = block->at(nodeIndex);
                if (node->op() != SetLocal && node->op() != SetArgument)
                    continue;
                
                VariableAccessData* variable = node->variableAccessData();
                
                Node* childNode;
                if (node->op() == SetLocal)
                    childNode = node->child1().node();
                else {
                    ASSERT(node->op() == SetArgument);
                    childNode = m_insertionSet.insertNode(
                        nodeIndex, node->variableAccessData()->prediction(),
                        GetStack, node->origin,
                        OpInfo(m_graph.m_stackAccessData.add(variable->local(), variable->flushFormat())));
                    if (!ASSERT_DISABLED)
                        m_argumentGetters.add(childNode);
                    m_argumentMapping.add(node, childNode);
                }
                
                m_calculator.newDef(
                    m_ssaVariableForVariable.get(variable), block, childNode);
            }
            
            m_insertionSet.execute(block);
        }
        
        // Decide where Phis are to be inserted. This creates the Phi's but doesn't insert them
        // yet. We will later know where to insert them because SSACalculator is such a bro.
        m_calculator.computePhis(
            [&] (SSACalculator::Variable* ssaVariable, BasicBlock* block) -> Node* {
                VariableAccessData* variable = m_variableForSSAIndex[ssaVariable->index()];
                
                // Prune by liveness. This doesn't buy us much other than compile times.
                Node* headNode = block->variablesAtHead.operand(variable->local());
                if (!headNode)
                    return nullptr;

                // There is the possibiltiy of "rebirths". The SSA calculator will already prune
                // rebirths for the same VariableAccessData. But it will not be able to prune
                // rebirths that arose from the same local variable number but a different
                // VariableAccessData. We do that pruning here.
                //
                // Here's an example of a rebirth that this would catch:
                //
                //     var x;
                //     if (foo) {
                //         if (bar) {
                //             x = 42;
                //         } else {
                //             x = 43;
                //         }
                //         print(x);
                //         x = 44;
                //     } else {
                //         x = 45;
                //     }
                //     print(x); // Without this check, we'd have a Phi for x = 42|43 here.
                //
                // FIXME: Consider feeding local variable numbers, not VariableAccessData*'s, as
                // the "variables" for SSACalculator. That would allow us to eliminate this
                // special case.
                // https://bugs.webkit.org/show_bug.cgi?id=136641
                if (headNode->variableAccessData() != variable)
                    return nullptr;
                
                Node* phiNode = m_graph.addNode(
                    variable->prediction(), Phi, block->at(0)->origin.withInvalidExit());
                FlushFormat format = variable->flushFormat();
                NodeFlags result = resultFor(format);
                phiNode->mergeFlags(result);
                return phiNode;
            });
        
        if (verbose) {
            dataLog("Computed Phis, about to transform the graph.\n");
            dataLog("\n");
            dataLog("Graph:\n");
            m_graph.dump();
            dataLog("\n");
            dataLog("Mappings:\n");
            for (unsigned i = 0; i < m_variableForSSAIndex.size(); ++i)
                dataLog("    ", i, ": ", VariableAccessDataDump(m_graph, m_variableForSSAIndex[i]), "\n");
            dataLog("\n");
            dataLog("SSA calculator: ", m_calculator, "\n");
        }
        
        // Do the bulk of the SSA conversion. For each block, this tracks the operand->Node
        // mapping based on a combination of what the SSACalculator tells us, and us walking over
        // the block in forward order. We use our own data structure, valueForOperand, for
        // determining the local mapping, but we rely on SSACalculator for the non-local mapping.
        //
        // This does three things at once:
        //
        // - Inserts the Phis in all of the places where they need to go. We've already created
        //   them and they are accounted for in the SSACalculator's data structures, but we
        //   haven't inserted them yet, mostly because we want to insert all of a block's Phis in
        //   one go to amortize the cost of node insertion.
        //
        // - Create and insert Upsilons.
        //
        // - Convert all of the preexisting SSA nodes (other than the old CPS Phi nodes) into SSA
        //   form by replacing as follows:
        //
        //   - MovHint has KillLocal prepended to it.
        //
        //   - GetLocal die and get replaced with references to the node specified by
        //     valueForOperand.
        //
        //   - SetLocal turns into PutStack if it's flushed, or turns into a Check otherwise.
        //
        //   - Flush loses its children and turns into a Phantom.
        //
        //   - PhantomLocal becomes Phantom, and its child is whatever is specified by
        //     valueForOperand.
        //
        //   - SetArgument is removed. Note that GetStack nodes have already been inserted.
        Operands<Node*> valueForOperand(OperandsLike, m_graph.block(0)->variablesAtHead);
        for (BasicBlock* block : m_graph.blocksInPreOrder()) {
            valueForOperand.clear();
            
            // CPS will claim that the root block has all arguments live. But we have already done
            // the first step of SSA conversion: argument locals are no longer live at head;
            // instead we have GetStack nodes for extracting the values of arguments. So, we
            // skip the at-head available value calculation for the root block.
            if (block != m_graph.block(0)) {
                for (size_t i = valueForOperand.size(); i--;) {
                    Node* nodeAtHead = block->variablesAtHead[i];
                    if (!nodeAtHead)
                        continue;
                    
                    VariableAccessData* variable = nodeAtHead->variableAccessData();
                    
                    if (verbose)
                        dataLog("Considering live variable ", VariableAccessDataDump(m_graph, variable), " at head of block ", *block, "\n");
                    
                    SSACalculator::Variable* ssaVariable = m_ssaVariableForVariable.get(variable);
                    SSACalculator::Def* def = m_calculator.reachingDefAtHead(block, ssaVariable);
                    if (!def) {
                        // If we are required to insert a Phi, then we won't have a reaching def
                        // at head.
                        continue;
                    }
                    
                    Node* node = def->value();
                    if (node->replacement()) {
                        // This will occur when a SetLocal had a GetLocal as its source. The
                        // GetLocal would get replaced with an actual SSA value by the time we get
                        // here. Note that the SSA value with which the GetLocal got replaced
                        // would not in turn have a replacement.
                        node = node->replacement();
                        ASSERT(!node->replacement());
                    }
                    if (verbose)
                        dataLog("Mapping: ", VirtualRegister(valueForOperand.operandForIndex(i)), " -> ", node, "\n");
                    valueForOperand[i] = node;
                }
            }
            
            // Insert Phis by asking the calculator what phis there are in this block. Also update
            // valueForOperand with those Phis. For Phis associated with variables that are not
            // flushed, we also insert a MovHint.
            size_t phiInsertionPoint = 0;
            for (SSACalculator::Def* phiDef : m_calculator.phisForBlock(block)) {
                VariableAccessData* variable = m_variableForSSAIndex[phiDef->variable()->index()];
                
                m_insertionSet.insert(phiInsertionPoint, phiDef->value());
                valueForOperand.operand(variable->local()) = phiDef->value();
                
                m_insertionSet.insertNode(
                    phiInsertionPoint, SpecNone, MovHint, block->at(0)->origin.withInvalidExit(),
                    OpInfo(variable->local().offset()), phiDef->value()->defaultEdge());
            }

            if (block->at(0)->origin.exitOK)
                m_insertionSet.insertNode(phiInsertionPoint, SpecNone, ExitOK, block->at(0)->origin);
            
            for (unsigned nodeIndex = 0; nodeIndex < block->size(); ++nodeIndex) {
                Node* node = block->at(nodeIndex);
                
                if (verbose) {
                    dataLog("Processing node ", node, ":\n");
                    m_graph.dump(WTF::dataFile(), "    ", node);
                }
                
                m_graph.performSubstitution(node);
                
                switch (node->op()) {
                case MovHint: {
                    m_insertionSet.insertNode(
                        nodeIndex, SpecNone, KillStack, node->origin,
                        OpInfo(node->unlinkedLocal().offset()));
                    node->origin.exitOK = false; // KillStack clobbers exit.
                    break;
                }
                    
                case SetLocal: {
                    VariableAccessData* variable = node->variableAccessData();
                    Node* child = node->child1().node();
                    
                    if (!!(node->flags() & NodeIsFlushed)) {
                        node->convertToPutStack(
                            m_graph.m_stackAccessData.add(
                                variable->local(), variable->flushFormat()));
                    } else
                        node->remove();
                    
                    if (verbose)
                        dataLog("Mapping: ", variable->local(), " -> ", child, "\n");
                    valueForOperand.operand(variable->local()) = child;
                    break;
                }
                    
                case GetStack: {
                    ASSERT(m_argumentGetters.contains(node));
                    valueForOperand.operand(node->stackAccessData()->local) = node;
                    break;
                }
                    
                case GetLocal: {
                    VariableAccessData* variable = node->variableAccessData();
                    node->children.reset();
                    
                    node->remove();
                    if (verbose)
                        dataLog("Replacing node ", node, " with ", valueForOperand.operand(variable->local()), "\n");
                    node->setReplacement(valueForOperand.operand(variable->local()));
                    break;
                }
                    
                case Flush: {
                    node->children.reset();
                    node->remove();
                    break;
                }
                    
                case PhantomLocal: {
                    ASSERT(node->child1().useKind() == UntypedUse);
                    VariableAccessData* variable = node->variableAccessData();
                    node->child1() = valueForOperand.operand(variable->local())->defaultEdge();
                    node->remove();
                    break;
                }
                    
                case SetArgument: {
                    node->remove();
                    break;
                }
                    
                default:
                    break;
                }
            }
            
            // We want to insert Upsilons just before the end of the block. On the surface this
            // seems dangerous because the Upsilon will have a checking UseKind. But, we will not
            // actually be performing the check at the point of the Upsilon; the check will
            // already have been performed at the point where the original SetLocal was.
            NodeAndIndex terminal = block->findTerminal();
            size_t upsilonInsertionPoint = terminal.index;
            NodeOrigin upsilonOrigin = terminal.node->origin;
            for (unsigned successorIndex = block->numSuccessors(); successorIndex--;) {
                BasicBlock* successorBlock = block->successor(successorIndex);
                for (SSACalculator::Def* phiDef : m_calculator.phisForBlock(successorBlock)) {
                    Node* phiNode = phiDef->value();
                    SSACalculator::Variable* ssaVariable = phiDef->variable();
                    VariableAccessData* variable = m_variableForSSAIndex[ssaVariable->index()];
                    FlushFormat format = variable->flushFormat();

                    // We can use an unchecked use kind because the SetLocal was turned into a Check.
                    // We have to use an unchecked use because at least sometimes, the end of the block
                    // is not exitOK.
                    UseKind useKind = uncheckedUseKindFor(format);
                    
                    m_insertionSet.insertNode(
                        upsilonInsertionPoint, SpecNone, Upsilon, upsilonOrigin,
                        OpInfo(phiNode), Edge(
                            valueForOperand.operand(variable->local()),
                            useKind));
                }
            }
            
            m_insertionSet.execute(block);
        }
        
        // Free all CPS phis and reset variables vectors.
        for (BlockIndex blockIndex = m_graph.numBlocks(); blockIndex--;) {
            BasicBlock* block = m_graph.block(blockIndex);
            if (!block)
                continue;
            for (unsigned phiIndex = block->phis.size(); phiIndex--;)
                m_graph.deleteNode(block->phis[phiIndex]);
            block->phis.clear();
            block->variablesAtHead.clear();
            block->variablesAtTail.clear();
            block->valuesAtHead.clear();
            block->valuesAtHead.clear();
            block->ssa = std::make_unique<BasicBlock::SSAData>(block);
        }
        
        m_graph.m_argumentFormats.resize(m_graph.m_arguments.size());
        for (unsigned i = m_graph.m_arguments.size(); i--;) {
            FlushFormat format = FlushedJSValue;

            Node* node = m_argumentMapping.get(m_graph.m_arguments[i]);
            
            RELEASE_ASSERT(node);
            format = node->stackAccessData()->format;
            
            m_graph.m_argumentFormats[i] = format;
            m_graph.m_arguments[i] = node; // Record the load that loads the arguments for the benefit of exit profiling.
        }
        
        m_graph.m_form = SSA;

        if (verbose) {
            dataLog("Graph after SSA transformation:\n");
            m_graph.dump();
        }

        return true;
    }

private:
    SSACalculator m_calculator;
    InsertionSet m_insertionSet;
    HashMap<VariableAccessData*, SSACalculator::Variable*> m_ssaVariableForVariable;
    HashMap<Node*, Node*> m_argumentMapping;
    HashSet<Node*> m_argumentGetters;
    Vector<VariableAccessData*> m_variableForSSAIndex;
};

bool performSSAConversion(Graph& graph)
{
    return runPhase<SSAConversionPhase>(graph);
}

} } // namespace JSC::DFG

#endif // ENABLE(DFG_JIT)

