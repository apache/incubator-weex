/*
 * Copyright (C) 2016 Apple Inc. All rights reserved.
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

#include "InterpreterInlines.h"
#include "Opcode.h"
#include "PreciseJumpTargets.h"

namespace JSC {

template<typename Block, typename Instruction, typename Function>
inline void extractStoredJumpTargetsForBytecodeOffset(Block* codeBlock, Interpreter* interpreter, Instruction* instructionsBegin, unsigned bytecodeOffset, Function function)
{
    OpcodeID opcodeID = interpreter->getOpcodeID(instructionsBegin[bytecodeOffset]);
    Instruction* current = instructionsBegin + bytecodeOffset;
    switch (opcodeID) {
    case op_jmp:
        function(current[1].u.operand);
        break;
    case op_jtrue:
    case op_jfalse:
    case op_jeq_null:
    case op_jneq_null:
        function(current[2].u.operand);
        break;
    case op_jneq_ptr:
    case op_jless:
    case op_jlesseq:
    case op_jgreater:
    case op_jgreatereq:
    case op_jnless:
    case op_jnlesseq:
    case op_jngreater:
    case op_jngreatereq:
        function(current[3].u.operand);
        break;
    case op_switch_imm:
    case op_switch_char: {
        auto& table = codeBlock->switchJumpTable(current[1].u.operand);
        for (unsigned i = table.branchOffsets.size(); i--;)
            function(table.branchOffsets[i]);
        function(current[2].u.operand);
        break;
    }
    case op_switch_string: {
        auto& table = codeBlock->stringSwitchJumpTable(current[1].u.operand);
        auto iter = table.offsetTable.begin();
        auto end = table.offsetTable.end();
        for (; iter != end; ++iter)
            function(iter->value.branchOffset);
        function(current[2].u.operand);
        break;
    }
    default:
        break;
    }
}

} // namespace JSC
