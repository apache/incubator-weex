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

#if ENABLE(JIT)

#include "CCallHelpers.h"
#include "JITMathICInlineResult.h"
#include "SnippetOperand.h"

namespace JSC {

struct MathICGenerationState;

class JITMulGenerator {
public:
    JITMulGenerator()
    { }

    JITMulGenerator(SnippetOperand leftOperand, SnippetOperand rightOperand,
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
    {
        ASSERT(!m_leftOperand.isPositiveConstInt32() || !m_rightOperand.isPositiveConstInt32());
    }

    JITMathICInlineResult generateInline(CCallHelpers&, MathICGenerationState&, const ArithProfile*);
    bool generateFastPath(CCallHelpers&, CCallHelpers::JumpList& endJumpList, CCallHelpers::JumpList& slowJumpList, const ArithProfile*, bool shouldEmitProfiling);

    static bool isLeftOperandValidConstant(SnippetOperand leftOperand) { return leftOperand.isPositiveConstInt32(); }
    static bool isRightOperandValidConstant(SnippetOperand rightOperand) { return rightOperand.isPositiveConstInt32(); }
    
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
