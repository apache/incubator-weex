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

#pragma once

#if ENABLE(FTL_JIT)

#include "CCallHelpers.h"
#include "FTLSlowPathCallKey.h"
#include "JITOperations.h"

namespace JSC { namespace FTL {

class State;

class SlowPathCall {
public:
    SlowPathCall() { }
    
    SlowPathCall(MacroAssembler::Call call, const SlowPathCallKey& key)
        : m_call(call)
        , m_key(key)
    {
    }
    
    MacroAssembler::Call call() const { return m_call; }
    SlowPathCallKey key() const { return m_key; }
    
private:
    MacroAssembler::Call m_call;
    SlowPathCallKey m_key;
};

// This will be an RAII thingy that will set up the necessary stack sizes and offsets and such.
class SlowPathCallContext {
public:
    SlowPathCallContext(RegisterSet usedRegisters, CCallHelpers&, unsigned numArgs, GPRReg returnRegister);
    ~SlowPathCallContext();

    // NOTE: The call that this returns is already going to be linked by the JIT using addLinkTask(),
    // so there is no need for you to link it yourself.
    SlowPathCall makeCall(void* callTarget);

private:
    SlowPathCallKey keyWithTarget(void* callTarget) const;
    
    RegisterSet m_argumentRegisters;
    RegisterSet m_callingConventionRegisters;
    CCallHelpers& m_jit;
    unsigned m_numArgs;
    GPRReg m_returnRegister;
    size_t m_offsetToSavingArea;
    size_t m_stackBytesNeeded;
    RegisterSet m_thunkSaveSet;
    ptrdiff_t m_offset;
};

template<typename... ArgumentTypes>
SlowPathCall callOperation(
    const RegisterSet& usedRegisters, CCallHelpers& jit, CCallHelpers::JumpList* exceptionTarget,
    FunctionPtr function, GPRReg resultGPR, ArgumentTypes... arguments)
{
    SlowPathCall call;
    {
        SlowPathCallContext context(usedRegisters, jit, sizeof...(ArgumentTypes) + 1, resultGPR);
        jit.setupArgumentsWithExecState(arguments...);
        call = context.makeCall(function.value());
    }
    if (exceptionTarget)
        exceptionTarget->append(jit.emitExceptionCheck());
    return call;
}

template<typename... ArgumentTypes>
SlowPathCall callOperation(
    const RegisterSet& usedRegisters, CCallHelpers& jit, CallSiteIndex callSiteIndex,
    CCallHelpers::JumpList* exceptionTarget, FunctionPtr function, GPRReg resultGPR,
    ArgumentTypes... arguments)
{
    if (callSiteIndex) {
        jit.store32(
            CCallHelpers::TrustedImm32(callSiteIndex.bits()),
            CCallHelpers::tagFor(CallFrameSlot::argumentCount));
    }
    return callOperation(usedRegisters, jit, exceptionTarget, function, resultGPR, arguments...);
}

CallSiteIndex callSiteIndexForCodeOrigin(State&, CodeOrigin);

template<typename... ArgumentTypes>
SlowPathCall callOperation(
    State& state, const RegisterSet& usedRegisters, CCallHelpers& jit, CodeOrigin codeOrigin,
    CCallHelpers::JumpList* exceptionTarget, FunctionPtr function, GPRReg result, ArgumentTypes... arguments)
{
    return callOperation(
        usedRegisters, jit, callSiteIndexForCodeOrigin(state, codeOrigin), exceptionTarget, function,
        result, arguments...);
}

} } // namespace JSC::FTL

#endif // ENABLE(FTL_JIT)
