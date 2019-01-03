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
