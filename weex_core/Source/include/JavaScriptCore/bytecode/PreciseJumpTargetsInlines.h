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
