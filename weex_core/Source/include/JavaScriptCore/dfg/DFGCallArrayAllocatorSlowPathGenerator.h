/*
 * Copyright (C) 2012 Apple Inc. All rights reserved.
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
#include "DFGSlowPathGenerator.h"
#include "DFGSpeculativeJIT.h"
#include <wtf/Vector.h>

namespace JSC { namespace DFG {

class CallArrayAllocatorSlowPathGenerator : public JumpingSlowPathGenerator<MacroAssembler::JumpList> {
public:
    CallArrayAllocatorSlowPathGenerator(
        MacroAssembler::JumpList from, SpeculativeJIT* jit, P_JITOperation_EStZB function,
        GPRReg resultGPR, GPRReg storageGPR, RegisteredStructure structure, size_t size)
        : JumpingSlowPathGenerator<MacroAssembler::JumpList>(from, jit)
        , m_function(function)
        , m_resultGPR(resultGPR)
        , m_storageGPR(storageGPR)
        , m_structure(structure)
        , m_size(size)
    {
        ASSERT(size < static_cast<size_t>(std::numeric_limits<int32_t>::max()));
        jit->silentSpillAllRegistersImpl(false, m_plans, resultGPR);
    }

protected:
    void generateInternal(SpeculativeJIT* jit) override
    {
        linkFrom(jit);
        for (unsigned i = 0; i < m_plans.size(); ++i)
            jit->silentSpill(m_plans[i]);
        jit->callOperation(m_function, m_resultGPR, m_structure, m_size, m_storageGPR);
        GPRReg canTrample = SpeculativeJIT::pickCanTrample(m_resultGPR);
        for (unsigned i = m_plans.size(); i--;)
            jit->silentFill(m_plans[i], canTrample);
        jit->m_jit.exceptionCheck();
        jit->m_jit.loadPtr(MacroAssembler::Address(m_resultGPR, JSObject::butterflyOffset()), m_storageGPR);
        jumpTo(jit);
    }
    
private:
    P_JITOperation_EStZB m_function;
    GPRReg m_resultGPR;
    GPRReg m_storageGPR;
    RegisteredStructure m_structure;
    size_t m_size;
    Vector<SilentRegisterSavePlan, 2> m_plans;
};

class CallArrayAllocatorWithVariableSizeSlowPathGenerator : public JumpingSlowPathGenerator<MacroAssembler::JumpList> {
public:
    CallArrayAllocatorWithVariableSizeSlowPathGenerator(
        MacroAssembler::JumpList from, SpeculativeJIT* jit, P_JITOperation_EStZB function,
        GPRReg resultGPR, RegisteredStructure contiguousStructure, RegisteredStructure arrayStorageStructure, GPRReg sizeGPR, GPRReg storageGPR)
        : JumpingSlowPathGenerator<MacroAssembler::JumpList>(from, jit)
        , m_function(function)
        , m_resultGPR(resultGPR)
        , m_contiguousStructure(contiguousStructure)
        , m_arrayStorageOrContiguousStructure(arrayStorageStructure)
        , m_sizeGPR(sizeGPR)
        , m_storageGPR(storageGPR)
    {
        jit->silentSpillAllRegistersImpl(false, m_plans, resultGPR);
    }

protected:
    void generateInternal(SpeculativeJIT* jit) override
    {
        linkFrom(jit);
        for (unsigned i = 0; i < m_plans.size(); ++i)
            jit->silentSpill(m_plans[i]);
        GPRReg scratchGPR = AssemblyHelpers::selectScratchGPR(m_sizeGPR, m_storageGPR);
        if (m_contiguousStructure.get() != m_arrayStorageOrContiguousStructure.get()) {
            MacroAssembler::Jump bigLength = jit->m_jit.branch32(MacroAssembler::AboveOrEqual, m_sizeGPR, MacroAssembler::TrustedImm32(MIN_ARRAY_STORAGE_CONSTRUCTION_LENGTH));
            jit->m_jit.move(SpeculativeJIT::TrustedImmPtr(m_contiguousStructure), scratchGPR);
            MacroAssembler::Jump done = jit->m_jit.jump();
            bigLength.link(&jit->m_jit);
            jit->m_jit.move(SpeculativeJIT::TrustedImmPtr(m_arrayStorageOrContiguousStructure), scratchGPR);
            done.link(&jit->m_jit);
        } else
            jit->m_jit.move(SpeculativeJIT::TrustedImmPtr(m_contiguousStructure), scratchGPR);
        jit->callOperation(m_function, m_resultGPR, scratchGPR, m_sizeGPR, m_storageGPR);
        GPRReg canTrample = SpeculativeJIT::pickCanTrample(m_resultGPR);
        for (unsigned i = m_plans.size(); i--;)
            jit->silentFill(m_plans[i], canTrample);
        jit->m_jit.exceptionCheck();
        jumpTo(jit);
    }
    
private:
    P_JITOperation_EStZB m_function;
    GPRReg m_resultGPR;
    RegisteredStructure m_contiguousStructure;
    RegisteredStructure m_arrayStorageOrContiguousStructure;
    GPRReg m_sizeGPR;
    GPRReg m_storageGPR;
    Vector<SilentRegisterSavePlan, 2> m_plans;
};

class CallArrayAllocatorWithVariableStructureVariableSizeSlowPathGenerator : public JumpingSlowPathGenerator<MacroAssembler::JumpList> {
public:
    CallArrayAllocatorWithVariableStructureVariableSizeSlowPathGenerator(
        MacroAssembler::JumpList from, SpeculativeJIT* jit, P_JITOperation_EStZB function,
        GPRReg resultGPR, GPRReg structureGPR, GPRReg sizeGPR, GPRReg storageGPR, GPRReg scratchGPR)
        : JumpingSlowPathGenerator<MacroAssembler::JumpList>(from, jit)
        , m_function(function)
        , m_resultGPR(resultGPR)
        , m_structureGPR(structureGPR)
        , m_sizeGPR(sizeGPR)
        , m_storageGPR(storageGPR)
        , m_scratchGPR(scratchGPR)
    {
        jit->silentSpillAllRegistersImpl(false, m_plans, resultGPR, m_scratchGPR);
    }

protected:
    void generateInternal(SpeculativeJIT* jit) override
    {
        linkFrom(jit);
        for (unsigned i = 0; i < m_plans.size(); ++i)
            jit->silentSpill(m_plans[i]);
        jit->callOperation(m_function, m_resultGPR, m_structureGPR, m_sizeGPR, m_storageGPR);
        for (unsigned i = m_plans.size(); i--;)
            jit->silentFill(m_plans[i], m_scratchGPR);
        jit->m_jit.exceptionCheck();
        jumpTo(jit);
    }
    
private:
    P_JITOperation_EStZB m_function;
    GPRReg m_resultGPR;
    GPRReg m_structureGPR;
    GPRReg m_sizeGPR;
    GPRReg m_storageGPR;
    GPRReg m_scratchGPR;
    Vector<SilentRegisterSavePlan, 2> m_plans;
};

} } // namespace JSC::DFG

#endif // ENABLE(DFG_JIT)
