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
#include "DFGCombinedLiveness.h"

#if ENABLE(DFG_JIT)

#include "DFGAvailabilityMap.h"
#include "DFGBlockMapInlines.h"
#include "FullBytecodeLiveness.h"
#include "JSCInlines.h"

namespace JSC { namespace DFG {

NodeSet liveNodesAtHead(Graph& graph, BasicBlock* block)
{
    NodeSet seen;
    for (NodeFlowProjection node : block->ssa->liveAtHead) {
        if (node.kind() == NodeFlowProjection::Primary)
            seen.addVoid(node.node());
    }
    
    AvailabilityMap& availabilityMap = block->ssa->availabilityAtHead;
    graph.forAllLocalsLiveInBytecode(
        block->at(0)->origin.forExit,
        [&] (VirtualRegister reg) {
            availabilityMap.closeStartingWithLocal(
                reg,
                [&] (Node* node) -> bool {
                    return seen.contains(node);
                },
                [&] (Node* node) -> bool {
                    return seen.add(node).isNewEntry;
                });
        });
    
    return seen;
}

CombinedLiveness::CombinedLiveness(Graph& graph)
    : liveAtHead(graph)
    , liveAtTail(graph)
{
    // First compute the liveAtHead for each block.
    for (BasicBlock* block : graph.blocksInNaturalOrder())
        liveAtHead[block] = liveNodesAtHead(graph, block);
    
    // Now compute the liveAtTail by unifying the liveAtHead of the successors.
    for (BasicBlock* block : graph.blocksInNaturalOrder()) {
        for (BasicBlock* successor : block->successors()) {
            for (Node* node : liveAtHead[successor])
                liveAtTail[block].addVoid(node);
        }
    }
}

} } // namespace JSC::DFG

#endif // ENABLE(DFG_JIT)

