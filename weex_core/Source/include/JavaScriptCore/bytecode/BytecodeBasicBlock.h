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

#include <limits.h>
#include <wtf/FastBitVector.h>
#include <wtf/Vector.h>

namespace JSC {

class CodeBlock;
class UnlinkedCodeBlock;
struct Instruction;
struct UnlinkedInstruction;

class BytecodeBasicBlock {
    WTF_MAKE_FAST_ALLOCATED;
public:
    enum SpecialBlockType { EntryBlock, ExitBlock };
    BytecodeBasicBlock(unsigned start, unsigned length);
    BytecodeBasicBlock(SpecialBlockType);
    void shrinkToFit();

    bool isEntryBlock() { return !m_leaderOffset && !m_totalLength; }
    bool isExitBlock() { return m_leaderOffset == UINT_MAX && m_totalLength == UINT_MAX; }

    unsigned leaderOffset() { return m_leaderOffset; }
    unsigned totalLength() { return m_totalLength; }

    const Vector<unsigned>& offsets() const { return m_offsets; }

    const Vector<BytecodeBasicBlock*>& successors() const { return m_successors; }

    FastBitVector& in() { return m_in; }
    FastBitVector& out() { return m_out; }

    unsigned index() const { return m_index; }

    static void compute(CodeBlock*, Instruction* instructionsBegin, unsigned instructionCount, Vector<std::unique_ptr<BytecodeBasicBlock>>&);
    static void compute(UnlinkedCodeBlock*, UnlinkedInstruction* instructionsBegin, unsigned instructionCount, Vector<std::unique_ptr<BytecodeBasicBlock>>&);

private:
    template<typename Block, typename Instruction> static void computeImpl(Block* codeBlock, Instruction* instructionsBegin, unsigned instructionCount, Vector<std::unique_ptr<BytecodeBasicBlock>>& basicBlocks);

    void addSuccessor(BytecodeBasicBlock* block) { m_successors.append(block); }

    void addLength(unsigned);

    unsigned m_leaderOffset;
    unsigned m_totalLength;
    unsigned m_index;

    Vector<unsigned> m_offsets;
    Vector<BytecodeBasicBlock*> m_successors;

    FastBitVector m_in;
    FastBitVector m_out;
};

inline BytecodeBasicBlock::BytecodeBasicBlock(unsigned start, unsigned length)
    : m_leaderOffset(start)
    , m_totalLength(length)
{
    m_offsets.append(m_leaderOffset);
}

inline BytecodeBasicBlock::BytecodeBasicBlock(BytecodeBasicBlock::SpecialBlockType blockType)
    : m_leaderOffset(blockType == BytecodeBasicBlock::EntryBlock ? 0 : UINT_MAX)
    , m_totalLength(blockType == BytecodeBasicBlock::EntryBlock ? 0 : UINT_MAX)
{
}

inline void BytecodeBasicBlock::addLength(unsigned bytecodeLength)
{
    m_offsets.append(m_leaderOffset + m_totalLength);
    m_totalLength += bytecodeLength;
}

} // namespace JSC
