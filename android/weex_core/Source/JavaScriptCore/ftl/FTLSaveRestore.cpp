/*
 * Copyright (C) 2013, 2014 Apple Inc. All rights reserved.
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
#include "FTLSaveRestore.h"

#if ENABLE(FTL_JIT)

#include "FPRInfo.h"
#include "GPRInfo.h"
#include "MacroAssembler.h"
#include "RegisterSet.h"

namespace JSC { namespace FTL {

static size_t bytesForGPRs()
{
    return MacroAssembler::numberOfRegisters() * sizeof(int64_t);
}

static size_t bytesForFPRs()
{
    // FIXME: It might be worthwhile saving the full state of the FP registers, at some point.
    // Right now we don't need this since we only do the save/restore just prior to OSR exit, and
    // OSR exit will be guaranteed to only need the double portion of the FP registers.
    return MacroAssembler::numberOfFPRegisters() * sizeof(double);
}

size_t requiredScratchMemorySizeInBytes()
{
    return bytesForGPRs() + bytesForFPRs();
}

size_t offsetOfGPR(GPRReg reg)
{
    return MacroAssembler::registerIndex(reg) * sizeof(int64_t);
}

size_t offsetOfFPR(FPRReg reg)
{
    return bytesForGPRs() + MacroAssembler::fpRegisterIndex(reg) * sizeof(double);
}

size_t offsetOfReg(Reg reg)
{
    if (reg.isGPR())
        return offsetOfGPR(reg.gpr());
    return offsetOfFPR(reg.fpr());
}

namespace {

struct Regs {
    Regs()
    {
        special = RegisterSet::stackRegisters();
        special.merge(RegisterSet::reservedHardwareRegisters());
        
        first = MacroAssembler::firstRegister();
        while (special.get(first))
            first = MacroAssembler::nextRegister(first);
        second = MacroAssembler::nextRegister(first);
        while (special.get(second))
            second = MacroAssembler::nextRegister(second);
    }
    
    RegisterSet special;
    GPRReg first;
    GPRReg second;
};

} // anonymous namespace

void saveAllRegisters(MacroAssembler& jit, char* scratchMemory)
{
    Regs regs;
    
    // Get the first register out of the way, so that we can use it as a pointer.
    jit.poke64(regs.first, 0);
    jit.move(MacroAssembler::TrustedImmPtr(scratchMemory), regs.first);
    
    // Get all of the other GPRs out of the way.
    for (MacroAssembler::RegisterID reg = regs.second; reg <= MacroAssembler::lastRegister(); reg = MacroAssembler::nextRegister(reg)) {
        if (regs.special.get(reg))
            continue;
        jit.store64(reg, MacroAssembler::Address(regs.first, offsetOfGPR(reg)));
    }
    
    // Restore the first register into the second one and save it.
    jit.peek64(regs.second, 0);
    jit.store64(regs.second, MacroAssembler::Address(regs.first, offsetOfGPR(regs.first)));
    
    // Finally save all FPR's.
    for (MacroAssembler::FPRegisterID reg = MacroAssembler::firstFPRegister(); reg <= MacroAssembler::lastFPRegister(); reg = MacroAssembler::nextFPRegister(reg)) {
        if (regs.special.get(reg))
            continue;
        jit.storeDouble(reg, MacroAssembler::Address(regs.first, offsetOfFPR(reg)));
    }
}

void restoreAllRegisters(MacroAssembler& jit, char* scratchMemory)
{
    Regs regs;
    
    // Give ourselves a pointer to the scratch memory.
    jit.move(MacroAssembler::TrustedImmPtr(scratchMemory), regs.first);
    
    // Restore all FPR's.
    for (MacroAssembler::FPRegisterID reg = MacroAssembler::firstFPRegister(); reg <= MacroAssembler::lastFPRegister(); reg = MacroAssembler::nextFPRegister(reg)) {
        if (regs.special.get(reg))
            continue;
        jit.loadDouble(MacroAssembler::Address(regs.first, offsetOfFPR(reg)), reg);
    }
    
    for (MacroAssembler::RegisterID reg = regs.second; reg <= MacroAssembler::lastRegister(); reg = MacroAssembler::nextRegister(reg)) {
        if (regs.special.get(reg))
            continue;
        jit.load64(MacroAssembler::Address(regs.first, offsetOfGPR(reg)), reg);
    }
    
    jit.load64(MacroAssembler::Address(regs.first, offsetOfGPR(regs.first)), regs.first);
}

} } // namespace JSC::FTL

#endif // ENABLE(FTL_JIT)

