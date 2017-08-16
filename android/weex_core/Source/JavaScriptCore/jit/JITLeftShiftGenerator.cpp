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
#include "JITLeftShiftGenerator.h"

#if ENABLE(JIT)

namespace JSC {

void JITLeftShiftGenerator::generateFastPath(CCallHelpers& jit)
{
    ASSERT(m_scratchGPR != InvalidGPRReg);
    ASSERT(m_scratchGPR != m_left.payloadGPR());
    ASSERT(m_scratchGPR != m_right.payloadGPR());
#if USE(JSVALUE32_64)
    ASSERT(m_scratchGPR != m_left.tagGPR());
    ASSERT(m_scratchGPR != m_right.tagGPR());
#endif

    ASSERT(!m_leftOperand.isConstInt32() || !m_rightOperand.isConstInt32());

    m_didEmitFastPath = true;

    if (m_rightOperand.isConstInt32()) {
        // Try to do (intVar << intConstant).
        m_slowPathJumpList.append(jit.branchIfNotInt32(m_left));

        jit.moveValueRegs(m_left, m_result);
        jit.lshift32(CCallHelpers::Imm32(m_rightOperand.asConstInt32()), m_result.payloadGPR());

    } else {
        // Try to do (intConstant << intVar) or (intVar << intVar).
        m_slowPathJumpList.append(jit.branchIfNotInt32(m_right));

        GPRReg rightOperandGPR = m_right.payloadGPR();
        if (rightOperandGPR == m_result.payloadGPR()) {
            jit.move(rightOperandGPR, m_scratchGPR);
            rightOperandGPR = m_scratchGPR;
        }

        if (m_leftOperand.isConstInt32()) {
#if USE(JSVALUE32_64)
            jit.move(m_right.tagGPR(), m_result.tagGPR());
#endif
            jit.move(CCallHelpers::Imm32(m_leftOperand.asConstInt32()), m_result.payloadGPR());
        } else {
            m_slowPathJumpList.append(jit.branchIfNotInt32(m_left));
            jit.moveValueRegs(m_left, m_result);
        }

        jit.lshift32(rightOperandGPR, m_result.payloadGPR());
    }

#if USE(JSVALUE64)
    jit.or64(GPRInfo::tagTypeNumberRegister, m_result.payloadGPR());
#endif
}

} // namespace JSC

#endif // ENABLE(JIT)
