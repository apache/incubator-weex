/*
 * Copyright (C) 2013, 2015 Apple Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "BytecodeBasicBlock.h"

#include "CodeBlock.h"
#include "InterpreterInlines.h"
#include "JSCInlines.h"
#include "PreciseJumpTargets.h"

namespace JSC {

void BytecodeBasicBlock::shrinkToFit()
{
    m_offsets.shrinkToFit();
    m_successors.shrinkToFit();
}

static bool isJumpTarget(OpcodeID opcodeID, const Vector<unsigned, 32>& jumpTargets, unsigned bytecodeOffset)
{
    if (opcodeID == op_catch)
        return true;

    return std::binary_search(jumpTargets.begin(), jumpTargets.end(), bytecodeOffset);
}

template<typename Block, typename Instruction>
void BytecodeBasicBlock::computeImpl(Block* codeBlock, Instruction* instructionsBegin, unsigned instructionCount, Vector<std::unique_ptr<BytecodeBasicBlock>>& basicBlocks)
{
    Vector<unsigned, 32> jumpTargets;
    computePreciseJumpTargets(codeBlock, instructionsBegin, instructionCount, jumpTargets);

    auto appendBlock = [&] (std::unique_ptr<BytecodeBasicBlock>&& block) {
        block->m_index = basicBlocks.size();
        basicBlocks.append(WTFMove(block));
    };

    auto linkBlocks = [&] (BytecodeBasicBlock* from, BytecodeBasicBlock* to) {
        from->addSuccessor(to);
    };

    // Create the entry and exit basic blocks.
    basicBlocks.reserveCapacity(jumpTargets.size() + 2);

    auto entry = std::make_unique<BytecodeBasicBlock>(BytecodeBasicBlock::EntryBlock);
    auto firstBlock = std::make_unique<BytecodeBasicBlock>(0, 0);
    linkBlocks(entry.get(), firstBlock.get());

    appendBlock(WTFMove(entry));
    BytecodeBasicBlock* current = firstBlock.get();
    appendBlock(WTFMove(firstBlock));

    auto exit = std::make_unique<BytecodeBasicBlock>(BytecodeBasicBlock::ExitBlock);

    bool nextInstructionIsLeader = false;

    Interpreter* interpreter = codeBlock->vm()->interpreter;
    for (unsigned bytecodeOffset = 0; bytecodeOffset < instructionCount;) {
        OpcodeID opcodeID = interpreter->getOpcodeID(instructionsBegin[bytecodeOffset]);
        unsigned opcodeLength = opcodeLengths[opcodeID];

        bool createdBlock = false;
        // If the current bytecode is a jump target, then it's the leader of its own basic block.
        if (isJumpTarget(opcodeID, jumpTargets, bytecodeOffset) || nextInstructionIsLeader) {
            auto newBlock = std::make_unique<BytecodeBasicBlock>(bytecodeOffset, opcodeLength);
            current = newBlock.get();
            appendBlock(WTFMove(newBlock));
            createdBlock = true;
            nextInstructionIsLeader = false;
            bytecodeOffset += opcodeLength;
        }

        // If the current bytecode is a branch or a return, then the next instruction is the leader of its own basic block.
        if (isBranch(opcodeID) || isTerminal(opcodeID) || isThrow(opcodeID))
            nextInstructionIsLeader = true;

        if (createdBlock)
            continue;

        // Otherwise, just add to the length of the current block.
        current->addLength(opcodeLength);
        bytecodeOffset += opcodeLength;
    }

    // Link basic blocks together.
    for (unsigned i = 0; i < basicBlocks.size(); i++) {
        BytecodeBasicBlock* block = basicBlocks[i].get();

        if (block->isEntryBlock() || block->isExitBlock())
            continue;

        bool fallsThrough = true; 
        for (unsigned bytecodeOffset = block->leaderOffset(); bytecodeOffset < block->leaderOffset() + block->totalLength();) {
            OpcodeID opcodeID = interpreter->getOpcodeID(instructionsBegin[bytecodeOffset]);
            unsigned opcodeLength = opcodeLengths[opcodeID];
            // If we found a terminal bytecode, link to the exit block.
            if (isTerminal(opcodeID)) {
                ASSERT(bytecodeOffset + opcodeLength == block->leaderOffset() + block->totalLength());
                linkBlocks(block, exit.get());
                fallsThrough = false;
                break;
            }

            // If we found a throw, get the HandlerInfo for this instruction to see where we will jump. 
            // If there isn't one, treat this throw as a terminal. This is true even if we have a finally
            // block because the finally block will create its own catch, which will generate a HandlerInfo.
            if (isThrow(opcodeID)) {
                ASSERT(bytecodeOffset + opcodeLength == block->leaderOffset() + block->totalLength());
                auto* handler = codeBlock->handlerForBytecodeOffset(bytecodeOffset);
                fallsThrough = false;
                if (!handler) {
                    linkBlocks(block, exit.get());
                    break;
                }
                for (unsigned i = 0; i < basicBlocks.size(); i++) {
                    BytecodeBasicBlock* otherBlock = basicBlocks[i].get();
                    if (handler->target == otherBlock->leaderOffset()) {
                        linkBlocks(block, otherBlock);
                        break;
                    }
                }
                break;
            }

            // If we found a branch, link to the block(s) that we jump to.
            if (isBranch(opcodeID)) {
                ASSERT(bytecodeOffset + opcodeLength == block->leaderOffset() + block->totalLength());
                Vector<unsigned, 1> bytecodeOffsetsJumpedTo;
                findJumpTargetsForBytecodeOffset(codeBlock, instructionsBegin, bytecodeOffset, bytecodeOffsetsJumpedTo);

                size_t numberOfJumpTargets = bytecodeOffsetsJumpedTo.size();
                ASSERT(numberOfJumpTargets);
                for (unsigned i = 0; i < basicBlocks.size(); i++) {
                    BytecodeBasicBlock* otherBlock = basicBlocks[i].get();
                    if (bytecodeOffsetsJumpedTo.contains(otherBlock->leaderOffset())) {
                        linkBlocks(block, otherBlock);
                        --numberOfJumpTargets;
                        if (!numberOfJumpTargets)
                            break;
                    }
                }
                // numberOfJumpTargets may not be 0 here if there are multiple jumps targeting the same
                // basic blocks (e.g. in a switch type opcode). Since we only decrement numberOfJumpTargets
                // once per basic block, the duplicates are not accounted for. For our purpose here,
                // that doesn't matter because we only need to link to the target block once regardless
                // of how many ways this block can jump there.

                if (isUnconditionalBranch(opcodeID))
                    fallsThrough = false;

                break;
            }
            bytecodeOffset += opcodeLength;
        }

        // If we fall through then link to the next block in program order.
        if (fallsThrough) {
            ASSERT(i + 1 < basicBlocks.size());
            BytecodeBasicBlock* nextBlock = basicBlocks[i + 1].get();
            linkBlocks(block, nextBlock);
        }
    }

    appendBlock(WTFMove(exit));
    
    for (auto& basicBlock : basicBlocks)
        basicBlock->shrinkToFit();
}

void BytecodeBasicBlock::compute(CodeBlock* codeBlock, Instruction* instructionsBegin, unsigned instructionCount, Vector<std::unique_ptr<BytecodeBasicBlock>>& basicBlocks)
{
    computeImpl(codeBlock, instructionsBegin, instructionCount, basicBlocks);
}

void BytecodeBasicBlock::compute(UnlinkedCodeBlock* codeBlock, UnlinkedInstruction* instructionsBegin, unsigned instructionCount, Vector<std::unique_ptr<BytecodeBasicBlock>>& basicBlocks)
{
    BytecodeBasicBlock::computeImpl(codeBlock, instructionsBegin, instructionCount, basicBlocks);
}

} // namespace JSC
