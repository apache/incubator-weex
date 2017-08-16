/*
 * Copyright (C) 2015-2016 Apple Inc. All rights reserved.
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
#include "JITAddGenerator.h"

#include "ArithProfile.h"
#include "JITMathIC.h"
#include "LinkBuffer.h"

#if ENABLE(JIT)

namespace JSC {

JITMathICInlineResult JITAddGenerator::generateInline(CCallHelpers& jit, MathICGenerationState& state, const ArithProfile* arithProfile)
{
    // We default to speculating int32.
    ObservedType lhs = ObservedType().withInt32();
    ObservedType rhs = ObservedType().withInt32();
    if (arithProfile) {
        lhs = arithProfile->lhsObservedType();
        rhs = arithProfile->rhsObservedType();
    }

    if (lhs.isOnlyNonNumber() && rhs.isOnlyNonNumber())
        return JITMathICInlineResult::DontGenerate;

    if ((lhs.isOnlyInt32() || m_leftOperand.isConstInt32()) && (rhs.isOnlyInt32() || m_rightOperand.isConstInt32())) {
        ASSERT(!m_leftOperand.isConstInt32() || !m_rightOperand.isConstInt32());
        if (!m_leftOperand.isConstInt32())
            state.slowPathJumps.append(jit.branchIfNotInt32(m_left));
        if (!m_rightOperand.isConstInt32())
            state.slowPathJumps.append(jit.branchIfNotInt32(m_right));

        GPRReg scratch = m_scratchGPR;
        if (m_leftOperand.isConstInt32() || m_rightOperand.isConstInt32()) {
            JSValueRegs var = m_leftOperand.isConstInt32() ? m_right : m_left;
            int32_t constValue = m_leftOperand.isConstInt32() ? m_leftOperand.asConstInt32() : m_rightOperand.asConstInt32();
            if (var.payloadGPR() != m_result.payloadGPR())
                scratch = m_result.payloadGPR();
            state.slowPathJumps.append(jit.branchAdd32(CCallHelpers::Overflow, var.payloadGPR(), CCallHelpers::Imm32(constValue), scratch));
        } else {
            if (m_left.payloadGPR() != m_result.payloadGPR() && m_right.payloadGPR() != m_result.payloadGPR())
                scratch = m_result.payloadGPR();
            state.slowPathJumps.append(jit.branchAdd32(CCallHelpers::Overflow, m_right.payloadGPR(), m_left.payloadGPR(), scratch));
        }
        jit.boxInt32(scratch, m_result);
        return JITMathICInlineResult::GeneratedFastPath;
    }

    return JITMathICInlineResult::GenerateFullSnippet;
}

bool JITAddGenerator::generateFastPath(CCallHelpers& jit, CCallHelpers::JumpList& endJumpList, CCallHelpers::JumpList& slowPathJumpList, const ArithProfile* arithProfile, bool shouldEmitProfiling)
{
    ASSERT(m_scratchGPR != InvalidGPRReg);
    ASSERT(m_scratchGPR != m_left.payloadGPR());
    ASSERT(m_scratchGPR != m_right.payloadGPR());
#if USE(JSVALUE32_64)
    ASSERT(m_scratchGPR != m_left.tagGPR());
    ASSERT(m_scratchGPR != m_right.tagGPR());
    ASSERT(m_scratchFPR != InvalidFPRReg);
#endif

    ASSERT(!m_leftOperand.isConstInt32() || !m_rightOperand.isConstInt32());
    
    if (!m_leftOperand.mightBeNumber() || !m_rightOperand.mightBeNumber())
        return false;

    if (m_leftOperand.isConstInt32() || m_rightOperand.isConstInt32()) {
        JSValueRegs var = m_leftOperand.isConstInt32() ? m_right : m_left;
        SnippetOperand& varOpr = m_leftOperand.isConstInt32() ? m_rightOperand : m_leftOperand;
        SnippetOperand& constOpr = m_leftOperand.isConstInt32() ? m_leftOperand : m_rightOperand;

        // Try to do intVar + intConstant.
        CCallHelpers::Jump notInt32 = jit.branchIfNotInt32(var);

        GPRReg scratch = m_scratchGPR;
        if (var.payloadGPR() != m_result.payloadGPR())
            scratch = m_result.payloadGPR();
        slowPathJumpList.append(jit.branchAdd32(CCallHelpers::Overflow, var.payloadGPR(), CCallHelpers::Imm32(constOpr.asConstInt32()), scratch));

        jit.boxInt32(scratch, m_result);
        endJumpList.append(jit.jump());

        if (!jit.supportsFloatingPoint()) {
            slowPathJumpList.append(notInt32);
            return true;
        }

        // Try to do doubleVar + double(intConstant).
        notInt32.link(&jit);
        if (!varOpr.definitelyIsNumber())
            slowPathJumpList.append(jit.branchIfNotNumber(var, m_scratchGPR));

        jit.unboxDoubleNonDestructive(var, m_leftFPR, m_scratchGPR, m_scratchFPR);

        jit.move(CCallHelpers::Imm32(constOpr.asConstInt32()), m_scratchGPR);
        jit.convertInt32ToDouble(m_scratchGPR, m_rightFPR);

        // Fall thru to doubleVar + doubleVar.

    } else {
        ASSERT(!m_leftOperand.isConstInt32() && !m_rightOperand.isConstInt32());
        CCallHelpers::Jump leftNotInt;
        CCallHelpers::Jump rightNotInt;

        // Try to do intVar + intVar.
        leftNotInt = jit.branchIfNotInt32(m_left);
        rightNotInt = jit.branchIfNotInt32(m_right);

        GPRReg scratch = m_scratchGPR;
        if (m_left.payloadGPR() != m_result.payloadGPR() && m_right.payloadGPR() != m_result.payloadGPR())
            scratch = m_result.payloadGPR();
        slowPathJumpList.append(jit.branchAdd32(CCallHelpers::Overflow, m_right.payloadGPR(), m_left.payloadGPR(), scratch));

        jit.boxInt32(scratch, m_result);
        endJumpList.append(jit.jump());


        if (!jit.supportsFloatingPoint()) {
            slowPathJumpList.append(leftNotInt);
            slowPathJumpList.append(rightNotInt);
            return true;
        }

        leftNotInt.link(&jit);
        if (!m_leftOperand.definitelyIsNumber())
            slowPathJumpList.append(jit.branchIfNotNumber(m_left, m_scratchGPR));
        if (!m_rightOperand.definitelyIsNumber())
            slowPathJumpList.append(jit.branchIfNotNumber(m_right, m_scratchGPR));

        jit.unboxDoubleNonDestructive(m_left, m_leftFPR, m_scratchGPR, m_scratchFPR);
        CCallHelpers::Jump rightIsDouble = jit.branchIfNotInt32(m_right);

        jit.convertInt32ToDouble(m_right.payloadGPR(), m_rightFPR);
        CCallHelpers::Jump rightWasInteger = jit.jump();

        rightNotInt.link(&jit);
        if (!m_rightOperand.definitelyIsNumber())
            slowPathJumpList.append(jit.branchIfNotNumber(m_right, m_scratchGPR));

        jit.convertInt32ToDouble(m_left.payloadGPR(), m_leftFPR);

        rightIsDouble.link(&jit);
        jit.unboxDoubleNonDestructive(m_right, m_rightFPR, m_scratchGPR, m_scratchFPR);

        rightWasInteger.link(&jit);

        // Fall thru to doubleVar + doubleVar.
    }

    // Do doubleVar + doubleVar.
    jit.addDouble(m_rightFPR, m_leftFPR);
    if (arithProfile && shouldEmitProfiling)
        arithProfile->emitSetDouble(jit);

    jit.boxDouble(m_leftFPR, m_result);

    return true;
}

} // namespace JSC

#endif // ENABLE(JIT)
