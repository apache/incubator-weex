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
