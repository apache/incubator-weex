/**
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */
#pragma once

#include "DFGCombinedLiveness.h"
#include "DFGGraph.h"
#include "DFGOSRAvailabilityAnalysisPhase.h"
#include "FullBytecodeLiveness.h"

namespace JSC { namespace DFG {

// Utilities for finding the last points where a node is live in DFG SSA. This accounts for liveness due
// to OSR exit. This is usually used for enumerating over all of the program points where a node is live,
// by exploring all blocks where the node is live at tail and then exploring all program points where the
// node is killed. A prerequisite to using these utilities is having liveness and OSR availability
// computed.

// This tells you those things that die on the boundary between nodeBefore and nodeAfter. It is
// conservative in the sense that it might resort to telling you some things that are still live at
// nodeAfter.
template<typename Functor>
void forAllKilledOperands(Graph& graph, Node* nodeBefore, Node* nodeAfter, const Functor& functor)
{
    CodeOrigin before = nodeBefore->origin.forExit;

    if (!nodeAfter) {
        graph.forAllLiveInBytecode(before, functor);
        return;
    }
    
    CodeOrigin after = nodeAfter->origin.forExit;
    
    VirtualRegister alreadyNoted;
    // If we MovHint something that is live at the time, then we kill the old value.
    if (nodeAfter->containsMovHint()) {
        VirtualRegister reg = nodeAfter->unlinkedLocal();
        if (graph.isLiveInBytecode(reg, after)) {
            functor(reg);
            alreadyNoted = reg;
        }
    }
    
    if (before == after)
        return;
    
    // It's easier to do this if the inline call frames are the same. This is way faster than the
    // other loop, below.
    if (before.inlineCallFrame == after.inlineCallFrame) {
        int stackOffset = before.inlineCallFrame ? before.inlineCallFrame->stackOffset : 0;
        CodeBlock* codeBlock = graph.baselineCodeBlockFor(before.inlineCallFrame);
        FullBytecodeLiveness& fullLiveness = graph.livenessFor(codeBlock);
        const FastBitVector& liveBefore = fullLiveness.getLiveness(before.bytecodeIndex);
        const FastBitVector& liveAfter = fullLiveness.getLiveness(after.bytecodeIndex);
        
        (liveBefore & ~liveAfter).forEachSetBit(
            [&] (size_t relativeLocal) {
                functor(virtualRegisterForLocal(relativeLocal) + stackOffset);
            });
        return;
    }
    
    // Detect kills the super conservative way: it is killed if it was live before and dead after.
    BitVector liveAfter = graph.localsLiveInBytecode(after);
    graph.forAllLocalsLiveInBytecode(
        before,
        [&] (VirtualRegister reg) {
            if (reg == alreadyNoted)
                return;
            if (liveAfter.get(reg.toLocal()))
                return;
            functor(reg);
        });
}
    
// Tells you all of the nodes that would no longer be live across the node at this nodeIndex.
template<typename Functor>
void forAllKilledNodesAtNodeIndex(
    Graph& graph, AvailabilityMap& availabilityMap, BasicBlock* block, unsigned nodeIndex,
    const Functor& functor)
{
    static const unsigned seenInClosureFlag = 1;
    static const unsigned calledFunctorFlag = 2;
    HashMap<Node*, unsigned> flags;
    
    Node* node = block->at(nodeIndex);
    
    graph.doToChildren(
        node,
        [&] (Edge edge) {
            if (edge.doesKill()) {
                auto& result = flags.add(edge.node(), 0).iterator->value;
                if (!(result & calledFunctorFlag)) {
                    functor(edge.node());
                    result |= calledFunctorFlag;
                }
            }
        });

    Node* before = nullptr;
    if (nodeIndex)
        before = block->at(nodeIndex - 1);

    forAllKilledOperands(
        graph, before, node,
        [&] (VirtualRegister reg) {
            availabilityMap.closeStartingWithLocal(
                reg,
                [&] (Node* node) -> bool {
                    return flags.get(node) & seenInClosureFlag;
                },
                [&] (Node* node) -> bool {
                    auto& resultFlags = flags.add(node, 0).iterator->value;
                    bool result = resultFlags & seenInClosureFlag;
                    if (!(resultFlags & calledFunctorFlag))
                        functor(node);
                    resultFlags |= seenInClosureFlag | calledFunctorFlag;
                    return result;
                });
        });
}

// Tells you all of the places to start searching from in a basic block. Gives you the node index at which
// the value is either no longer live. This pretends that nodes are dead at the end of the block, so that
// you can use this to do per-basic-block analyses.
template<typename Functor>
void forAllKillsInBlock(
    Graph& graph, const CombinedLiveness& combinedLiveness, BasicBlock* block,
    const Functor& functor)
{
    for (Node* node : combinedLiveness.liveAtTail[block])
        functor(block->size(), node);
    
    LocalOSRAvailabilityCalculator localAvailability(graph);
    localAvailability.beginBlock(block);
    // Start at the second node, because the functor is expected to only inspect nodes from the start of
    // the block up to nodeIndex (exclusive), so if nodeIndex is zero then the functor has nothing to do.
    for (unsigned nodeIndex = 1; nodeIndex < block->size(); ++nodeIndex) {
        forAllKilledNodesAtNodeIndex(
            graph, localAvailability.m_availability, block, nodeIndex,
            [&] (Node* node) {
                functor(nodeIndex, node);
            });
        localAvailability.executeNode(block->at(nodeIndex));
    }
}

} } // namespace JSC::DFG
