/*
 * Copyright (C) 2013-2016 Apple Inc. All rights reserved.
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
#include "DFGLICMPhase.h"

#if ENABLE(DFG_JIT)

#include "DFGAbstractInterpreterInlines.h"
#include "DFGAtTailAbstractState.h"
#include "DFGBasicBlockInlines.h"
#include "DFGClobberSet.h"
#include "DFGClobberize.h"
#include "DFGControlEquivalenceAnalysis.h"
#include "DFGEdgeDominates.h"
#include "DFGGraph.h"
#include "DFGInsertionSet.h"
#include "DFGMayExit.h"
#include "DFGNaturalLoops.h"
#include "DFGPhase.h"
#include "DFGSafeToExecute.h"
#include "JSCInlines.h"

namespace JSC { namespace DFG {

namespace {

struct LoopData {
    LoopData()
        : preHeader(nullptr)
    {
    }
    
    ClobberSet writes;
    BasicBlock* preHeader;
};

} // anonymous namespace

class LICMPhase : public Phase {
    static const bool verbose = false;
    
public:
    LICMPhase(Graph& graph)
        : Phase(graph, "LICM")
        , m_state(graph)
        , m_interpreter(graph, m_state)
    {
    }
    
    bool run()
    {
        DFG_ASSERT(m_graph, nullptr, m_graph.m_form == SSA);
        
        m_graph.ensureDominators();
        m_graph.ensureNaturalLoops();
        m_graph.ensureControlEquivalenceAnalysis();

        if (verbose) {
            dataLog("Graph before LICM:\n");
            m_graph.dump();
        }
        
        m_data.resize(m_graph.m_naturalLoops->numLoops());
        
        // Figure out the set of things each loop writes to, not including blocks that
        // belong to inner loops. We fix this later.
        for (BlockIndex blockIndex = m_graph.numBlocks(); blockIndex--;) {
            BasicBlock* block = m_graph.block(blockIndex);
            if (!block)
                continue;
            
            // Skip blocks that are proved to not execute.
            // FIXME: This shouldn't be needed.
            // https://bugs.webkit.org/show_bug.cgi?id=128584
            if (!block->cfaHasVisited)
                continue;
            
            const NaturalLoop* loop = m_graph.m_naturalLoops->innerMostLoopOf(block);
            if (!loop)
                continue;
            LoopData& data = m_data[loop->index()];
            for (auto* node : *block) {
                // Don't look beyond parts of the code that definitely always exit.
                // FIXME: This shouldn't be needed.
                // https://bugs.webkit.org/show_bug.cgi?id=128584
                if (node->op() == ForceOSRExit)
                    break;

                addWrites(m_graph, node, data.writes);
            }
        }
        
        // For each loop:
        // - Identify its pre-header.
        // - Make sure its outer loops know what it clobbers.
        for (unsigned loopIndex = m_graph.m_naturalLoops->numLoops(); loopIndex--;) {
            const NaturalLoop& loop = m_graph.m_naturalLoops->loop(loopIndex);
            LoopData& data = m_data[loop.index()];
            
            for (
                const NaturalLoop* outerLoop = m_graph.m_naturalLoops->innerMostOuterLoop(loop);
                outerLoop;
                outerLoop = m_graph.m_naturalLoops->innerMostOuterLoop(*outerLoop))
                m_data[outerLoop->index()].writes.addAll(data.writes);
            
            BasicBlock* header = loop.header();
            BasicBlock* preHeader = nullptr;
            unsigned numberOfPreHeaders = 0; // We're cool if this is 1.

            // This is guaranteed because we expect the CFG not to have unreachable code. Therefore, a
            // loop header must have a predecessor. (Also, we don't allow the root block to be a loop,
            // which cuts out the one other way of having a loop header with only one predecessor.)
            DFG_ASSERT(m_graph, header->at(0), header->predecessors.size() > 1);
            
            for (unsigned i = header->predecessors.size(); i--;) {
                BasicBlock* predecessor = header->predecessors[i];
                if (m_graph.m_dominators->dominates(header, predecessor))
                    continue;

                preHeader = predecessor;
                ++numberOfPreHeaders;
            }

            // We need to validate the pre-header. There are a bunch of things that could be wrong
            // about it:
            //
            // - There might be more than one. This means that pre-header creation either did not run,
            //   or some CFG transformation destroyed the pre-headers.
            //
            // - It may not be legal to exit at the pre-header. That would be a real bummer. Currently,
            //   LICM assumes that it can always hoist checks. See
            //   https://bugs.webkit.org/show_bug.cgi?id=148545. Though even with that fixed, we anyway
            //   would need to check if it's OK to exit at the pre-header since if we can't then we
            //   would have to restrict hoisting to non-exiting nodes.

            if (numberOfPreHeaders != 1)
                continue;

            // This is guaranteed because the header has multiple predecessors and critical edges are
            // broken. Therefore the predecessors must all have one successor, which implies that they
            // must end in a Jump.
            DFG_ASSERT(m_graph, preHeader->terminal(), preHeader->terminal()->op() == Jump);

            if (!preHeader->terminal()->origin.exitOK)
                continue;
            
            data.preHeader = preHeader;
        }
        
        m_graph.initializeNodeOwners();
        
        // Walk all basic blocks that belong to loops, looking for hoisting opportunities.
        // We try to hoist to the outer-most loop that permits it. Hoisting is valid if:
        // - The node doesn't write anything.
        // - The node doesn't read anything that the loop writes.
        // - The preHeader is valid (i.e. it passed the validation above).
        // - The preHeader's state at tail makes the node safe to execute.
        // - The loop's children all belong to nodes that strictly dominate the loop header.
        // - The preHeader's state at tail is still valid. This is mostly to save compile
        //   time and preserve some kind of sanity, if we hoist something that must exit.
        //
        // Also, we need to remember to:
        // - Update the state-at-tail with the node we hoisted, so future hoist candidates
        //   know about any type checks we hoisted.
        //
        // For maximum profit, we walk blocks in DFS order to ensure that we generally
        // tend to hoist dominators before dominatees.
        Vector<const NaturalLoop*> loopStack;
        bool changed = false;
        for (BasicBlock* block : m_graph.blocksInPreOrder()) {
            const NaturalLoop* loop = m_graph.m_naturalLoops->innerMostLoopOf(block);
            if (!loop)
                continue;
            
            loopStack.resize(0);
            for (
                const NaturalLoop* current = loop;
                current;
                current = m_graph.m_naturalLoops->innerMostOuterLoop(*current))
                loopStack.append(current);
            
            // Remember: the loop stack has the inner-most loop at index 0, so if we want
            // to bias hoisting to outer loops then we need to use a reverse loop.
            
            if (verbose) {
                dataLog(
                    "Attempting to hoist out of block ", *block, " in loops:\n");
                for (unsigned stackIndex = loopStack.size(); stackIndex--;) {
                    dataLog(
                        "        ", *loopStack[stackIndex], ", which writes ",
                        m_data[loopStack[stackIndex]->index()].writes, "\n");
                }
            }
            
            for (unsigned nodeIndex = 0; nodeIndex < block->size(); ++nodeIndex) {
                Node*& nodeRef = block->at(nodeIndex);
                if (doesWrites(m_graph, nodeRef)) {
                    if (verbose)
                        dataLog("    Not hoisting ", nodeRef, " because it writes things.\n");
                    continue;
                }

                for (unsigned stackIndex = loopStack.size(); stackIndex--;)
                    changed |= attemptHoist(block, nodeRef, loopStack[stackIndex]);
            }
        }
        
        return changed;
    }

private:
    bool attemptHoist(BasicBlock* fromBlock, Node*& nodeRef, const NaturalLoop* loop)
    {
        Node* node = nodeRef;
        LoopData& data = m_data[loop->index()];

        if (!data.preHeader) {
            if (verbose)
                dataLog("    Not hoisting ", node, " because the pre-header is invalid.\n");
            return false;
        }
        
        if (!data.preHeader->cfaDidFinish) {
            if (verbose)
                dataLog("    Not hoisting ", node, " because CFA is invalid.\n");
            return false;
        }
        
        if (!edgesDominate(m_graph, node, data.preHeader)) {
            if (verbose) {
                dataLog(
                    "    Not hoisting ", node, " because it isn't loop invariant.\n");
            }
            return false;
        }
        
        // FIXME: At this point if the hoisting of the full node fails but the node has type checks,
        // we could still hoist just the checks.
        // https://bugs.webkit.org/show_bug.cgi?id=144525
        
        if (readsOverlap(m_graph, node, data.writes)) {
            if (verbose) {
                dataLog(
                    "    Not hoisting ", node,
                    " because it reads things that the loop writes.\n");
            }
            return false;
        }
        
        m_state.initializeTo(data.preHeader);
        if (!safeToExecute(m_state, m_graph, node)) {
            if (verbose) {
                dataLog(
                    "    Not hoisting ", node, " because it isn't safe to execute.\n");
            }
            return false;
        }
        
        NodeOrigin originalOrigin = node->origin;

        // NOTE: We could just use BackwardsDominators here directly, since we already know that the
        // preHeader dominates fromBlock. But we wouldn't get anything from being so clever, since
        // dominance checks are O(1) and only a few integer compares.
        bool addsBlindSpeculation = mayExit(m_graph, node, m_state)
            && !m_graph.m_controlEquivalenceAnalysis->dominatesEquivalently(data.preHeader, fromBlock);
        
        if (addsBlindSpeculation
            && m_graph.baselineCodeBlockFor(originalOrigin.semantic)->hasExitSite(FrequentExitSite(HoistingFailed))) {
            if (verbose) {
                dataLog(
                    "    Not hoisting ", node, " because it may exit and the pre-header (",
                    *data.preHeader, ") is not control equivalent to the node's original block (",
                    *fromBlock, ") and hoisting had previously failed.\n");
            }
            return false;
        }
        
        if (verbose) {
            dataLog(
                "    Hoisting ", node, " from ", *fromBlock, " to ", *data.preHeader,
                "\n");
        }

        // FIXME: We should adjust the Check: flags on the edges of node. There are phases that assume
        // that those flags are correct even if AI is stale.
        // https://bugs.webkit.org/show_bug.cgi?id=148544
        data.preHeader->insertBeforeTerminal(node);
        node->owner = data.preHeader;
        NodeOrigin terminalOrigin = data.preHeader->terminal()->origin;
        node->origin = terminalOrigin.withSemantic(node->origin.semantic);
        node->origin.wasHoisted |= addsBlindSpeculation;
        
        // Modify the states at the end of the preHeader of the loop we hoisted to,
        // and all pre-headers inside the loop. This isn't a stability bottleneck right now
        // because most loops are small and most blocks belong to few loops.
        for (unsigned bodyIndex = loop->size(); bodyIndex--;) {
            BasicBlock* subBlock = loop->at(bodyIndex);
            const NaturalLoop* subLoop = m_graph.m_naturalLoops->headerOf(subBlock);
            if (!subLoop)
                continue;
            BasicBlock* subPreHeader = m_data[subLoop->index()].preHeader;
            // We may not have given this loop a pre-header because either it didn't have exitOK
            // or the header had multiple predecessors that it did not dominate. In that case the
            // loop wouldn't be a hoisting candidate anyway, so we don't have to do anything.
            if (!subPreHeader)
                continue;
            // The pre-header's tail may be unreachable, in which case we have nothing to do.
            if (!subPreHeader->cfaDidFinish)
                continue;
            m_state.initializeTo(subPreHeader);
            m_interpreter.execute(node);
        }
        
        // It just so happens that all of the nodes we currently know how to hoist
        // don't have var-arg children. That may change and then we can fix this
        // code. But for now we just assert that's the case.
        DFG_ASSERT(m_graph, node, !(node->flags() & NodeHasVarArgs));
        
        nodeRef = m_graph.addNode(Check, originalOrigin, node->children);
        
        return true;
    }
    
    AtTailAbstractState m_state;
    AbstractInterpreter<AtTailAbstractState> m_interpreter;
    Vector<LoopData> m_data;
};

bool performLICM(Graph& graph)
{
    return runPhase<LICMPhase>(graph);
}

} } // namespace JSC::DFG

#endif // ENABLE(DFG_JIT)

