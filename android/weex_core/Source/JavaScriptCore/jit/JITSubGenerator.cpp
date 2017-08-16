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
#include "JITSubGenerator.h"

#include "ArithProfile.h"
#include "JITMathIC.h"

#if ENABLE(JIT)

namespace JSC {

JITMathICInlineResult JITSubGenerator::generateInline(CCallHelpers& jit, MathICGenerationState& state, const ArithProfile* arithProfile)
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
        jit.subDouble(m_rightFPR, m_leftFPR);
        jit.boxDouble(m_leftFPR, m_result);

        return JITMathICInlineResult::GeneratedFastPath;
    }
    if (lhs.isOnlyInt32() && rhs.isOnlyInt32()) {
        ASSERT(!m_leftOperand.isConstInt32() || !m_rightOperand.isConstInt32());
        state.slowPathJumps.append(jit.branchIfNotInt32(m_left));
        state.slowPathJumps.append(jit.branchIfNotInt32(m_right));

        jit.move(m_left.payloadGPR(), m_scratchGPR);
        state.slowPathJumps.append(jit.branchSub32(CCallHelpers::Overflow, m_right.payloadGPR(), m_scratchGPR));

        jit.boxInt32(m_scratchGPR, m_result);
        return JITMathICInlineResult::GeneratedFastPath;
    }

    return JITMathICInlineResult::GenerateFullSnippet;
}

bool JITSubGenerator::generateFastPath(CCallHelpers& jit, CCallHelpers::JumpList& endJumpList, CCallHelpers::JumpList& slowPathJumpList, const ArithProfile* arithProfile, bool shouldEmitProfiling)
{
    ASSERT(m_scratchGPR != InvalidGPRReg);
    ASSERT(m_scratchGPR != m_left.payloadGPR());
    ASSERT(m_scratchGPR != m_right.payloadGPR());
#if USE(JSVALUE32_64)
    ASSERT(m_scratchGPR != m_left.tagGPR());
    ASSERT(m_scratchGPR != m_right.tagGPR());
    ASSERT(m_scratchFPR != InvalidFPRReg);
#endif

    CCallHelpers::Jump leftNotInt = jit.branchIfNotInt32(m_left);
    CCallHelpers::Jump rightNotInt = jit.branchIfNotInt32(m_right);

    jit.move(m_left.payloadGPR(), m_scratchGPR);
    slowPathJumpList.append(jit.branchSub32(CCallHelpers::Overflow, m_right.payloadGPR(), m_scratchGPR));

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

    jit.subDouble(m_rightFPR, m_leftFPR);
    if (arithProfile && shouldEmitProfiling)
        arithProfile->emitSetDouble(jit);

    jit.boxDouble(m_leftFPR, m_result);

    return true;
}

} // namespace JSC

#endif // ENABLE(JIT)
