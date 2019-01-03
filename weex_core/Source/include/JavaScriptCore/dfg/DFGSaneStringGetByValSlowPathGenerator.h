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

#include "DFGCommon.h"
#include "DFGOperations.h"
#include "DFGSlowPathGenerator.h"
#include "DFGSpeculativeJIT.h"
#include <wtf/Vector.h>

namespace JSC { namespace DFG {

class SaneStringGetByValSlowPathGenerator : public JumpingSlowPathGenerator<MacroAssembler::Jump> {
public:
    SaneStringGetByValSlowPathGenerator(
        const MacroAssembler::Jump& from, SpeculativeJIT* jit, JSValueRegs resultRegs,
        GPRReg baseReg, GPRReg propertyReg)
        : JumpingSlowPathGenerator<MacroAssembler::Jump>(from, jit)
        , m_resultRegs(resultRegs)
        , m_baseReg(baseReg)
        , m_propertyReg(propertyReg)
    {
        jit->silentSpillAllRegistersImpl(false, m_plans, extractResult(resultRegs));
    }
    
protected:
    void generateInternal(SpeculativeJIT* jit) override
    {
        linkFrom(jit);
        
        MacroAssembler::Jump isNeg = jit->m_jit.branch32(
            MacroAssembler::LessThan, m_propertyReg, MacroAssembler::TrustedImm32(0));
        
#if USE(JSVALUE64)
        jit->m_jit.move(
            MacroAssembler::TrustedImm64(JSValue::encode(jsUndefined())), m_resultRegs.gpr());
#else
        jit->m_jit.move(
            MacroAssembler::TrustedImm32(JSValue::UndefinedTag), m_resultRegs.tagGPR());
        jit->m_jit.move(
            MacroAssembler::TrustedImm32(0), m_resultRegs.payloadGPR());
#endif
        jumpTo(jit);
        
        isNeg.link(&jit->m_jit);

        for (unsigned i = 0; i < m_plans.size(); ++i)
            jit->silentSpill(m_plans[i]);
        jit->callOperation(operationGetByValStringInt, extractResult(m_resultRegs), m_baseReg, m_propertyReg);
        GPRReg canTrample = SpeculativeJIT::pickCanTrample(extractResult(m_resultRegs));
        for (unsigned i = m_plans.size(); i--;)
            jit->silentFill(m_plans[i], canTrample);
        jit->m_jit.exceptionCheck();
        
        jumpTo(jit);
    }
    
private:
    JSValueRegs m_resultRegs;
    GPRReg m_baseReg;
    GPRReg m_propertyReg;
    Vector<SilentRegisterSavePlan, 2> m_plans;
};

} } // namespace JSC::DFG

#endif // ENABLE(DFG_JIT)
