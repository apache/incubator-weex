/*
 * Copyright (C) 2013, 2014 Apple Inc. All rights reserved.
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
#include "DFGBlockInsertionSet.h"

#if ENABLE(DFG_JIT)

#include "JSCInlines.h"

namespace JSC { namespace DFG {

BlockInsertionSet::BlockInsertionSet(Graph& graph)
    : m_graph(graph)
{
}

BlockInsertionSet::~BlockInsertionSet() { }

void BlockInsertionSet::insert(const BlockInsertion& insertion)
{
    m_insertions.append(insertion);
}

void BlockInsertionSet::insert(size_t index, Ref<BasicBlock>&& block)
{
    insert(BlockInsertion(index, WTFMove(block)));
}

BasicBlock* BlockInsertionSet::insert(size_t index, float executionCount)
{
    Ref<BasicBlock> block = adoptRef(*new BasicBlock(
        UINT_MAX,
        m_graph.block(0)->variablesAtHead.numberOfArguments(),
        m_graph.block(0)->variablesAtHead.numberOfLocals(),
        executionCount));
    block->isReachable = true;
    auto* result = block.ptr();
    insert(index, WTFMove(block));
    return result;
}

BasicBlock* BlockInsertionSet::insertBefore(BasicBlock* before, float executionCount)
{
    return insert(before->index, executionCount);
}

bool BlockInsertionSet::execute()
{
    if (m_insertions.isEmpty())
        return false;
    
    // We allow insertions to be given to us in any order. So, we need to sort them before
    // running WTF::executeInsertions. Also, we don't really care if the sort is stable since
    // basic block order doesn't have semantics - it's just to make code easier to read.
    std::sort(m_insertions.begin(), m_insertions.end());

    executeInsertions(m_graph.m_blocks, m_insertions);
    
    // Prune out empty entries. This isn't strictly necessary but it's
    // healthy to keep the block list from growing.
    unsigned targetIndex = 0;
    for (unsigned sourceIndex = 0; sourceIndex < m_graph.m_blocks.size();) {
        RefPtr<BasicBlock> block = m_graph.m_blocks[sourceIndex++];
        if (!block)
            continue;
        m_graph.m_blocks[targetIndex++] = block;
    }
    m_graph.m_blocks.resize(targetIndex);
    
    // Make sure that the blocks know their new indices.
    for (unsigned i = 0; i < m_graph.m_blocks.size(); ++i)
        m_graph.m_blocks[i]->index = i;
    
    // And finally, invalidate all analyses that rely on the CFG.
    m_graph.invalidateCFG();
    m_graph.dethread();
    
    return true;
}

} } // namespace JSC::DFG

#endif // ENABLE(DFG_JIT)

