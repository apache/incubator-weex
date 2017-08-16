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
#include "Disassembler.h"

#if USE(ARM_LLVM_DISASSEMBLER)

#include "MacroAssemblerCodeRef.h"
#include <llvm-c/Disassembler.h>
#include <llvm-c/Target.h>

namespace JSC {

bool tryToDisassemble(const MacroAssemblerCodePtr& codePtr, size_t size, const char* prefix, PrintStream& out)
{
    LLVMInitializeAllTargetInfos();
    LLVMInitializeAllTargetMCs();
    LLVMInitializeAllDisassemblers();

    const char* triple = "armv7-unknown-linux-gnueabihf";
    LLVMDisasmContextRef disassemblyContext = LLVMCreateDisasm(triple, 0, 0, 0, 0);

    RELEASE_ASSERT(disassemblyContext);

    char pcString[20];
    char instructionString[100];

    uint8_t* pc = static_cast<uint8_t*>(codePtr.executableAddress());
    uint8_t* end = pc + size;

    while (pc < end) {
        snprintf(
            pcString, sizeof(pcString), "0x%lx",
            static_cast<unsigned long>(bitwise_cast<uintptr_t>(pc)));

        size_t instructionSize = LLVMDisasmInstruction(
            disassemblyContext, pc, end - pc, bitwise_cast<uintptr_t>(pc),
            instructionString, sizeof(instructionString));

        if (!instructionSize)
            snprintf(instructionString, sizeof(instructionString), "unknown instruction");

        out.printf("%s%16s: [0x%08lx] %s\n", prefix, pcString, *(reinterpret_cast<unsigned long*>(pc)), instructionString);
        pc += 4;
    }

    LLVMDisasmDispose(disassemblyContext);
    return true;
}

} // namespace JSC

#endif // USE(ARM_LLVM_DISASSEMBLER)
