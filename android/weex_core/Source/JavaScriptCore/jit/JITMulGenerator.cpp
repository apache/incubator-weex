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
#include "JITMulGenerator.h"

#if ENABLE(JIT)

#include "ArithProfile.h"
#include "JITMathIC.h"

namespace JSC {

JITMathICInlineResult JITMulGenerator::generateInline(CCallHelpers& jit, MathICGenerationState& state, const ArithProfile* arithProfile)
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

    if (lhs.isOnlyNumber() && rhs.isOnlyNumber()) {
        if (!jit.supportsFloatingPoint())
            return JITMathICInlineResult::DontGenerate;

        if (!m_leftOperand.definitelyIsNumber())
            state.slowPathJumps.append(jit.branchIfNotNumber(m_left, m_scratchGPR));
        if (!m_rightOperand.definitelyIsNumber())
            state.slowPathJumps.append(jit.branchIfNotNumber(m_right, m_scratchGPR));
        state.slowPathJumps.append(jit.branchIfInt32(m_left));
        state.slowPathJumps.append(jit.branchIfInt32(m_right));
        jit.unboxDoubleNonDestructive(m_left, m_leftFPR, m_scratchGPR, m_scratchFPR);
        jit.unboxDoubleNonDestructive(m_right, m_rightFPR, m_scratchGPR, m_scratchFPR);
        jit.mulDouble(m_rightFPR, m_leftFPR);
        jit.boxDouble(m_leftFPR, m_result);

        return JITMathICInlineResult::GeneratedFastPath;
    }

    if ((lhs.isOnlyInt32() || m_leftOperand.isPositiveConstInt32()) && (rhs.isOnlyInt32() || m_rightOperand.isPositiveConstInt32())) {
        ASSERT(!m_leftOperand.isPositiveConstInt32() || !m_rightOperand.isPositiveConstInt32());
        if (!m_leftOperand.isPositiveConstInt32())
            state.slowPathJumps.append(jit.branchIfNotInt32(m_left));
        if (!m_rightOperand.isPositiveConstInt32())
            state.slowPathJumps.append(jit.branchIfNotInt32(m_right));

        if (m_leftOperand.isPositiveConstInt32() || m_rightOperand.isPositiveConstInt32()) {
            JSValueRegs var = m_leftOperand.isPositiveConstInt32() ? m_right : m_left;
            int32_t constValue = m_leftOperand.isPositiveConstInt32() ? m_leftOperand.asConstInt32() : m_rightOperand.asConstInt32();
            state.slowPathJumps.append(jit.branchMul32(CCallHelpers::Overflow, var.payloadGPR(), CCallHelpers::Imm32(constValue), m_scratchGPR));
        } else {
            state.slowPathJumps.append(jit.branchMul32(CCallHelpers::Overflow, m_right.payloadGPR(), m_left.payloadGPR(), m_scratchGPR));
            state.slowPathJumps.append(jit.branchTest32(CCallHelpers::Zero, m_scratchGPR)); // Go slow if potential negative zero.
        }
        jit.boxInt32(m_scratchGPR, m_result);

        return JITMathICInlineResult::GeneratedFastPath;
    }

    return JITMathICInlineResult::GenerateFullSnippet;
}

bool JITMulGenerator::generateFastPath(CCallHelpers& jit, CCallHelpers::JumpList& endJumpList, CCallHelpers::JumpList& slowPathJumpList, const ArithProfile* arithProfile, bool shouldEmitProfiling)
{
    ASSERT(m_scratchGPR != InvalidGPRReg);
    ASSERT(m_scratchGPR != m_left.payloadGPR());
    ASSERT(m_scratchGPR != m_right.payloadGPR());
#if USE(JSVALUE64)
    ASSERT(m_scratchGPR != m_result.payloadGPR());
#else
    ASSERT(m_scratchGPR != m_left.tagGPR());
    ASSERT(m_scratchGPR != m_right.tagGPR());
    ASSERT(m_scratchFPR != InvalidFPRReg);
#endif

    ASSERT(!m_leftOperand.isPositiveConstInt32() || !m_rightOperand.isPositiveConstInt32());

    if (!m_leftOperand.mightBeNumber() || !m_rightOperand.mightBeNumber())
        return false;

    if (m_leftOperand.isPositiveConstInt32() || m_rightOperand.isPositiveConstInt32()) {
        JSValueRegs var = m_leftOperand.isPositiveConstInt32() ? m_right : m_left;
        SnippetOperand& varOpr = m_leftOperand.isPositiveConstInt32() ? m_rightOperand : m_leftOperand;
        SnippetOperand& constOpr = m_leftOperand.isPositiveConstInt32() ? m_leftOperand : m_rightOperand;

        // Try to do intVar * intConstant.
        CCallHelpers::Jump notInt32 = jit.branchIfNotInt32(var);

        GPRReg multiplyResultGPR = m_result.payloadGPR();
        if (multiplyResultGPR == var.payloadGPR())
            multiplyResultGPR = m_scratchGPR;

        slowPathJumpList.append(jit.branchMul32(CCallHelpers::Overflow, var.payloadGPR(), CCallHelpers::Imm32(constOpr.asConstInt32()), multiplyResultGPR));

        jit.boxInt32(multiplyResultGPR, m_result);
        endJumpList.append(jit.jump());

        if (!jit.supportsFloatingPoint()) {
            slowPathJumpList.append(notInt32);
            return true;
        }

        // Try to do doubleVar * double(intConstant).
        notInt32.link(&jit);
        if (!varOpr.definitelyIsNumber())
            slowPathJumpList.append(jit.branchIfNotNumber(var, m_scratchGPR));

        jit.unboxDoubleNonDestructive(var, m_leftFPR, m_scratchGPR, m_scratchFPR);

        jit.move(CCallHelpers::Imm32(constOpr.asConstInt32()), m_scratchGPR);
        jit.convertInt32ToDouble(m_scratchGPR, m_rightFPR);

        // Fall thru to doubleVar * doubleVar.

    } else {
        ASSERT(!m_leftOperand.isPositiveConstInt32() && !m_rightOperand.isPositiveConstInt32());

        CCallHelpers::Jump leftNotInt;
        CCallHelpers::Jump rightNotInt;

        // Try to do intVar * intVar.
        leftNotInt = jit.branchIfNotInt32(m_left);
        rightNotInt = jit.branchIfNotInt32(m_right);

        slowPathJumpList.append(jit.branchMul32(CCallHelpers::Overflow, m_right.payloadGPR(), m_left.payloadGPR(), m_scratchGPR));
        slowPathJumpList.append(jit.branchTest32(CCallHelpers::Zero, m_scratchGPR)); // Go slow if potential negative zero.

        jit.boxInt32(m_scratchGPR, m_result);
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

        // Fall thru to doubleVar * doubleVar.
    }

    // Do doubleVar * doubleVar.
    jit.mulDouble(m_rightFPR, m_leftFPR);

    if (!arithProfile || !shouldEmitProfiling)
        jit.boxDouble(m_leftFPR, m_result);
    else {
        // The Int52 overflow check below intentionally omits 1ll << 51 as a valid negative Int52 value.
        // Therefore, we will get a false positive if the result is that value. This is intentionally
        // done to simplify the checking algorithm.

        const int64_t negativeZeroBits = 1ll << 63;
#if USE(JSVALUE64)
        jit.moveDoubleTo64(m_leftFPR, m_result.payloadGPR());

        CCallHelpers::Jump notNegativeZero = jit.branch64(CCallHelpers::NotEqual, m_result.payloadGPR(), CCallHelpers::TrustedImm64(negativeZeroBits));

        jit.or32(CCallHelpers::TrustedImm32(ArithProfile::NegZeroDouble), CCallHelpers::AbsoluteAddress(arithProfile->addressOfBits()));
        CCallHelpers::Jump done = jit.jump();

        notNegativeZero.link(&jit);
        jit.or32(CCallHelpers::TrustedImm32(ArithProfile::NonNegZeroDouble), CCallHelpers::AbsoluteAddress(arithProfile->addressOfBits()));

        jit.move(m_result.payloadGPR(), m_scratchGPR);
        jit.urshiftPtr(CCallHelpers::Imm32(52), m_scratchGPR);
        jit.and32(CCallHelpers::Imm32(0x7ff), m_scratchGPR);
        CCallHelpers::Jump noInt52Overflow = jit.branch32(CCallHelpers::LessThanOrEqual, m_scratchGPR, CCallHelpers::TrustedImm32(0x431));

        jit.or32(CCallHelpers::TrustedImm32(ArithProfile::Int52Overflow), CCallHelpers::AbsoluteAddress(arithProfile->addressOfBits()));
        noInt52Overflow.link(&jit);

        done.link(&jit);
        jit.sub64(GPRInfo::tagTypeNumberRegister, m_result.payloadGPR()); // Box the double.
#else
        jit.boxDouble(m_leftFPR, m_result);
        CCallHelpers::JumpList notNegativeZero;
        notNegativeZero.append(jit.branch32(CCallHelpers::NotEqual, m_result.payloadGPR(), CCallHelpers::TrustedImm32(0)));
        notNegativeZero.append(jit.branch32(CCallHelpers::NotEqual, m_result.tagGPR(), CCallHelpers::TrustedImm32(negativeZeroBits >> 32)));

        jit.or32(CCallHelpers::TrustedImm32(ArithProfile::NegZeroDouble), CCallHelpers::AbsoluteAddress(arithProfile->addressOfBits()));
        CCallHelpers::Jump done = jit.jump();

        notNegativeZero.link(&jit);
        jit.or32(CCallHelpers::TrustedImm32(ArithProfile::NonNegZeroDouble), CCallHelpers::AbsoluteAddress(arithProfile->addressOfBits()));

        jit.move(m_result.tagGPR(), m_scratchGPR);
        jit.urshiftPtr(CCallHelpers::Imm32(52 - 32), m_scratchGPR);
        jit.and32(CCallHelpers::Imm32(0x7ff), m_scratchGPR);
        CCallHelpers::Jump noInt52Overflow = jit.branch32(CCallHelpers::LessThanOrEqual, m_scratchGPR, CCallHelpers::TrustedImm32(0x431));
        
        jit.or32(CCallHelpers::TrustedImm32(ArithProfile::Int52Overflow), CCallHelpers::AbsoluteAddress(arithProfile->addressOfBits()));

        endJumpList.append(noInt52Overflow);
        if (m_scratchGPR == m_result.tagGPR() || m_scratchGPR == m_result.payloadGPR())
            jit.boxDouble(m_leftFPR, m_result);

        endJumpList.append(done);
#endif
    }

    return true;
}

} // namespace JSC

#endif // ENABLE(JIT)
