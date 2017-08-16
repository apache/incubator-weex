/*
 * Copyright (C) 2015-2016 Apple Inc. All rights reserved.
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
#include "AirHandleCalleeSaves.h"

#if ENABLE(B3_JIT)

#include "AirCode.h"
#include "AirInstInlines.h"
#include "AirPhaseScope.h"

namespace JSC { namespace B3 { namespace Air {

void handleCalleeSaves(Code& code)
{
    PhaseScope phaseScope(code, "handleCalleeSaves");

    RegisterSet usedCalleeSaves;

    for (BasicBlock* block : code) {
        for (Inst& inst : *block) {
            inst.forEachTmpFast(
                [&] (Tmp& tmp) {
                    // At first we just record all used regs.
                    usedCalleeSaves.set(tmp.reg());
                });

            if (inst.kind.opcode == Patch)
                usedCalleeSaves.merge(inst.extraClobberedRegs());
        }
    }

    // Now we filter to really get the callee saves.
    usedCalleeSaves.filter(RegisterSet::calleeSaveRegisters());
    usedCalleeSaves.filter(code.mutableRegs());
    usedCalleeSaves.exclude(RegisterSet::stackRegisters()); // We don't need to save FP here.

    if (!usedCalleeSaves.numberOfSetRegisters())
        return;

    code.calleeSaveRegisters() = RegisterAtOffsetList(usedCalleeSaves);

    size_t byteSize = 0;
    for (const RegisterAtOffset& entry : code.calleeSaveRegisters())
        byteSize = std::max(static_cast<size_t>(-entry.offset()), byteSize);

    StackSlot* savesArea = code.addStackSlot(byteSize, StackSlotKind::Locked);
    // This is a bit weird since we could have already pinned a different stack slot to this
    // area. Also, our runtime does not require us to pin the saves area. Maybe we shouldn't pin it?
    savesArea->setOffsetFromFP(-byteSize);
}

} } } // namespace JSC::B3::Air

#endif // ENABLE(B3_JIT)
