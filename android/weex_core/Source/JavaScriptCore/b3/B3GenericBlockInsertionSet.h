/*
 * Copyright (C) 2017 Apple Inc. All rights reserved.
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

#include "PureNaN.h"
#include <climits>
#include <wtf/BubbleSort.h>
#include <wtf/Insertion.h>
#include <wtf/Vector.h>

namespace JSC { namespace B3 {

class InsertionSet;

template<typename BasicBlock>
class GenericBlockInsertionSet {
public:
    typedef WTF::Insertion<std::unique_ptr<BasicBlock>> BlockInsertion;
    
    GenericBlockInsertionSet(Vector<std::unique_ptr<BasicBlock>>& blocks)
        : m_blocks(blocks)
    {
    }
    
    void insert(BlockInsertion&& insertion)
    {
        m_insertions.append(WTFMove(insertion));
    }

    // Insert a new block at a given index.
    BasicBlock* insert(unsigned index, double frequency = PNaN)
    {
        std::unique_ptr<BasicBlock> block(new BasicBlock(UINT_MAX, frequency));
        BasicBlock* result = block.get();
        insert(BlockInsertion(index, WTFMove(block)));
        return result;
    }

    // Inserts a new block before the given block. Usually you will not pass the frequency
    // argument. Passing PNaN causes us to just use the frequency of the 'before' block. That's
    // usually what you want.
    BasicBlock* insertBefore(BasicBlock* before, double frequency = PNaN)
    {
        return insert(before->index(), frequency == frequency ? frequency : before->frequency());
    }

    // Inserts a new block after the given block.
    BasicBlock* insertAfter(BasicBlock* after, double frequency = PNaN)
    {
        return insert(after->index() + 1, frequency == frequency ? frequency : after->frequency());
    }

    bool execute()
    {
        if (m_insertions.isEmpty())
            return false;
        
        // We allow insertions to be given to us in any order. So, we need to sort them before
        // running WTF::executeInsertions. We strongly prefer a stable sort and we want it to be
        // fast, so we use bubble sort.
        bubbleSort(m_insertions.begin(), m_insertions.end());
        
        executeInsertions(m_blocks, m_insertions);
        
        // Prune out empty entries. This isn't strictly necessary but it's
        // healthy to keep the block list from growing.
        m_blocks.removeAllMatching(
            [&] (std::unique_ptr<BasicBlock>& blockPtr) -> bool {
                return !blockPtr;
            });
        
        // Make sure that the blocks know their new indices.
        for (unsigned i = 0; i < m_blocks.size(); ++i)
            m_blocks[i]->m_index = i;
        
        return true;
    }

private:
    Vector<std::unique_ptr<BasicBlock>>& m_blocks;
    Vector<BlockInsertion, 8> m_insertions;
};

} } // namespace JSC::B3

#endif // ENABLE(B3_JIT)
