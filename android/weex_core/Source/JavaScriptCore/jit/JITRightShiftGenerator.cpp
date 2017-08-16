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

#include "config.h"
#include "JITRightShiftGenerator.h"

#if ENABLE(JIT)

namespace JSC {

void JITRightShiftGenerator::generateFastPath(CCallHelpers& jit)
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

    m_didEmitFastPath = true;

    if (m_rightOperand.isConstInt32()) {
        // Try to do (intVar >> intConstant).
        CCallHelpers::Jump notInt = jit.branchIfNotInt32(m_left);

        jit.moveValueRegs(m_left, m_result);
        int32_t shiftAmount = m_rightOperand.asConstInt32() & 0x1f;
        if (shiftAmount) {
            if (m_shiftType == SignedShift)
                jit.rshift32(CCallHelpers::Imm32(shiftAmount), m_result.payloadGPR());
            else
                jit.urshift32(CCallHelpers::Imm32(shiftAmount), m_result.payloadGPR());
#if USE(JSVALUE64)
            jit.or64(GPRInfo::tagTypeNumberRegister, m_result.payloadGPR());
#endif
        }

        if (jit.supportsFloatingPointTruncate()) {
            m_endJumpList.append(jit.jump()); // Terminate the above case before emitting more code.

            // Try to do (doubleVar >> intConstant).
            notInt.link(&jit);

            m_slowPathJumpList.append(jit.branchIfNotNumber(m_left, m_scratchGPR));

            jit.unboxDoubleNonDestructive(m_left, m_leftFPR, m_scratchGPR, m_scratchFPR);
            m_slowPathJumpList.append(jit.branchTruncateDoubleToInt32(m_leftFPR, m_scratchGPR));

            if (shiftAmount) {
                if (m_shiftType == SignedShift)
                    jit.rshift32(CCallHelpers::Imm32(shiftAmount), m_scratchGPR);
                else
                    jit.urshift32(CCallHelpers::Imm32(shiftAmount), m_scratchGPR);
            }
            jit.boxInt32(m_scratchGPR, m_result);

        } else
            m_slowPathJumpList.append(notInt);

    } else {
        // Try to do (intConstant >> intVar) or (intVar >> intVar).
        m_slowPathJumpList.append(jit.branchIfNotInt32(m_right));

        GPRReg rightOperandGPR = m_right.payloadGPR();
        if (rightOperandGPR == m_result.payloadGPR())
            rightOperandGPR = m_scratchGPR;

        CCallHelpers::Jump leftNotInt;
        if (m_leftOperand.isConstInt32()) {
            jit.move(m_right.payloadGPR(), rightOperandGPR);
#if USE(JSVALUE32_64)
            jit.move(m_right.tagGPR(), m_result.tagGPR());
#endif
            jit.move(CCallHelpers::Imm32(m_leftOperand.asConstInt32()), m_result.payloadGPR());
        } else {
            leftNotInt = jit.branchIfNotInt32(m_left);
            jit.move(m_right.payloadGPR(), rightOperandGPR);
            jit.moveValueRegs(m_left, m_result);
        }

        if (m_shiftType == SignedShift)
            jit.rshift32(rightOperandGPR, m_result.payloadGPR());
        else
            jit.urshift32(rightOperandGPR, m_result.payloadGPR());
#if USE(JSVALUE64)
        jit.or64(GPRInfo::tagTypeNumberRegister, m_result.payloadGPR());
#endif
        if (m_leftOperand.isConstInt32())
            return;

        if (jit.supportsFloatingPointTruncate()) {
            m_endJumpList.append(jit.jump()); // Terminate the above case before emitting more code.

            // Try to do (doubleVar >> intVar).
            leftNotInt.link(&jit);

            m_slowPathJumpList.append(jit.branchIfNotNumber(m_left, m_scratchGPR));
            jit.unboxDoubleNonDestructive(m_left, m_leftFPR, m_scratchGPR, m_scratchFPR);
            m_slowPathJumpList.append(jit.branchTruncateDoubleToInt32(m_leftFPR, m_scratchGPR));

            if (m_shiftType == SignedShift)
                jit.rshift32(m_right.payloadGPR(), m_scratchGPR);
            else
                jit.urshift32(m_right.payloadGPR(), m_scratchGPR);
            jit.boxInt32(m_scratchGPR, m_result);

        } else
            m_slowPathJumpList.append(leftNotInt);
    }
}

} // namespace JSC

#endif // ENABLE(JIT)
