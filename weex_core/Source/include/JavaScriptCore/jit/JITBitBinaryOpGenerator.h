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
#include "SnippetOperand.h"

namespace JSC {

class JITBitBinaryOpGenerator {
public:
    JITBitBinaryOpGenerator(const SnippetOperand& leftOperand, const SnippetOperand& rightOperand,
        JSValueRegs result, JSValueRegs left, JSValueRegs right, GPRReg scratchGPR)
        : m_leftOperand(leftOperand)
        , m_rightOperand(rightOperand)
        , m_result(result)
        , m_left(left)
        , m_right(right)
        , m_scratchGPR(scratchGPR)
    {
        ASSERT(!m_leftOperand.isConstInt32() || !m_rightOperand.isConstInt32());
    }

    bool didEmitFastPath() const { return m_didEmitFastPath; }
    CCallHelpers::JumpList& endJumpList() { return m_endJumpList; }
    CCallHelpers::JumpList& slowPathJumpList() { return m_slowPathJumpList; }

protected:
    SnippetOperand m_leftOperand;
    SnippetOperand m_rightOperand;
    JSValueRegs m_result;
    JSValueRegs m_left;
    JSValueRegs m_right;
    GPRReg m_scratchGPR;
    bool m_didEmitFastPath { false };

    CCallHelpers::JumpList m_endJumpList;
    CCallHelpers::JumpList m_slowPathJumpList;
};

} // namespace JSC

#endif // ENABLE(JIT)
