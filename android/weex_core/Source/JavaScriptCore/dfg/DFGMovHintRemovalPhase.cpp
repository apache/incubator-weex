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
#include "DFGMovHintRemovalPhase.h"

#if ENABLE(DFG_JIT)

#include "BytecodeLivenessAnalysisInlines.h"
#include "DFGEpoch.h"
#include "DFGForAllKills.h"
#include "DFGGraph.h"
#include "DFGInsertionSet.h"
#include "DFGMayExit.h"
#include "DFGPhase.h"
#include "JSCInlines.h"
#include "OperandsInlines.h"

namespace JSC { namespace DFG {

namespace {

bool verbose = false;

class MovHintRemovalPhase : public Phase {
public:
    MovHintRemovalPhase(Graph& graph)
        : Phase(graph, "MovHint removal")
        , m_state(OperandsLike, graph.block(0)->variablesAtHead)
        , m_changed(false)
    {
    }
    
    bool run()
    {
        if (verbose) {
            dataLog("Graph before MovHint removal:\n");
            m_graph.dump();
        }
        
        for (BasicBlock* block : m_graph.blocksInNaturalOrder())
            handleBlock(block);
        
        return m_changed;
    }

private:
    void handleBlock(BasicBlock* block)
    {
        if (verbose)
            dataLog("Handing block ", pointerDump(block), "\n");
        
        // A MovHint is unnecessary if the local dies before it is used. We answer this question by
        // maintaining the current exit epoch, and associating an epoch with each local. When a
        // local dies, it gets the current exit epoch. If a MovHint occurs in the same epoch as its
        // local, then it means there was no exit between the local's death and the MovHint - i.e.
        // the MovHint is unnecessary.
        
        Epoch currentEpoch = Epoch::first();
        
        m_state.fill(Epoch());
        m_graph.forAllLiveInBytecode(
            block->terminal()->origin.forExit,
            [&] (VirtualRegister reg) {
                m_state.operand(reg) = currentEpoch;
            });
        
        if (verbose)
            dataLog("    Locals: ", m_state, "\n");
        
        // Assume that blocks after us exit.
        currentEpoch.bump();
        
        for (unsigned nodeIndex = block->size(); nodeIndex--;) {
            Node* node = block->at(nodeIndex);
            
            if (node->op() == MovHint) {
                Epoch localEpoch = m_state.operand(node->unlinkedLocal());
                if (verbose)
                    dataLog("    At ", node, ": current = ", currentEpoch, ", local = ", localEpoch, "\n");
                if (!localEpoch || localEpoch == currentEpoch) {
                    node->setOpAndDefaultFlags(ZombieHint);
                    node->child1() = Edge();
                    m_changed = true;
                }
                m_state.operand(node->unlinkedLocal()) = Epoch();
            }
            
            if (mayExit(m_graph, node) != DoesNotExit)
                currentEpoch.bump();
            
            if (nodeIndex) {
                forAllKilledOperands(
                    m_graph, block->at(nodeIndex - 1), node,
                    [&] (VirtualRegister reg) {
                        // This function is a bit sloppy - it might claim to kill a local even if
                        // it's still live after. We need to protect against that.
                        if (!!m_state.operand(reg))
                            return;
                        
                        if (verbose)
                            dataLog("    Killed operand at ", node, ": ", reg, "\n");
                        m_state.operand(reg) = currentEpoch;
                    });
            }
        }
    }
    
    Operands<Epoch> m_state;
    bool m_changed;
};

} // anonymous namespace
    
bool performMovHintRemoval(Graph& graph)
{
    return runPhase<MovHintRemovalPhase>(graph);
}

} } // namespace JSC::DFG

#endif // ENABLE(DFG_JIT)

