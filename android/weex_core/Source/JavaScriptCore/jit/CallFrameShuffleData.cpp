/*
 * Copyright (C) 2015 Apple Inc. All rights reserved.
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
#include "CallFrameShuffleData.h"

#if ENABLE(JIT)

#include "CCallHelpers.h"
#include "CodeBlock.h"

namespace JSC {

#if USE(JSVALUE64)

void CallFrameShuffleData::setupCalleeSaveRegisters(CodeBlock* codeBlock)
{
    RegisterSet calleeSaveRegisters { RegisterSet::vmCalleeSaveRegisters() };
    RegisterAtOffsetList* registerSaveLocations = codeBlock->calleeSaveRegisters();

    for (size_t i = 0; i < registerSaveLocations->size(); ++i) {
        RegisterAtOffset entry { registerSaveLocations->at(i) };
        if (!calleeSaveRegisters.get(entry.reg()))
            continue;

        VirtualRegister saveSlot { entry.offsetAsIndex() };
        registers[entry.reg()]
            = ValueRecovery::displacedInJSStack(saveSlot, DataFormatJS);
    }

    for (Reg reg = Reg::first(); reg <= Reg::last(); reg = reg.next()) {
        if (!calleeSaveRegisters.get(reg))
            continue;

        if (registers[reg])
            continue;

        registers[reg] = ValueRecovery::inRegister(reg, DataFormatJS);
    }
}

#endif // USE(JSVALUE64)

} // namespace JSC

#endif // ENABLE(JIT)
