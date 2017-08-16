/*
 * Copyright (C) 2013-2015 Apple Inc. All rights reserved.
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
#include "FTLSlowPathCall.h"

#if ENABLE(FTL_JIT)

#include "CCallHelpers.h"
#include "FTLState.h"
#include "FTLThunks.h"
#include "GPRInfo.h"
#include "JSCInlines.h"

namespace JSC { namespace FTL {

// This code relies on us being 64-bit. FTL is currently always 64-bit.
static const size_t wordSize = 8;

SlowPathCallContext::SlowPathCallContext(
    RegisterSet usedRegisters, CCallHelpers& jit, unsigned numArgs, GPRReg returnRegister)
    : m_jit(jit)
    , m_numArgs(numArgs)
    , m_returnRegister(returnRegister)
{
    // We don't care that you're using callee-save, stack, or hardware registers.
    usedRegisters.exclude(RegisterSet::stackRegisters());
    usedRegisters.exclude(RegisterSet::reservedHardwareRegisters());
    usedRegisters.exclude(RegisterSet::calleeSaveRegisters());
        
    // The return register doesn't need to be saved.
    if (m_returnRegister != InvalidGPRReg)
        usedRegisters.clear(m_returnRegister);
        
    size_t stackBytesNeededForReturnAddress = wordSize;
        
    m_offsetToSavingArea =
        (std::max(m_numArgs, NUMBER_OF_ARGUMENT_REGISTERS) - NUMBER_OF_ARGUMENT_REGISTERS) * wordSize;
        
    for (unsigned i = std::min(NUMBER_OF_ARGUMENT_REGISTERS, numArgs); i--;)
        m_argumentRegisters.set(GPRInfo::toArgumentRegister(i));
    m_callingConventionRegisters.merge(m_argumentRegisters);
    if (returnRegister != InvalidGPRReg)
        m_callingConventionRegisters.set(GPRInfo::returnValueGPR);
    m_callingConventionRegisters.filter(usedRegisters);
        
    unsigned numberOfCallingConventionRegisters =
        m_callingConventionRegisters.numberOfSetRegisters();
        
    size_t offsetToThunkSavingArea =
        m_offsetToSavingArea +
        numberOfCallingConventionRegisters * wordSize;
        
    m_stackBytesNeeded =
        offsetToThunkSavingArea +
        stackBytesNeededForReturnAddress +
        (usedRegisters.numberOfSetRegisters() - numberOfCallingConventionRegisters) * wordSize;
        
    m_stackBytesNeeded = (m_stackBytesNeeded + stackAlignmentBytes() - 1) & ~(stackAlignmentBytes() - 1);
        
    m_jit.subPtr(CCallHelpers::TrustedImm32(m_stackBytesNeeded), CCallHelpers::stackPointerRegister);

    m_thunkSaveSet = usedRegisters;
        
    // This relies on all calling convention registers also being temp registers.
    unsigned stackIndex = 0;
    for (unsigned i = GPRInfo::numberOfRegisters; i--;) {
        GPRReg reg = GPRInfo::toRegister(i);
        if (!m_callingConventionRegisters.get(reg))
            continue;
        m_jit.storePtr(reg, CCallHelpers::Address(CCallHelpers::stackPointerRegister, m_offsetToSavingArea + (stackIndex++) * wordSize));
        m_thunkSaveSet.clear(reg);
    }
        
    m_offset = offsetToThunkSavingArea;
}
    
SlowPathCallContext::~SlowPathCallContext()
{
    if (m_returnRegister != InvalidGPRReg)
        m_jit.move(GPRInfo::returnValueGPR, m_returnRegister);
    
    unsigned stackIndex = 0;
    for (unsigned i = GPRInfo::numberOfRegisters; i--;) {
        GPRReg reg = GPRInfo::toRegister(i);
        if (!m_callingConventionRegisters.get(reg))
            continue;
        m_jit.loadPtr(CCallHelpers::Address(CCallHelpers::stackPointerRegister, m_offsetToSavingArea + (stackIndex++) * wordSize), reg);
    }
    
    m_jit.addPtr(CCallHelpers::TrustedImm32(m_stackBytesNeeded), CCallHelpers::stackPointerRegister);
}

SlowPathCallKey SlowPathCallContext::keyWithTarget(void* callTarget) const
{
    return SlowPathCallKey(m_thunkSaveSet, callTarget, m_argumentRegisters, m_offset);
}

SlowPathCall SlowPathCallContext::makeCall(void* callTarget)
{
    SlowPathCall result = SlowPathCall(m_jit.call(), keyWithTarget(callTarget));

    m_jit.addLinkTask(
        [result] (LinkBuffer& linkBuffer) {
            VM& vm = linkBuffer.vm();

            MacroAssemblerCodeRef thunk =
                vm.ftlThunks->getSlowPathCallThunk(vm, result.key());

            linkBuffer.link(result.call(), CodeLocationLabel(thunk.code()));
        });
    
    return result;
}

CallSiteIndex callSiteIndexForCodeOrigin(State& state, CodeOrigin codeOrigin)
{
    if (codeOrigin)
        return state.jitCode->common.addCodeOrigin(codeOrigin);
    return CallSiteIndex();
}

} } // namespace JSC::FTL

#endif // ENABLE(FTL_JIT)

