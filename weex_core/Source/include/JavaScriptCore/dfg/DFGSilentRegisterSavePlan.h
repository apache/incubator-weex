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

#if ENABLE(DFG_JIT)

#include "DFGCommon.h"
#include "FPRInfo.h"
#include "GPRInfo.h"

namespace JSC { namespace DFG {

enum SilentSpillAction {
    DoNothingForSpill,
    Store32Tag,
    Store32Payload,
    StorePtr,
    Store64,
    StoreDouble
};

enum SilentFillAction {
    DoNothingForFill,
    SetInt32Constant,
    SetInt52Constant,
    SetStrictInt52Constant,
    SetBooleanConstant,
    SetCellConstant,
    SetTrustedJSConstant,
    SetJSConstant,
    SetJSConstantTag,
    SetJSConstantPayload,
    SetInt32Tag,
    SetCellTag,
    SetBooleanTag,
    SetDoubleConstant,
    Load32Tag,
    Load32Payload,
    Load32PayloadBoxInt,
    Load32PayloadConvertToInt52,
    Load32PayloadSignExtend,
    LoadPtr,
    Load64,
    Load64ShiftInt52Right,
    Load64ShiftInt52Left,
    LoadDouble,
    LoadDoubleBoxDouble,
    LoadJSUnboxDouble
};

class SilentRegisterSavePlan {
public:
    SilentRegisterSavePlan()
        : m_spillAction(DoNothingForSpill)
        , m_fillAction(DoNothingForFill)
        , m_register(-1)
        , m_node(0)
    {
    }
    
    SilentRegisterSavePlan(
        SilentSpillAction spillAction,
        SilentFillAction fillAction,
        Node* node,
        GPRReg gpr)
        : m_spillAction(spillAction)
        , m_fillAction(fillAction)
        , m_register(gpr)
        , m_node(node)
    {
    }
    
    SilentRegisterSavePlan(
        SilentSpillAction spillAction,
        SilentFillAction fillAction,
        Node* node,
        FPRReg fpr)
        : m_spillAction(spillAction)
        , m_fillAction(fillAction)
        , m_register(fpr)
        , m_node(node)
    {
    }
    
    SilentSpillAction spillAction() const { return static_cast<SilentSpillAction>(m_spillAction); }
    SilentFillAction fillAction() const { return static_cast<SilentFillAction>(m_fillAction); }
    
    Node* node() const { return m_node; }
    
    GPRReg gpr() const { return static_cast<GPRReg>(m_register); }
    FPRReg fpr() const { return static_cast<FPRReg>(m_register); }
    
private:
    int8_t m_spillAction;
    int8_t m_fillAction;
    int8_t m_register;
    Node* m_node;
};

} } // namespace JSC::DFG

#endif // ENABLE(DFG_JIT)
