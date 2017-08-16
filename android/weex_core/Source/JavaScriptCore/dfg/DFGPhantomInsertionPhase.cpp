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
#include "DFGPhantomInsertionPhase.h"

#if ENABLE(DFG_JIT)

#include "BytecodeLivenessAnalysisInlines.h"
#include "DFGForAllKills.h"
#include "DFGGraph.h"
#include "DFGInsertionSet.h"
#include "DFGMayExit.h"
#include "DFGPhase.h"
#include "DFGPredictionPropagationPhase.h"
#include "DFGVariableAccessDataDump.h"
#include "JSCInlines.h"
#include "OperandsInlines.h"

namespace JSC { namespace DFG {

namespace {

bool verbose = false;

class PhantomInsertionPhase : public Phase {
public:
    PhantomInsertionPhase(Graph& graph)
        : Phase(graph, "phantom insertion")
        , m_insertionSet(graph)
        , m_values(OperandsLike, graph.block(0)->variablesAtHead)
    {
    }
    
    bool run()
    {
        // We assume that DCE has already run. If we run before DCE then we think that all
        // SetLocals execute, which is inaccurate. That causes us to insert too few Phantoms.
        DFG_ASSERT(m_graph, nullptr, m_graph.m_refCountState == ExactRefCount);
        
        if (verbose) {
            dataLog("Graph before Phantom insertion:\n");
            m_graph.dump();
        }
        
        m_graph.clearEpochs();
        
        for (BasicBlock* block : m_graph.blocksInNaturalOrder())
            handleBlock(block);
        
        if (verbose) {
            dataLog("Graph after Phantom insertion:\n");
            m_graph.dump();
        }
        
        return true;
    }

private:
    void handleBlock(BasicBlock* block)
    {
        // FIXME: For blocks that have low register pressure, it would make the most sense to
        // simply insert Phantoms at the last point possible since that would obviate the need to
        // query bytecode liveness:
        //
        // - If we MovHint @x into loc42 then put a Phantom on the last MovHinted value in loc42.
        // - At the end of the block put Phantoms for each MovHinted value.
        //
        // This will definitely not work if there are any phantom allocations. For those blocks
        // where this would be legal, it remains to be seen how profitable it would be even if there
        // was high register pressure. After all, a Phantom would cause a spill but it wouldn't
        // cause a fill.
        //
        // https://bugs.webkit.org/show_bug.cgi?id=144524
        
        m_values.fill(nullptr);

        Epoch currentEpoch = Epoch::first();
        unsigned lastExitingIndex = 0;
        for (unsigned nodeIndex = 0; nodeIndex < block->size(); ++nodeIndex) {
            Node* node = block->at(nodeIndex);
            if (verbose)
                dataLog("Considering ", node, "\n");
            
            switch (node->op()) {
            case MovHint:
                m_values.operand(node->unlinkedLocal()) = node->child1().node();
                break;
                
            case ZombieHint:
                m_values.operand(node->unlinkedLocal()) = nullptr;
                break;

            case GetLocal:
            case SetArgument:
                m_values.operand(node->local()) = nullptr;
                break;
                
            default:
                break;
            }

            bool nodeMayExit = mayExit(m_graph, node) != DoesNotExit;
            if (nodeMayExit) {
                currentEpoch.bump();
                lastExitingIndex = nodeIndex;
            }
            
            m_graph.doToChildren(
                node,
                [&] (Edge edge) {
                    edge->setEpoch(currentEpoch);
                });
            
            node->setEpoch(currentEpoch);

            VirtualRegister alreadyKilled;

            auto processKilledOperand = [&] (VirtualRegister reg) {
                if (verbose)
                    dataLog("    Killed operand: ", reg, "\n");

                // Already handled from SetLocal.
                if (reg == alreadyKilled)
                    return;
                
                Node* killedNode = m_values.operand(reg);
                if (!killedNode)
                    return;
                
                // We only need to insert a Phantom if the node hasn't been used since the last
                // exit, and was born before the last exit.
                if (killedNode->epoch() == currentEpoch)
                    return;
                
                if (verbose) {
                    dataLog(
                        "    Inserting Phantom on ", killedNode, " after ",
                        block->at(lastExitingIndex), "\n");
                }
                
                // We have exact ref counts, so creating a new use means that we have to
                // increment the ref count.
                killedNode->postfixRef();

                Node* lastExitingNode = block->at(lastExitingIndex);
                
                m_insertionSet.insertNode(
                    lastExitingIndex + 1, SpecNone, Phantom,
                    lastExitingNode->origin.forInsertingAfter(m_graph, lastExitingNode),
                    killedNode->defaultEdge());
            };

            if (node->op() == SetLocal) {
                VirtualRegister local = node->local();
                if (nodeMayExit) {
                    // If the SetLocal does exit, we need the MovHint of its local
                    // to be live until the SetLocal is done.
                    processKilledOperand(local);
                    alreadyKilled = local;
                }
                m_values.operand(local) = nullptr;
            }

            forAllKilledOperands(m_graph, node, block->tryAt(nodeIndex + 1), processKilledOperand);
        }
        
        m_insertionSet.execute(block);
    }
    
    InsertionSet m_insertionSet;
    Operands<Node*> m_values;
};

} // anonymous namespace
    
bool performPhantomInsertion(Graph& graph)
{
    return runPhase<PhantomInsertionPhase>(graph);
}

} } // namespace JSC::DFG

#endif // ENABLE(DFG_JIT)

