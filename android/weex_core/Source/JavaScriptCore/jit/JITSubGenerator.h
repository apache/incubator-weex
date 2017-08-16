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

#pragma once

#if ENABLE(JIT)

#include "CCallHelpers.h"
#include "JITMathICInlineResult.h"
#include "SnippetOperand.h"

namespace JSC {

struct MathICGenerationState;

class JITSubGenerator {
public:
    JITSubGenerator() { }

    JITSubGenerator(SnippetOperand leftOperand, SnippetOperand rightOperand,
        JSValueRegs result, JSValueRegs left, JSValueRegs right,
        FPRReg leftFPR, FPRReg rightFPR, GPRReg scratchGPR, FPRReg scratchFPR)
        : m_leftOperand(leftOperand)
        , m_rightOperand(rightOperand)
        , m_result(result)
        , m_left(left)
        , m_right(right)
        , m_leftFPR(leftFPR)
        , m_rightFPR(rightFPR)
        , m_scratchGPR(scratchGPR)
        , m_scratchFPR(scratchFPR)
    { }

    JITMathICInlineResult generateInline(CCallHelpers&, MathICGenerationState&, const ArithProfile*);
    bool generateFastPath(CCallHelpers&, CCallHelpers::JumpList& endJumpList, CCallHelpers::JumpList& slowPathJumpList, const ArithProfile*, bool shouldEmitProfiling);

    static bool isLeftOperandValidConstant(SnippetOperand) { return false; }
    static bool isRightOperandValidConstant(SnippetOperand) { return false; }

private:
    SnippetOperand m_leftOperand;
    SnippetOperand m_rightOperand;
    JSValueRegs m_result;
    JSValueRegs m_left;
    JSValueRegs m_right;
    FPRReg m_leftFPR;
    FPRReg m_rightFPR;
    GPRReg m_scratchGPR;
    FPRReg m_scratchFPR;
};

} // namespace JSC

#endif // ENABLE(JIT)
