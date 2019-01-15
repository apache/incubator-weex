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

#include "BytecodeBasicBlock.h"
#include <wtf/IndexedContainerIterator.h>
#include <wtf/IteratorRange.h>
#include <wtf/Vector.h>

namespace JSC {

class BytecodeBasicBlock;

template<typename Block>
class BytecodeGraph {
    WTF_MAKE_FAST_ALLOCATED;
    WTF_MAKE_NONCOPYABLE(BytecodeGraph);
public:
    typedef Block CodeBlock;
    typedef typename Block::Instruction Instruction;
    typedef Vector<std::unique_ptr<BytecodeBasicBlock>> BasicBlocksVector;

    typedef WTF::IndexedContainerIterator<BytecodeGraph<Block>> iterator;

    inline BytecodeGraph(Block*, typename Block::UnpackedInstructions&);

    Block* codeBlock() const { return m_codeBlock; }

    typename Block::UnpackedInstructions& instructions() { return m_instructions; }

    WTF::IteratorRange<BasicBlocksVector::reverse_iterator> basicBlocksInReverseOrder()
    {
        return WTF::makeIteratorRange(m_basicBlocks.rbegin(), m_basicBlocks.rend());
    }

    static bool blockContainsBytecodeOffset(BytecodeBasicBlock* block, unsigned bytecodeOffset)
    {
        unsigned leaderOffset = block->leaderOffset();
        return bytecodeOffset >= leaderOffset && bytecodeOffset < leaderOffset + block->totalLength();
    }

    BytecodeBasicBlock* findBasicBlockForBytecodeOffset(unsigned bytecodeOffset)
    {
        /*
            for (unsigned i = 0; i < m_basicBlocks.size(); i++) {
                if (blockContainsBytecodeOffset(m_basicBlocks[i].get(), bytecodeOffset))
                    return m_basicBlocks[i].get();
            }
            return 0;
        */

        std::unique_ptr<BytecodeBasicBlock>* basicBlock = approximateBinarySearch<std::unique_ptr<BytecodeBasicBlock>, unsigned>(m_basicBlocks, m_basicBlocks.size(), bytecodeOffset, [] (std::unique_ptr<BytecodeBasicBlock>* basicBlock) { return (*basicBlock)->leaderOffset(); });
        // We found the block we were looking for.
        if (blockContainsBytecodeOffset((*basicBlock).get(), bytecodeOffset))
            return (*basicBlock).get();

        // Basic block is to the left of the returned block.
        if (bytecodeOffset < (*basicBlock)->leaderOffset()) {
            ASSERT(basicBlock - 1 >= m_basicBlocks.data());
            ASSERT(blockContainsBytecodeOffset(basicBlock[-1].get(), bytecodeOffset));
            return basicBlock[-1].get();
        }

        // Basic block is to the right of the returned block.
        ASSERT(&basicBlock[1] <= &m_basicBlocks.last());
        ASSERT(blockContainsBytecodeOffset(basicBlock[1].get(), bytecodeOffset));
        return basicBlock[1].get();
    }

    BytecodeBasicBlock* findBasicBlockWithLeaderOffset(unsigned leaderOffset)
    {
        return (*tryBinarySearch<std::unique_ptr<BytecodeBasicBlock>, unsigned>(m_basicBlocks, m_basicBlocks.size(), leaderOffset, [] (std::unique_ptr<BytecodeBasicBlock>* basicBlock) { return (*basicBlock)->leaderOffset(); })).get();
    }

    unsigned size() const { return m_basicBlocks.size(); }
    BytecodeBasicBlock* at(unsigned index) const { return m_basicBlocks[index].get(); }
    BytecodeBasicBlock* operator[](unsigned index) const { return at(index); }

    iterator begin() const { return iterator(*this, 0); }
    iterator end() const { return iterator(*this, size()); }
    BytecodeBasicBlock* first() { return at(0); }
    BytecodeBasicBlock* last() { return at(size() - 1); }

private:
    Block* m_codeBlock;
    BasicBlocksVector m_basicBlocks;
    typename Block::UnpackedInstructions& m_instructions;
};


template<typename Block>
BytecodeGraph<Block>::BytecodeGraph(Block* codeBlock, typename Block::UnpackedInstructions& instructions)
    : m_codeBlock(codeBlock)
    , m_instructions(instructions)
{
    ASSERT(m_codeBlock);
    BytecodeBasicBlock::compute(m_codeBlock, instructions.begin(), instructions.size(), m_basicBlocks);
    ASSERT(m_basicBlocks.size());
}

} // namespace JSC
