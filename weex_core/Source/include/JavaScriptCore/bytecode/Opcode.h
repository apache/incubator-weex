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

#include "Bytecodes.h"
#include "LLIntOpcode.h"

#include <algorithm>
#include <string.h>

#include <wtf/Assertions.h>

namespace JSC {

#define FOR_EACH_CORE_OPCODE_ID_WITH_EXTENSION(macro, extension__) \
    FOR_EACH_BYTECODE_ID(macro) \
    extension__

#define FOR_EACH_CORE_OPCODE_ID(macro) \
    FOR_EACH_CORE_OPCODE_ID_WITH_EXTENSION(macro, /* No extension */ )

#define FOR_EACH_OPCODE_ID(macro) \
    FOR_EACH_CORE_OPCODE_ID_WITH_EXTENSION( \
        macro, \
        FOR_EACH_LLINT_OPCODE_EXTENSION(macro) \
    )


#define OPCODE_ID_ENUM(opcode, length) opcode,
    enum OpcodeID : unsigned { FOR_EACH_OPCODE_ID(OPCODE_ID_ENUM) };
#undef OPCODE_ID_ENUM

const int maxOpcodeLength = 9;
#if !ENABLE(JIT)
const int numOpcodeIDs = NUMBER_OF_BYTECODE_IDS + NUMBER_OF_CLOOP_BYTECODE_HELPER_IDS + NUMBER_OF_BYTECODE_HELPER_IDS;
#else
const int numOpcodeIDs = NUMBER_OF_BYTECODE_IDS + NUMBER_OF_BYTECODE_HELPER_IDS;
#endif

#define OPCODE_ID_LENGTHS(id, length) const int id##_length = length;
    FOR_EACH_OPCODE_ID(OPCODE_ID_LENGTHS);
#undef OPCODE_ID_LENGTHS

#define OPCODE_LENGTH(opcode) opcode##_length

#define OPCODE_ID_LENGTH_MAP(opcode, length) length,
    const int opcodeLengths[numOpcodeIDs] = { FOR_EACH_OPCODE_ID(OPCODE_ID_LENGTH_MAP) };
#undef OPCODE_ID_LENGTH_MAP

#if COMPILER(GCC)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtype-limits"
#endif

#define VERIFY_OPCODE_ID(id, size) COMPILE_ASSERT(id <= numOpcodeIDs, ASSERT_THAT_JS_OPCODE_IDS_ARE_VALID);
    FOR_EACH_OPCODE_ID(VERIFY_OPCODE_ID);
#undef VERIFY_OPCODE_ID

#if COMPILER(GCC)
#pragma GCC diagnostic pop
#endif

#if ENABLE(COMPUTED_GOTO_OPCODES)
typedef void* Opcode;
#else
typedef OpcodeID Opcode;
#endif

#define PADDING_STRING "                                "
#define PADDING_STRING_LENGTH static_cast<unsigned>(strlen(PADDING_STRING))

extern const char* const opcodeNames[];

inline const char* padOpcodeName(OpcodeID op, unsigned width)
{
    unsigned pad = width - strlen(opcodeNames[op]);
    pad = std::min(pad, PADDING_STRING_LENGTH);
    return PADDING_STRING + PADDING_STRING_LENGTH - pad;
}

#undef PADDING_STRING_LENGTH
#undef PADDING_STRING

#if ENABLE(OPCODE_STATS)

struct OpcodeStats {
    OpcodeStats();
    ~OpcodeStats();
    static long long opcodeCounts[numOpcodeIDs];
    static long long opcodePairCounts[numOpcodeIDs][numOpcodeIDs];
    static int lastOpcode;
    
    static void recordInstruction(int opcode);
    static void resetLastInstruction();
};

#endif

inline size_t opcodeLength(OpcodeID opcode)
{
    switch (opcode) {
#define OPCODE_ID_LENGTHS(id, length) case id: return OPCODE_LENGTH(id);
         FOR_EACH_OPCODE_ID(OPCODE_ID_LENGTHS)
#undef OPCODE_ID_LENGTHS
    }
    RELEASE_ASSERT_NOT_REACHED();
    return 0;
}

inline bool isBranch(OpcodeID opcodeID)
{
    switch (opcodeID) {
    case op_jmp:
    case op_jtrue:
    case op_jfalse:
    case op_jeq_null:
    case op_jneq_null:
    case op_jneq_ptr:
    case op_jless:
    case op_jlesseq:
    case op_jgreater:
    case op_jgreatereq:
    case op_jnless:
    case op_jnlesseq:
    case op_jngreater:
    case op_jngreatereq:
    case op_switch_imm:
    case op_switch_char:
    case op_switch_string:
        return true;
    default:
        return false;
    }
}

inline bool isUnconditionalBranch(OpcodeID opcodeID)
{
    switch (opcodeID) {
    case op_jmp:
        return true;
    default:
        return false;
    }
}

inline bool isTerminal(OpcodeID opcodeID)
{
    switch (opcodeID) {
    case op_ret:
    case op_end:
        return true;
    default:
        return false;
    }
}

inline bool isThrow(OpcodeID opcodeID)
{
    switch (opcodeID) {
    case op_throw:
    case op_throw_static_error:
        return true;
    default:
        return false;
    }
}

} // namespace JSC

namespace WTF {

class PrintStream;

void printInternal(PrintStream&, JSC::OpcodeID);

} // namespace WTF
