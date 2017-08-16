/*
 * Copyright (C) 2015, 2016 Apple Inc. All rights reserved.
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
#include "JITNegGenerator.h"

#include "ArithProfile.h"

#if ENABLE(JIT)

namespace JSC {

JITMathICInlineResult JITNegGenerator::generateInline(CCallHelpers& jit, MathICGenerationState& state, const ArithProfile* arithProfile)
{
    ASSERT(m_scratchGPR != InvalidGPRReg);
    ASSERT(m_scratchGPR != m_src.payloadGPR());
    ASSERT(m_scratchGPR != m_result.payloadGPR());
#if USE(JSVALUE32_64)
    ASSERT(m_scratchGPR != m_src.tagGPR());
    ASSERT(m_scratchGPR != m_result.tagGPR());
#endif

    // We default to speculating int32.
    ObservedType observedTypes = ObservedType().withInt32();
    if (arithProfile)
        observedTypes = arithProfile->lhsObservedType();
    ASSERT_WITH_MESSAGE(!observedTypes.isEmpty(), "We should not attempt to generate anything if we do not have a profile.");

    if (observedTypes.isOnlyNonNumber())
        return JITMathICInlineResult::DontGenerate;

    if (observedTypes.isOnlyInt32()) {
        jit.moveValueRegs(m_src, m_result);
        state.slowPathJumps.append(jit.branchIfNotInt32(m_src));
        state.slowPathJumps.append(jit.branchTest32(CCallHelpers::Zero, m_src.payloadGPR(), CCallHelpers::TrustedImm32(0x7fffffff)));
        jit.neg32(m_result.payloadGPR());
#if USE(JSVALUE64)
        jit.boxInt32(m_result.payloadGPR(), m_result);
#endif

        return JITMathICInlineResult::GeneratedFastPath;
    }
    if (observedTypes.isOnlyNumber()) {
        state.slowPathJumps.append(jit.branchIfInt32(m_src));
        state.slowPathJumps.append(jit.branchIfNotNumber(m_src, m_scratchGPR));
#if USE(JSVALUE64)
        if (m_src.payloadGPR() != m_result.payloadGPR()) {
            jit.move(CCallHelpers::TrustedImm64(static_cast<int64_t>(1ull << 63)), m_result.payloadGPR());
            jit.xor64(m_src.payloadGPR(), m_result.payloadGPR());
        } else {
            jit.move(CCallHelpers::TrustedImm64(static_cast<int64_t>(1ull << 63)), m_scratchGPR);
            jit.xor64(m_scratchGPR, m_result.payloadGPR());
        }
#else
        jit.moveValueRegs(m_src, m_result);
        jit.xor32(CCallHelpers::TrustedImm32(1 << 31), m_result.tagGPR());
#endif
        return JITMathICInlineResult::GeneratedFastPath;
    }
    return JITMathICInlineResult::GenerateFullSnippet;
}

bool JITNegGenerator::generateFastPath(CCallHelpers& jit, CCallHelpers::JumpList& endJumpList, CCallHelpers::JumpList& slowPathJumpList, const ArithProfile* arithProfile, bool shouldEmitProfiling)
{
    ASSERT(m_scratchGPR != m_src.payloadGPR());
    ASSERT(m_scratchGPR != m_result.payloadGPR());
    ASSERT(m_scratchGPR != InvalidGPRReg);
#if USE(JSVALUE32_64)
    ASSERT(m_scratchGPR != m_src.tagGPR());
    ASSERT(m_scratchGPR != m_result.tagGPR());
#endif

    jit.moveValueRegs(m_src, m_result);
    CCallHelpers::Jump srcNotInt = jit.branchIfNotInt32(m_src);

    // -0 should produce a double, and hence cannot be negated as an int.
    // The negative int32 0x80000000 doesn't have a positive int32 representation, and hence cannot be negated as an int.
    slowPathJumpList.append(jit.branchTest32(CCallHelpers::Zero, m_src.payloadGPR(), CCallHelpers::TrustedImm32(0x7fffffff)));

    jit.neg32(m_result.payloadGPR());
#if USE(JSVALUE64)
    jit.boxInt32(m_result.payloadGPR(), m_result);
#endif
    endJumpList.append(jit.jump());

    srcNotInt.link(&jit);
    slowPathJumpList.append(jit.branchIfNotNumber(m_src, m_scratchGPR));

    // For a double, all we need to do is to invert the sign bit.
#if USE(JSVALUE64)
    jit.move(CCallHelpers::TrustedImm64((int64_t)(1ull << 63)), m_scratchGPR);
    jit.xor64(m_scratchGPR, m_result.payloadGPR());
#else
    jit.xor32(CCallHelpers::TrustedImm32(1 << 31), m_result.tagGPR());
#endif
    // The flags of ArithNegate are basic in DFG.
    // We only need to know if we ever produced a number.
    if (shouldEmitProfiling && arithProfile && !arithProfile->lhsObservedType().sawNumber() && !arithProfile->didObserveDouble())
        arithProfile->emitSetDouble(jit);
    return true;
}

} // namespace JSC

#endif // ENABLE(JIT)
