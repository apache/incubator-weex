/*
 * Copyright (C) 2017 Apple Inc. All rights reserved.
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
#include "AirDisassembler.h"

#if ENABLE(B3_JIT)

#include "AirBasicBlock.h"
#include "AirCode.h"
#include "AirInst.h"
#include "B3Value.h"
#include "Disassembler.h"
#include "LinkBuffer.h"

namespace JSC { namespace B3 { namespace Air {

void Disassembler::startEntrypoint(CCallHelpers& jit)
{
    m_entrypointStart = jit.labelIgnoringWatchpoints();
}

void Disassembler::endEntrypoint(CCallHelpers& jit)
{
    m_entrypointEnd = jit.labelIgnoringWatchpoints();
}

void Disassembler::startLatePath(CCallHelpers& jit)
{
    m_latePathStart = jit.labelIgnoringWatchpoints();
}

void Disassembler::endLatePath(CCallHelpers& jit)
{
    m_latePathEnd = jit.labelIgnoringWatchpoints();
}

void Disassembler::startBlock(BasicBlock* block, CCallHelpers& jit)
{
    UNUSED_PARAM(jit);
    m_blocks.append(block);
}

void Disassembler::addInst(Inst* inst, CCallHelpers::Label start, CCallHelpers::Label end)
{
    auto addResult = m_instToRange.add(inst, std::make_pair(start, end));
    RELEASE_ASSERT(addResult.isNewEntry);
}

void Disassembler::dump(Code& code, PrintStream& out, LinkBuffer& linkBuffer, const char* airPrefix, const char* asmPrefix, std::function<void(Inst&)> doToEachInst)
{
    auto dumpAsmRange = [&] (CCallHelpers::Label startLabel, CCallHelpers::Label endLabel) {
        RELEASE_ASSERT(startLabel.isSet());
        RELEASE_ASSERT(endLabel.isSet());
        CodeLocationLabel start = linkBuffer.locationOf(startLabel);
        CodeLocationLabel end = linkBuffer.locationOf(endLabel);
        RELEASE_ASSERT(bitwise_cast<uintptr_t>(end.executableAddress()) >= bitwise_cast<uintptr_t>(start.executableAddress()));
        disassemble(start, bitwise_cast<uintptr_t>(end.executableAddress()) - bitwise_cast<uintptr_t>(start.executableAddress()), asmPrefix, out);
    };

    for (BasicBlock* block : m_blocks) {
        block->dumpHeader(out);
        if (code.isEntrypoint(block))
            dumpAsmRange(m_entrypointStart, m_entrypointEnd);

        for (Inst& inst : *block) {
            doToEachInst(inst);

            out.print(airPrefix);
            inst.dump(out);
            out.print("\n");

            auto iter = m_instToRange.find(&inst);
            if (iter == m_instToRange.end()) {
                RELEASE_ASSERT(&inst == &block->last());
                continue;
            }
            auto pair = iter->value;
            dumpAsmRange(pair.first, pair.second);
        }
        block->dumpFooter(out);
    }

    // FIXME: We could be better about various late paths. We can implement
    // this later if we find a strong use for it.
    out.print("# Late paths\n");
    dumpAsmRange(m_latePathStart, m_latePathEnd);
}

} } } // namespace JSC::B3::Air

#endif // ENABLE(B3_JIT)
