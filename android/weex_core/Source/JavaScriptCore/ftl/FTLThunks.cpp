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
#include "FTLThunks.h"

#if ENABLE(FTL_JIT)

#include "AssemblyHelpers.h"
#include "DFGOSRExitCompilerCommon.h"
#include "FPRInfo.h"
#include "FTLOSRExitCompiler.h"
#include "FTLOperations.h"
#include "FTLSaveRestore.h"
#include "GPRInfo.h"
#include "LinkBuffer.h"

namespace JSC { namespace FTL {

using namespace DFG;

enum class FrameAndStackAdjustmentRequirement {
    Needed, 
    NotNeeded 
};

static MacroAssemblerCodeRef genericGenerationThunkGenerator(
    VM* vm, FunctionPtr generationFunction, const char* name, unsigned extraPopsToRestore, FrameAndStackAdjustmentRequirement frameAndStackAdjustmentRequirement)
{
    AssemblyHelpers jit(vm, 0);

    if (frameAndStackAdjustmentRequirement == FrameAndStackAdjustmentRequirement::Needed) {
        // This needs to happen before we use the scratch buffer because this function also uses the scratch buffer.
        adjustFrameAndStackInOSRExitCompilerThunk<FTL::JITCode>(jit, vm, JITCode::FTLJIT);
    }
    
    // Note that the "return address" will be the ID that we pass to the generation function.
    
    ptrdiff_t stackMisalignment = MacroAssembler::pushToSaveByteOffset();
    
    // Pretend that we're a C call frame.
    jit.pushToSave(MacroAssembler::framePointerRegister);
    jit.move(MacroAssembler::stackPointerRegister, MacroAssembler::framePointerRegister);
    stackMisalignment += MacroAssembler::pushToSaveByteOffset();
    
    // Now create ourselves enough stack space to give saveAllRegisters() a scratch slot.
    unsigned numberOfRequiredPops = 0;
    do {
        jit.pushToSave(GPRInfo::regT0);
        stackMisalignment += MacroAssembler::pushToSaveByteOffset();
        numberOfRequiredPops++;
    } while (stackMisalignment % stackAlignmentBytes());
    
    ScratchBuffer* scratchBuffer = vm->scratchBufferForSize(requiredScratchMemorySizeInBytes());
    char* buffer = static_cast<char*>(scratchBuffer->dataBuffer());
    
    saveAllRegisters(jit, buffer);
    
    // Tell GC mark phase how much of the scratch buffer is active during call.
    jit.move(MacroAssembler::TrustedImmPtr(scratchBuffer->activeLengthPtr()), GPRInfo::nonArgGPR0);
    jit.storePtr(MacroAssembler::TrustedImmPtr(requiredScratchMemorySizeInBytes()), GPRInfo::nonArgGPR0);

    jit.loadPtr(GPRInfo::callFrameRegister, GPRInfo::argumentGPR0);
    jit.peek(
        GPRInfo::argumentGPR1,
        (stackMisalignment - MacroAssembler::pushToSaveByteOffset()) / sizeof(void*));
    MacroAssembler::Call functionCall = jit.call();
    
    // At this point we want to make a tail call to what was returned to us in the
    // returnValueGPR. But at the same time as we do this, we must restore all registers.
    // The way we will accomplish this is by arranging to have the tail call target in the
    // return address "slot" (be it a register or the stack).
    
    jit.move(GPRInfo::returnValueGPR, GPRInfo::regT0);
    
    // Make sure we tell the GC that we're not using the scratch buffer anymore.
    jit.move(MacroAssembler::TrustedImmPtr(scratchBuffer->activeLengthPtr()), GPRInfo::regT1);
    jit.storePtr(MacroAssembler::TrustedImmPtr(0), GPRInfo::regT1);
    
    // Prepare for tail call.
    while (numberOfRequiredPops--)
        jit.popToRestore(GPRInfo::regT1);
    jit.popToRestore(MacroAssembler::framePointerRegister);

    // When we came in here, there was an additional thing pushed to the stack. Some clients want it
    // popped before proceeding.
    while (extraPopsToRestore--)
        jit.popToRestore(GPRInfo::regT1);

    // Put the return address wherever the return instruction wants it. On all platforms, this
    // ensures that the return address is out of the way of register restoration.
    jit.restoreReturnAddressBeforeReturn(GPRInfo::regT0);

    restoreAllRegisters(jit, buffer);

    jit.ret();
    
    LinkBuffer patchBuffer(*vm, jit, GLOBAL_THUNK_ID);
    patchBuffer.link(functionCall, generationFunction);
    return FINALIZE_CODE(patchBuffer, ("%s", name));
}

MacroAssemblerCodeRef osrExitGenerationThunkGenerator(VM* vm)
{
    unsigned extraPopsToRestore = 0;
    return genericGenerationThunkGenerator(
        vm, compileFTLOSRExit, "FTL OSR exit generation thunk", extraPopsToRestore, FrameAndStackAdjustmentRequirement::Needed);
}

MacroAssemblerCodeRef lazySlowPathGenerationThunkGenerator(VM* vm)
{
    unsigned extraPopsToRestore = 1;
    return genericGenerationThunkGenerator(
        vm, compileFTLLazySlowPath, "FTL lazy slow path generation thunk", extraPopsToRestore, FrameAndStackAdjustmentRequirement::NotNeeded);
}

static void registerClobberCheck(AssemblyHelpers& jit, RegisterSet dontClobber)
{
    if (!Options::clobberAllRegsInFTLICSlowPath())
        return;
    
    RegisterSet clobber = RegisterSet::allRegisters();
    clobber.exclude(RegisterSet::reservedHardwareRegisters());
    clobber.exclude(RegisterSet::stackRegisters());
    clobber.exclude(RegisterSet::calleeSaveRegisters());
    clobber.exclude(dontClobber);
    
    GPRReg someGPR;
    for (Reg reg = Reg::first(); reg <= Reg::last(); reg = reg.next()) {
        if (!clobber.get(reg) || !reg.isGPR())
            continue;
        
        jit.move(AssemblyHelpers::TrustedImm32(0x1337beef), reg.gpr());
        someGPR = reg.gpr();
    }
    
    for (Reg reg = Reg::first(); reg <= Reg::last(); reg = reg.next()) {
        if (!clobber.get(reg) || !reg.isFPR())
            continue;
        
        jit.move64ToDouble(someGPR, reg.fpr());
    }
}

MacroAssemblerCodeRef slowPathCallThunkGenerator(VM& vm, const SlowPathCallKey& key)
{
    AssemblyHelpers jit(&vm, 0);
    
    // We want to save the given registers at the given offset, then we want to save the
    // old return address somewhere past that offset, and then finally we want to make the
    // call.
    
    size_t currentOffset = key.offset() + sizeof(void*);
    
#if CPU(X86) || CPU(X86_64)
    currentOffset += sizeof(void*);
#endif
    
    for (MacroAssembler::RegisterID reg = MacroAssembler::firstRegister(); reg <= MacroAssembler::lastRegister(); reg = static_cast<MacroAssembler::RegisterID>(reg + 1)) {
        if (!key.usedRegisters().get(reg))
            continue;
        jit.storePtr(reg, AssemblyHelpers::Address(MacroAssembler::stackPointerRegister, currentOffset));
        currentOffset += sizeof(void*);
    }
    
    for (MacroAssembler::FPRegisterID reg = MacroAssembler::firstFPRegister(); reg <= MacroAssembler::lastFPRegister(); reg = static_cast<MacroAssembler::FPRegisterID>(reg + 1)) {
        if (!key.usedRegisters().get(reg))
            continue;
        jit.storeDouble(reg, AssemblyHelpers::Address(MacroAssembler::stackPointerRegister, currentOffset));
        currentOffset += sizeof(double);
    }
    
    jit.preserveReturnAddressAfterCall(GPRInfo::nonArgGPR0);
    jit.storePtr(GPRInfo::nonArgGPR0, AssemblyHelpers::Address(MacroAssembler::stackPointerRegister, key.offset()));
    
    registerClobberCheck(jit, key.argumentRegisters());
    
    AssemblyHelpers::Call call = jit.call();

    jit.loadPtr(AssemblyHelpers::Address(MacroAssembler::stackPointerRegister, key.offset()), GPRInfo::nonPreservedNonReturnGPR);
    jit.restoreReturnAddressBeforeReturn(GPRInfo::nonPreservedNonReturnGPR);
    
    for (MacroAssembler::FPRegisterID reg = MacroAssembler::lastFPRegister(); ; reg = static_cast<MacroAssembler::FPRegisterID>(reg - 1)) {
        if (key.usedRegisters().get(reg)) {
            currentOffset -= sizeof(double);
            jit.loadDouble(AssemblyHelpers::Address(MacroAssembler::stackPointerRegister, currentOffset), reg);
        }
        if (reg == MacroAssembler::firstFPRegister())
            break;
    }
    
    for (MacroAssembler::RegisterID reg = MacroAssembler::lastRegister(); ; reg = static_cast<MacroAssembler::RegisterID>(reg - 1)) {
        if (key.usedRegisters().get(reg)) {
            currentOffset -= sizeof(void*);
            jit.loadPtr(AssemblyHelpers::Address(MacroAssembler::stackPointerRegister, currentOffset), reg);
        }
        if (reg == MacroAssembler::firstRegister())
            break;
    }
    
    jit.ret();

    LinkBuffer patchBuffer(vm, jit, GLOBAL_THUNK_ID);
    patchBuffer.link(call, FunctionPtr(key.callTarget()));
    return FINALIZE_CODE(patchBuffer, ("FTL slow path call thunk for %s", toCString(key).data()));
}

} } // namespace JSC::FTL

#endif // ENABLE(FTL_JIT)

