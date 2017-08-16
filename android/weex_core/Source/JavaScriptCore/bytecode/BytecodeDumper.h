/*
 * Copyright (C) 2017 Yusuke Suzuki <utatane.tea@gmail.com>
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
