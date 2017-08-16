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

#pragma once

#if ENABLE(B3_JIT)

#include <wtf/GraphNodeWorklist.h>
#include <wtf/IndexSet.h>
#include <wtf/Vector.h>

namespace JSC { namespace B3 {

template<typename BasicBlock>
bool addPredecessor(BasicBlock* block, BasicBlock* predecessor)
{
    auto& predecessors = block->predecessors();

    if (predecessors.contains(predecessor))
        return false;

    predecessors.append(predecessor);
    return true;
}

template<typename BasicBlock>
bool removePredecessor(BasicBlock* block, BasicBlock* predecessor)
{
    auto& predecessors = block->predecessors();
    for (unsigned i = 0; i < predecessors.size(); ++i) {
        if (predecessors[i] == predecessor) {
            predecessors[i--] = predecessors.last();
            predecessors.removeLast();
            ASSERT(!predecessors.contains(predecessor));
            return true;
        }
    }
    return false;
}

template<typename BasicBlock>
bool replacePredecessor(BasicBlock* block, BasicBlock* from, BasicBlock* to)
{
    bool changed = false;
    // We do it this way because 'to' may already be a predecessor of 'block'.
    changed |= removePredecessor(block, from);
    changed |= addPredecessor(block, to);
    return changed;
}

template<typename BasicBlock>
void updatePredecessorsAfter(BasicBlock* root)
{
    Vector<BasicBlock*, 16> worklist;
    worklist.append(root);
    while (!worklist.isEmpty()) {
        BasicBlock* block = worklist.takeLast();
        for (BasicBlock* successor : block->successorBlocks()) {
            if (addPredecessor(successor, block))
                worklist.append(successor);
        }
    }
}

template<typename BasicBlock>
void clearPredecessors(Vector<std::unique_ptr<BasicBlock>>& blocks)
{
    for (auto& block : blocks) {
        if (block)
            block->predecessors().resize(0);
    }
}

template<typename BasicBlock>
void recomputePredecessors(Vector<std::unique_ptr<BasicBlock>>& blocks)
{
    clearPredecessors(blocks);
    updatePredecessorsAfter(blocks[0].get());
}

template<typename BasicBlock>
bool isBlockDead(BasicBlock* block)
{
    if (!block)
        return false;
    if (!block->index())
        return false;
    return block->predecessors().isEmpty();
}

template<typename BasicBlock>
Vector<BasicBlock*> blocksInPreOrder(BasicBlock* root)
{
    Vector<BasicBlock*> result;
    GraphNodeWorklist<BasicBlock*, IndexSet<BasicBlock>> worklist;
    worklist.push(root);
    while (BasicBlock* block = worklist.pop()) {
        result.append(block);
        for (BasicBlock* successor : block->successorBlocks())
            worklist.push(successor);
    }
    return result;
}

template<typename BasicBlock>
Vector<BasicBlock*> blocksInPostOrder(BasicBlock* root)
{
    Vector<BasicBlock*> result;
    PostOrderGraphNodeWorklist<BasicBlock*, IndexSet<BasicBlock>> worklist;
    worklist.push(root);
    while (GraphNodeWithOrder<BasicBlock*> item = worklist.pop()) {
        switch (item.order) {
        case GraphVisitOrder::Pre:
            worklist.pushPost(item.node);
            for (BasicBlock* successor : item.node->successorBlocks())
                worklist.push(successor);
            break;
        case GraphVisitOrder::Post:
            result.append(item.node);
            break;
        }
    }
    return result;
}

} } // namespace JSC::B3

#endif // ENABLE(B3_JIT)
