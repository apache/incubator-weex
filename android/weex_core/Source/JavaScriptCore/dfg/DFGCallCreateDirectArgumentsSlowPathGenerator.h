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

#pragma once

#if ENABLE(DFG_JIT)

#include "DFGCommon.h"
#include "DFGOperations.h"
#include "DFGSlowPathGenerator.h"
#include "DFGSpeculativeJIT.h"
#include "DirectArguments.h"

namespace JSC { namespace DFG {

// This calls operationCreateDirectArguments but then restores the value of lengthGPR.
class CallCreateDirectArgumentsSlowPathGenerator : public JumpingSlowPathGenerator<MacroAssembler::JumpList> {
public:
    CallCreateDirectArgumentsSlowPathGenerator(
        MacroAssembler::JumpList from, SpeculativeJIT* jit, GPRReg resultGPR, RegisteredStructure structure,
        GPRReg lengthGPR, unsigned minCapacity)
        : JumpingSlowPathGenerator<MacroAssembler::JumpList>(from, jit)
        , m_resultGPR(resultGPR)
        , m_structure(structure)
        , m_lengthGPR(lengthGPR)
        , m_minCapacity(minCapacity)
    {
        jit->silentSpillAllRegistersImpl(false, m_plans, resultGPR);
    }

protected:
    void generateInternal(SpeculativeJIT* jit) override
    {
        linkFrom(jit);
        for (unsigned i = 0; i < m_plans.size(); ++i)
            jit->silentSpill(m_plans[i]);
        jit->callOperation(
            operationCreateDirectArguments, m_resultGPR, m_structure, m_lengthGPR, m_minCapacity);
        GPRReg canTrample = SpeculativeJIT::pickCanTrample(m_resultGPR);
        for (unsigned i = m_plans.size(); i--;)
            jit->silentFill(m_plans[i], canTrample);
        jit->m_jit.exceptionCheck();
        jit->m_jit.loadPtr(
            MacroAssembler::Address(m_resultGPR, DirectArguments::offsetOfLength()), m_lengthGPR);
        jumpTo(jit);
    }
    
private:
    GPRReg m_resultGPR;
    RegisteredStructure m_structure;
    GPRReg m_lengthGPR;
    unsigned m_minCapacity;
    Vector<SilentRegisterSavePlan, 2> m_plans;
};

} } // namespace JSC::DFG

#endif // ENABLE(DFG_JIT)
