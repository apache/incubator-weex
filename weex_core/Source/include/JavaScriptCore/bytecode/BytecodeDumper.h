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

#include "CallLinkInfo.h"
#include "StructureStubInfo.h"
#include <wtf/StringPrintStream.h>

namespace JSC {

struct Instruction;

template<class Block>
class BytecodeDumper {
public:
    typedef typename Block::Instruction Instruction;

    static void dumpBytecode(Block*, PrintStream& out, const Instruction* begin, const Instruction*& it, const StubInfoMap& = StubInfoMap(), const CallLinkInfoMap& = CallLinkInfoMap());
    static void dumpBlock(Block*, const typename Block::UnpackedInstructions&, PrintStream& out, const StubInfoMap& = StubInfoMap(), const CallLinkInfoMap& = CallLinkInfoMap());

private:
    BytecodeDumper(Block* block, const Instruction* instructionsBegin)
        : m_block(block)
        , m_instructionsBegin(instructionsBegin)
    {
    }

    Block* block() const { return m_block; }
    const Instruction* instructionsBegin() const { return m_instructionsBegin; }

    ALWAYS_INLINE VM* vm() const;

    CString registerName(int r) const;
    CString constantName(int index) const;

    const Identifier& identifier(int index) const;

    void dumpIdentifiers(PrintStream& out);
    void dumpConstants(PrintStream& out);
    void dumpRegExps(PrintStream& out);
    void dumpExceptionHandlers(PrintStream& out);
    void dumpSwitchJumpTables(PrintStream& out);
    void dumpStringSwitchJumpTables(PrintStream& out);

    void printUnaryOp(PrintStream& out, int location, const Instruction*& it, const char* op);
    void printBinaryOp(PrintStream& out, int location, const Instruction*& it, const char* op);
    void printConditionalJump(PrintStream& out, const Instruction*, const Instruction*& it, int location, const char* op);
    void printGetByIdOp(PrintStream& out, int location, const Instruction*& it);
    void printGetByIdCacheStatus(PrintStream& out, int location, const StubInfoMap&);
    void printPutByIdCacheStatus(PrintStream& out, int location, const StubInfoMap&);
    enum CacheDumpMode { DumpCaches, DontDumpCaches };
    void printCallOp(PrintStream& out, int location, const Instruction*& it, const char* op, CacheDumpMode, bool& hasPrintedProfiling, const CallLinkInfoMap&);
    void printPutByIdOp(PrintStream& out, int location, const Instruction*& it, const char* op);
    void printLocationOpAndRegisterOperand(PrintStream& out, int location, const Instruction*& it, const char* op, int operand);
    void dumpBytecode(PrintStream& out, const Instruction* begin, const Instruction*& it, const StubInfoMap&, const CallLinkInfoMap&);

    void dumpValueProfiling(PrintStream&, const Instruction*&, bool& hasPrintedProfiling);
    void dumpArrayProfiling(PrintStream&, const Instruction*&, bool& hasPrintedProfiling);
    void dumpProfilesForBytecodeOffset(PrintStream&, unsigned location, bool& hasPrintedProfiling);

    void* actualPointerFor(Special::Pointer) const;

#if ENABLE(JIT)
    void dumpCallLinkStatus(PrintStream&, unsigned location, const CallLinkInfoMap&);
#endif

    Block* m_block;
    const Instruction* m_instructionsBegin;
};

}
