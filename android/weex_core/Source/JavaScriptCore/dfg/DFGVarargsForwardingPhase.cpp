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
#include "DFGVarargsForwardingPhase.h"

#if ENABLE(DFG_JIT)

#include "DFGArgumentsUtilities.h"
#include "DFGClobberize.h"
#include "DFGForAllKills.h"
#include "DFGGraph.h"
#include "DFGPhase.h"
#include "JSCInlines.h"
#include <wtf/ListDump.h>

namespace JSC { namespace DFG {

namespace {

bool verbose = false;

class VarargsForwardingPhase : public Phase {
public:
    VarargsForwardingPhase(Graph& graph)
        : Phase(graph, "varargs forwarding")
    {
    }
    
    bool run()
    {
        DFG_ASSERT(m_graph, nullptr, m_graph.m_form != SSA);
        
        if (verbose) {
            dataLog("Graph before varargs forwarding:\n");
            m_graph.dump();
        }
        
        m_changed = false;
        for (BasicBlock* block : m_graph.blocksInNaturalOrder())
            handleBlock(block);
        return m_changed;
    }

private:
    void handleBlock(BasicBlock* block)
    {
        for (unsigned nodeIndex = 0; nodeIndex < block->size(); ++nodeIndex) {
            Node* node = block->at(nodeIndex);
            switch (node->op()) {
            case CreateDirectArguments:
            case CreateClonedArguments:
                handleCandidate(block, nodeIndex);
                break;
            default:
                break;
            }
        }
    }
    
    void handleCandidate(BasicBlock* block, unsigned candidateNodeIndex)
    {
        // We expect calls into this function to be rare. So, this is written in a simple O(n) manner.
        
        Node* candidate = block->at(candidateNodeIndex);
        if (verbose)
            dataLog("Handling candidate ", candidate, "\n");
        
        // Find the index of the last node in this block to use the candidate, and look for escaping
        // sites.
        unsigned lastUserIndex = candidateNodeIndex;
        Vector<VirtualRegister, 2> relevantLocals; // This is a set. We expect it to be a small set.
        for (unsigned nodeIndex = candidateNodeIndex + 1; nodeIndex < block->size(); ++nodeIndex) {
            Node* node = block->at(nodeIndex);
            
            switch (node->op()) {
            case MovHint:
                if (node->child1() != candidate)
                    break;
                lastUserIndex = nodeIndex;
                if (!relevantLocals.contains(node->unlinkedLocal()))
                    relevantLocals.append(node->unlinkedLocal());
                break;
                
            case Check: {
                bool sawEscape = false;
                m_graph.doToChildren(
                    node,
                    [&] (Edge edge) {
                        if (edge == candidate)
                            lastUserIndex = nodeIndex;
                        
                        if (edge.willNotHaveCheck())
                            return;
                        
                        if (alreadyChecked(edge.useKind(), SpecObject))
                            return;
                        
                        sawEscape = true;
                    });
                if (sawEscape) {
                    if (verbose)
                        dataLog("    Escape at ", node, "\n");
                    return;
                }
                break;
            }
                
            case LoadVarargs:
                if (m_graph.uses(node, candidate))
                    lastUserIndex = nodeIndex;
                break;
                
            case CallVarargs:
            case ConstructVarargs:
            case TailCallVarargs:
            case TailCallVarargsInlinedCaller:
                if (node->child1() == candidate || node->child2() == candidate) {
                    if (verbose)
                        dataLog("    Escape at ", node, "\n");
                    return;
                }
                if (node->child2() == candidate)
                    lastUserIndex = nodeIndex;
                break;
                
            case SetLocal:
                if (node->child1() == candidate && node->variableAccessData()->isLoadedFrom()) {
                    if (verbose)
                        dataLog("    Escape at ", node, "\n");
                    return;
                }
                break;
                
            default:
                if (m_graph.uses(node, candidate)) {
                    if (verbose)
                        dataLog("    Escape at ", node, "\n");
                    return;
                }
            }
            
            forAllKilledOperands(
                m_graph, node, block->tryAt(nodeIndex + 1),
                [&] (VirtualRegister reg) {
                    if (verbose)
                        dataLog("    Killing ", reg, " while we are interested in ", listDump(relevantLocals), "\n");
                    for (unsigned i = 0; i < relevantLocals.size(); ++i) {
                        if (relevantLocals[i] == reg) {
                            relevantLocals[i--] = relevantLocals.last();
                            relevantLocals.removeLast();
                            lastUserIndex = nodeIndex;
                        }
                    }
                });
        }
        if (verbose)
            dataLog("Selected lastUserIndex = ", lastUserIndex, ", ", block->at(lastUserIndex), "\n");
        
        // We're still in business. Determine if between the candidate and the last user there is any
        // effect that could interfere with sinking.
        for (unsigned nodeIndex = candidateNodeIndex + 1; nodeIndex <= lastUserIndex; ++nodeIndex) {
            Node* node = block->at(nodeIndex);
            
            // We have our own custom switch to detect some interferences that clobberize() wouldn't know
            // about, and also some of the common ones, too. In particular, clobberize() doesn't know
            // that Flush, MovHint, ZombieHint, and KillStack are bad because it's not worried about
            // what gets read on OSR exit.
            switch (node->op()) {
            case MovHint:
            case ZombieHint:
            case KillStack:
                if (argumentsInvolveStackSlot(candidate, node->unlinkedLocal())) {
                    if (verbose)
                        dataLog("    Interference at ", node, "\n");
                    return;
                }
                break;
                
            case PutStack:
                if (argumentsInvolveStackSlot(candidate, node->stackAccessData()->local)) {
                    if (verbose)
                        dataLog("    Interference at ", node, "\n");
                    return;
                }
                break;
                
            case SetLocal:
            case Flush:
                if (argumentsInvolveStackSlot(candidate, node->local())) {
                    if (verbose)
                        dataLog("    Interference at ", node, "\n");
                    return;
                }
                break;
                
            default: {
                bool doesInterfere = false;
                clobberize(
                    m_graph, node, NoOpClobberize(),
                    [&] (AbstractHeap heap) {
                        if (heap.kind() != Stack) {
                            ASSERT(!heap.overlaps(Stack));
                            return;
                        }
                        ASSERT(!heap.payload().isTop());
                        VirtualRegister reg(heap.payload().value32());
                        if (argumentsInvolveStackSlot(candidate, reg))
                            doesInterfere = true;
                    },
                    NoOpClobberize());
                if (doesInterfere) {
                    if (verbose)
                        dataLog("    Interference at ", node, "\n");
                    return;
                }
            } }
        }
        
        // We can make this work.
        if (verbose)
            dataLog("    Will do forwarding!\n");
        m_changed = true;
        
        // Transform the program.
        switch (candidate->op()) {
        case CreateDirectArguments:
            candidate->setOpAndDefaultFlags(PhantomDirectArguments);
            break;

        case CreateClonedArguments:
            candidate->setOpAndDefaultFlags(PhantomClonedArguments);
            break;
            
        default:
            DFG_CRASH(m_graph, candidate, "bad node type");
            break;
        }
        for (unsigned nodeIndex = candidateNodeIndex + 1; nodeIndex <= lastUserIndex; ++nodeIndex) {
            Node* node = block->at(nodeIndex);
            switch (node->op()) {
            case Check:
            case MovHint:
            case PutHint:
                // We don't need to change anything with these.
                break;
                
            case LoadVarargs:
                if (node->child1() != candidate)
                    break;
                node->setOpAndDefaultFlags(ForwardVarargs);
                break;
                
            case CallVarargs:
                if (node->child3() != candidate)
                    break;
                node->setOpAndDefaultFlags(CallForwardVarargs);
                break;
                
            case ConstructVarargs:
                if (node->child3() != candidate)
                    break;
                node->setOpAndDefaultFlags(ConstructForwardVarargs);
                break;

            case TailCallVarargs:
                if (node->child3() != candidate)
                    break;
                node->setOpAndDefaultFlags(TailCallForwardVarargs);
                break;

            case TailCallVarargsInlinedCaller:
                if (node->child3() != candidate)
                    break;
                node->setOpAndDefaultFlags(TailCallForwardVarargsInlinedCaller);
                break;

            case SetLocal:
                // This is super odd. We don't have to do anything here, since in DFG IR, the phantom
                // arguments nodes do produce a JSValue. Also, we know that if this SetLocal referenecs a
                // candidate then the SetLocal - along with all of its references - will die off pretty
                // soon, since it has no real users. DCE will surely kill it. If we make it to SSA, then
                // SSA conversion will kill it.
                break;
                
            default:
                if (ASSERT_DISABLED)
                    break;
                m_graph.doToChildren(
                    node,
                    [&] (Edge edge) {
                        DFG_ASSERT(m_graph, node, edge != candidate);
                    });
                break;
            }
        }
    }
    
    bool m_changed;
};

} // anonymous namespace

bool performVarargsForwarding(Graph& graph)
{
    return runPhase<VarargsForwardingPhase>(graph);
}

} } // namespace JSC::DFG

#endif // ENABLE(DFG_JIT)

