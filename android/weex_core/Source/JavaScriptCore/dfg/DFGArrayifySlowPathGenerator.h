/*
 * Copyright (C) 2012, 2013 Apple Inc. All rights reserved.
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

#include "DFGArrayMode.h"
#include "DFGCommon.h"
#include "DFGOSRExitJumpPlaceholder.h"
#include "DFGOperations.h"
#include "DFGSlowPathGenerator.h"
#include "DFGSpeculativeJIT.h"
#include <wtf/Vector.h>

namespace JSC { namespace DFG {

class ArrayifySlowPathGenerator : public JumpingSlowPathGenerator<MacroAssembler::JumpList> {
public:
    ArrayifySlowPathGenerator(
        const MacroAssembler::JumpList& from, SpeculativeJIT* jit, Node* node, GPRReg baseGPR,
        GPRReg propertyGPR, GPRReg tempGPR, GPRReg structureGPR)
        : JumpingSlowPathGenerator<MacroAssembler::JumpList>(from, jit)
        , m_op(node->op())
        , m_arrayMode(node->arrayMode())
        , m_structure(node->op() == ArrayifyToStructure ? node->structure() : RegisteredStructure())
        , m_baseGPR(baseGPR)
        , m_propertyGPR(propertyGPR)
        , m_tempGPR(tempGPR)
        , m_structureGPR(structureGPR)
    {
        ASSERT(m_op == Arrayify || m_op == ArrayifyToStructure);
        
        jit->silentSpillAllRegistersImpl(false, m_plans, InvalidGPRReg);
        
        if (m_propertyGPR != InvalidGPRReg) {
            switch (m_arrayMode.type()) {
            case Array::Int32:
            case Array::Double:
            case Array::Contiguous:
                m_badPropertyJump = jit->speculationCheck(Uncountable, JSValueRegs(), 0);
                break;
            default:
                break;
            }
        }
        m_badIndexingTypeJump = jit->speculationCheck(BadIndexingType, JSValueSource::unboxedCell(m_baseGPR), 0);
    }
    
protected:
    void generateInternal(SpeculativeJIT* jit) override
    {
        linkFrom(jit);
        
        ASSERT(m_op == Arrayify || m_op == ArrayifyToStructure);
        
        if (m_propertyGPR != InvalidGPRReg) {
            switch (m_arrayMode.type()) {
            case Array::Int32:
            case Array::Double:
            case Array::Contiguous:
                m_badPropertyJump.fill(jit, jit->m_jit.branch32(
                    MacroAssembler::AboveOrEqual, m_propertyGPR,
                    MacroAssembler::TrustedImm32(MIN_SPARSE_ARRAY_INDEX)));
                break;
            default:
                break;
            }
        }
        
        for (unsigned i = 0; i < m_plans.size(); ++i)
            jit->silentSpill(m_plans[i]);
        switch (m_arrayMode.type()) {
        case Array::Int32:
            jit->callOperation(operationEnsureInt32, m_tempGPR, m_baseGPR);
            break;
        case Array::Double:
            jit->callOperation(operationEnsureDouble, m_tempGPR, m_baseGPR);
            break;
        case Array::Contiguous:
            jit->callOperation(operationEnsureContiguous, m_tempGPR, m_baseGPR);
            break;
        case Array::ArrayStorage:
        case Array::SlowPutArrayStorage:
            jit->callOperation(operationEnsureArrayStorage, m_tempGPR, m_baseGPR);
            break;
        default:
            CRASH();
            break;
        }
        for (unsigned i = m_plans.size(); i--;)
            jit->silentFill(m_plans[i], GPRInfo::regT0);
        jit->m_jit.exceptionCheck();
        
        if (m_op == ArrayifyToStructure) {
            ASSERT(m_structure.get());
            m_badIndexingTypeJump.fill(
                jit, jit->m_jit.branchWeakStructure(MacroAssembler::NotEqual, MacroAssembler::Address(m_baseGPR, JSCell::structureIDOffset()), m_structure));
        } else {
            // Finally, check that we have the kind of array storage that we wanted to get.
            // Note that this is a backwards speculation check, which will result in the 
            // bytecode operation corresponding to this arrayification being reexecuted.
            // That's fine, since arrayification is not user-visible.
            jit->m_jit.load8(
                MacroAssembler::Address(m_baseGPR, JSCell::indexingTypeAndMiscOffset()),
                m_structureGPR);
            m_badIndexingTypeJump.fill(
                jit, jit->jumpSlowForUnwantedArrayMode(m_structureGPR, m_arrayMode));
        }
        
        jumpTo(jit);
    }
    
private:
    NodeType m_op;
    ArrayMode m_arrayMode;
    RegisteredStructure m_structure;
    GPRReg m_baseGPR;
    GPRReg m_propertyGPR;
    GPRReg m_tempGPR;
    GPRReg m_structureGPR;
    OSRExitJumpPlaceholder m_badPropertyJump;
    OSRExitJumpPlaceholder m_badIndexingTypeJump;
    Vector<SilentRegisterSavePlan, 2> m_plans;
};

} } // namespace JSC::DFG

#endif // ENABLE(DFG_JIT)
