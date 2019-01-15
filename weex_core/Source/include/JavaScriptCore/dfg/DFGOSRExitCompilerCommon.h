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

#if ENABLE(DFG_JIT)

#include "CCallHelpers.h"
#include "DFGOSRExit.h"
#include "DFGCommonData.h"
#include "DFGJITCode.h"
#include "FTLJITCode.h"
#include "RegisterSet.h"

namespace JSC { namespace DFG {

void handleExitCounts(CCallHelpers&, const OSRExitBase&);
void reifyInlinedCallFrames(CCallHelpers&, const OSRExitBase&);
void adjustAndJumpToTarget(CCallHelpers&, const OSRExitBase&);

template <typename JITCodeType>
void adjustFrameAndStackInOSRExitCompilerThunk(MacroAssembler& jit, VM* vm, JITCode::JITType jitType)
{
    ASSERT(jitType == JITCode::DFGJIT || jitType == JITCode::FTLJIT);

    bool isFTLOSRExit = jitType == JITCode::FTLJIT;
    RegisterSet registersToPreserve;
    registersToPreserve.set(GPRInfo::regT0);
    if (isFTLOSRExit) {
        // FTL can use the scratch registers for values. The code below uses
        // the scratch registers. We need to preserve them before doing anything.
        registersToPreserve.merge(RegisterSet::macroScratchRegisters());
    }

    size_t scratchSize = sizeof(void*) * registersToPreserve.numberOfSetGPRs();
    if (isFTLOSRExit)
        scratchSize += sizeof(void*);

    ScratchBuffer* scratchBuffer = vm->scratchBufferForSize(scratchSize);
    char* buffer = static_cast<char*>(scratchBuffer->dataBuffer());

    jit.pushToSave(GPRInfo::regT1);
    jit.move(MacroAssembler::TrustedImmPtr(buffer), GPRInfo::regT1);

    unsigned storeOffset = 0;
    registersToPreserve.forEach([&](Reg reg) {
        jit.storePtr(reg.gpr(), MacroAssembler::Address(GPRInfo::regT1, storeOffset));
        storeOffset += sizeof(void*);
    });

    if (isFTLOSRExit) {
        // FTL OSRExits are entered via the code FTLExitThunkGenerator emits which does
        // pushToSaveImmediateWithoutTouchRegisters with the OSR exit index. We need to load
        // that top value and then push it back when we reset our SP.
        jit.loadPtr(MacroAssembler::Address(MacroAssembler::stackPointerRegister, MacroAssembler::pushToSaveByteOffset()), GPRInfo::regT0);
        jit.storePtr(GPRInfo::regT0, MacroAssembler::Address(GPRInfo::regT1, registersToPreserve.numberOfSetGPRs() * sizeof(void*)));
    }
    jit.popToRestore(GPRInfo::regT1);

    // We need to reset FP in the case of an exception.
    jit.loadPtr(vm->addressOfCallFrameForCatch(), GPRInfo::regT0);
    MacroAssembler::Jump didNotHaveException = jit.branchTestPtr(MacroAssembler::Zero, GPRInfo::regT0);
    jit.move(GPRInfo::regT0, GPRInfo::callFrameRegister);
    didNotHaveException.link(&jit);
    // We need to make sure SP is correct in case of an exception.
    jit.loadPtr(MacroAssembler::Address(GPRInfo::callFrameRegister, CallFrameSlot::codeBlock * static_cast<int>(sizeof(Register))), GPRInfo::regT0);
    jit.loadPtr(MacroAssembler::Address(GPRInfo::regT0, CodeBlock::jitCodeOffset()), GPRInfo::regT0);
    jit.addPtr(MacroAssembler::TrustedImm32(JITCodeType::commonDataOffset()), GPRInfo::regT0);
    jit.load32(MacroAssembler::Address(GPRInfo::regT0, CommonData::frameRegisterCountOffset()), GPRInfo::regT0);
    // This does virtualRegisterForLocal(frameRegisterCount - 1)*sizeof(Register) where:
    // virtualRegisterForLocal(frameRegisterCount - 1)
    //     = VirtualRegister::localToOperand(frameRegisterCount - 1)
    //     = -1 - (frameRegisterCount - 1)
    //     = -frameRegisterCount
    jit.neg32(GPRInfo::regT0);
    jit.mul32(MacroAssembler::TrustedImm32(sizeof(Register)), GPRInfo::regT0, GPRInfo::regT0);
#if USE(JSVALUE64)
    jit.signExtend32ToPtr(GPRInfo::regT0, GPRInfo::regT0);
#endif
    jit.addPtr(GPRInfo::callFrameRegister, GPRInfo::regT0);
    jit.move(GPRInfo::regT0, MacroAssembler::stackPointerRegister);

    if (isFTLOSRExit) {
        // Leave space for saving the OSR Exit Index.
        jit.subPtr(MacroAssembler::TrustedImm32(MacroAssembler::pushToSaveByteOffset()), MacroAssembler::stackPointerRegister);
    }
    jit.pushToSave(GPRInfo::regT1);

    jit.move(MacroAssembler::TrustedImmPtr(buffer), GPRInfo::regT1);
    if (isFTLOSRExit) {
        // FTL OSRExits are entered via FTLExitThunkGenerator code with does
        // pushToSaveImmediateWithoutTouchRegisters. We need to load that top
        // register and then store it back when we have our SP back to a safe value.
        jit.loadPtr(MacroAssembler::Address(GPRInfo::regT1, registersToPreserve.numberOfSetGPRs() * sizeof(void*)), GPRInfo::regT0);
        jit.storePtr(GPRInfo::regT0, MacroAssembler::Address(MacroAssembler::stackPointerRegister, MacroAssembler::pushToSaveByteOffset()));
    }

    unsigned loadOffset = 0;
    registersToPreserve.forEach([&](Reg reg) {
        jit.loadPtr(MacroAssembler::Address(GPRInfo::regT1, loadOffset), reg.gpr());
        loadOffset += sizeof(void*);
    });
    jit.popToRestore(GPRInfo::regT1);
}


} } // namespace JSC::DFG

#endif // ENABLE(DFG_JIT)
