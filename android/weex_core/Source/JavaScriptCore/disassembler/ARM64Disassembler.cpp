/*
 * Copyright (C) 2012, 2014 Apple Inc. All rights reserved.
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

#if USE(ARM64_DISASSEMBLER)

#include "ARM64/A64DOpcode.h"
#include "MacroAssemblerCodeRef.h"

namespace JSC {

bool tryToDisassemble(const MacroAssemblerCodePtr& codePtr, size_t size, const char* prefix, PrintStream& out)
{
    A64DOpcode arm64Opcode;

    uint32_t* currentPC = reinterpret_cast<uint32_t*>(codePtr.executableAddress());
    size_t byteCount = size;

    while (byteCount) {
        char pcString[20];
        snprintf(pcString, sizeof(pcString), "0x%lx", reinterpret_cast<unsigned long>(currentPC));
        out.printf("%s%16s: %s\n", prefix, pcString, arm64Opcode.disassemble(currentPC));
        currentPC++;
        byteCount -= sizeof(uint32_t);
    }

    return true;
}

} // namespace JSC

#endif // USE(ARM64_DISASSEMBLER)

