/*
 * Copyright (C) 2013 Apple Inc. All rights reserved.
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
#include "PreciseJumpTargets.h"

#include "InterpreterInlines.h"
#include "JSCInlines.h"
#include "PreciseJumpTargetsInlines.h"

namespace JSC {

template <size_t vectorSize, typename Block, typename Instruction>
static void getJumpTargetsForBytecodeOffset(Block* codeBlock, Interpreter* interpreter, Instruction* instructionsBegin, unsigned bytecodeOffset, Vector<unsigned, vectorSize>& out)
{
    OpcodeID opcodeID = interpreter->getOpcodeID(instructionsBegin[bytecodeOffset]);
    extractStoredJumpTargetsForBytecodeOffset(codeBlock, interpreter, instructionsBegin, bytecodeOffset, [&](int32_t& relativeOffset) {
        out.append(bytecodeOffset + relativeOffset);
    });
    // op_loop_hint does not have jump target stored in bytecode instructions.
    if (opcodeID == op_loop_hint)
        out.append(bytecodeOffset);
}

enum class ComputePreciseJumpTargetsMode {
    FollowCodeBlockClaim,
    ForceCompute,
};

template<ComputePreciseJumpTargetsMode Mode, typename Block, typename Instruction, size_t vectorSize>
void computePreciseJumpTargetsInternal(Block* codeBlock, Instruction* instructionsBegin, unsigned instructionCount, Vector<unsigned, vectorSize>& out)
{
    ASSERT(out.isEmpty());
    
    // We will derive a superset of the jump targets that the code block thinks it has.
    // So, if the code block claims there are none, then we are done.
    if (Mode == ComputePreciseJumpTargetsMode::FollowCodeBlockClaim && !codeBlock->numberOfJumpTargets())
        return;
    
    for (unsigned i = codeBlock->numberOfExceptionHandlers(); i--;) {
        out.append(codeBlock->exceptionHandler(i).target);
        out.append(codeBlock->exceptionHandler(i).start);
        out.append(codeBlock->exceptionHandler(i).end);
    }

    Interpreter* interpreter = codeBlock->vm()->interpreter;
    for (unsigned bytecodeOffset = 0; bytecodeOffset < instructionCount;) {
        OpcodeID opcodeID = interpreter->getOpcodeID(instructionsBegin[bytecodeOffset]);
        getJumpTargetsForBytecodeOffset(codeBlock, interpreter, instructionsBegin, bytecodeOffset, out);
        bytecodeOffset += opcodeLengths[opcodeID];
    }
    
    std::sort(out.begin(), out.end());
    
    // We will have duplicates, and we must remove them.
    unsigned toIndex = 0;
    unsigned fromIndex = 0;
    unsigned lastValue = UINT_MAX;
    while (fromIndex < out.size()) {
        unsigned value = out[fromIndex++];
        if (value == lastValue)
            continue;
        out[toIndex++] = value;
        lastValue = value;
    }
    out.resize(toIndex);
    out.shrinkToFit();
}

void computePreciseJumpTargets(CodeBlock* codeBlock, Vector<unsigned, 32>& out)
{
    computePreciseJumpTargetsInternal<ComputePreciseJumpTargetsMode::FollowCodeBlockClaim>(codeBlock, codeBlock->instructions().begin(), codeBlock->instructions().size(), out);
}

void computePreciseJumpTargets(CodeBlock* codeBlock, Instruction* instructionsBegin, unsigned instructionCount, Vector<unsigned, 32>& out)
{
    computePreciseJumpTargetsInternal<ComputePreciseJumpTargetsMode::FollowCodeBlockClaim>(codeBlock, instructionsBegin, instructionCount, out);
}

void computePreciseJumpTargets(UnlinkedCodeBlock* codeBlock, UnlinkedInstruction* instructionsBegin, unsigned instructionCount, Vector<unsigned, 32>& out)
{
    computePreciseJumpTargetsInternal<ComputePreciseJumpTargetsMode::FollowCodeBlockClaim>(codeBlock, instructionsBegin, instructionCount, out);
}

void recomputePreciseJumpTargets(UnlinkedCodeBlock* codeBlock, UnlinkedInstruction* instructionsBegin, unsigned instructionCount, Vector<unsigned>& out)
{
    computePreciseJumpTargetsInternal<ComputePreciseJumpTargetsMode::ForceCompute>(codeBlock, instructionsBegin, instructionCount, out);
}

void findJumpTargetsForBytecodeOffset(CodeBlock* codeBlock, Instruction* instructionsBegin, unsigned bytecodeOffset, Vector<unsigned, 1>& out)
{
    getJumpTargetsForBytecodeOffset(codeBlock, codeBlock->vm()->interpreter, instructionsBegin, bytecodeOffset, out);
}

void findJumpTargetsForBytecodeOffset(UnlinkedCodeBlock* codeBlock, UnlinkedInstruction* instructionsBegin, unsigned bytecodeOffset, Vector<unsigned, 1>& out)
{
    getJumpTargetsForBytecodeOffset(codeBlock, codeBlock->vm()->interpreter, instructionsBegin, bytecodeOffset, out);
}

} // namespace JSC

