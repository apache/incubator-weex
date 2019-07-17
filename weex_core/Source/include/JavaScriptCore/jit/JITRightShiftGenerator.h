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

#pragma once

#if ENABLE(JIT)

#include "JITBitBinaryOpGenerator.h"

namespace JSC {

class JITRightShiftGenerator : public JITBitBinaryOpGenerator {
public:
    enum ShiftType {
        SignedShift,
        UnsignedShift
    };

    JITRightShiftGenerator(const SnippetOperand& leftOperand, const SnippetOperand& rightOperand,
        JSValueRegs result, JSValueRegs left, JSValueRegs right,
        FPRReg leftFPR, GPRReg scratchGPR, FPRReg scratchFPR, ShiftType type = SignedShift)
        : JITBitBinaryOpGenerator(leftOperand, rightOperand, result, left, right, scratchGPR)
        , m_shiftType(type)
        , m_leftFPR(leftFPR)
        , m_scratchFPR(scratchFPR)
    { }

    void generateFastPath(CCallHelpers&);

private:
    ShiftType m_shiftType;
    FPRReg m_leftFPR;
    FPRReg m_scratchFPR;
};

} // namespace JSC

#endif // ENABLE(JIT)
