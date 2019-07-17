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

#pragma once

#include "CommonSlowPaths.h"
#include "MacroAssemblerCodeRef.h"

#if ENABLE(JIT)

namespace JSC {

class JITSlowPathCall {
public:
    JITSlowPathCall(JIT* jit, Instruction* pc, SlowPathReturnType (SLOW_PATH *stub)(ExecState* exec, Instruction* pc))
        : m_jit(jit)
        , m_stub(stub)
        , m_pc(pc)
    {
    }

    JIT::Call call()
    {
#if ENABLE(OPCODE_SAMPLING)
        if (m_jit->m_bytecodeOffset != std::numeric_limits<unsigned>::max())
            m_jit->sampleInstruction(m_jit->m_codeBlock->instructions().begin() + m_jit->m_bytecodeOffset, true);
#endif
        m_jit->updateTopCallFrame();
#if CPU(X86) && USE(JSVALUE32_64)
        m_jit->addPtr(MacroAssembler::TrustedImm32(-8), MacroAssembler::stackPointerRegister);
        m_jit->push(JIT::TrustedImm32(JIT::TrustedImmPtr(m_pc)));
        m_jit->push(JIT::callFrameRegister);
#elif CPU(X86_64) && OS(WINDOWS)
        m_jit->addPtr(MacroAssembler::TrustedImm32(-16), MacroAssembler::stackPointerRegister);
        m_jit->move(MacroAssembler::stackPointerRegister, JIT::argumentGPR0);
        m_jit->move(JIT::callFrameRegister, JIT::argumentGPR1);
        m_jit->move(JIT::TrustedImmPtr(m_pc), JIT::argumentGPR2);
#else
        m_jit->move(JIT::callFrameRegister, JIT::argumentGPR0);
        m_jit->move(JIT::TrustedImmPtr(m_pc), JIT::argumentGPR1);
#endif
        JIT::Call call = m_jit->call();
        m_jit->m_calls.append(CallRecord(call, m_jit->m_bytecodeOffset, m_stub.value()));

#if CPU(X86) && USE(JSVALUE32_64)
        m_jit->addPtr(MacroAssembler::TrustedImm32(16), MacroAssembler::stackPointerRegister);
#elif CPU(X86_64) && OS(WINDOWS)
        m_jit->pop(JIT::regT0); // vPC
        m_jit->pop(JIT::regT1); // callFrame register
#endif

#if ENABLE(OPCODE_SAMPLING)
        if (m_jit->m_bytecodeOffset != std::numeric_limits<unsigned>::max())
            m_jit->sampleInstruction(m_jit->m_codeBlock->instructions().begin() + m_jit->m_bytecodeOffset, false);
#endif
        
        m_jit->exceptionCheck();
        return call;
    }

private:
    JIT* m_jit;
    FunctionPtr m_stub;
    Instruction* m_pc;
};

} // namespace JS

#endif // ENABLE(JIT)
