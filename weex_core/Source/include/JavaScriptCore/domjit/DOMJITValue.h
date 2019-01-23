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

#include "DOMJITReg.h"

#if ENABLE(JIT)

namespace JSC { namespace DOMJIT {

class Value {
public:
    Value(Reg reg)
        : m_reg(reg)
    {
    }

    Value(Reg reg, JSValue value)
        : m_reg(reg)
        , m_value(value)
    {
    }

    bool isGPR() const { return m_reg.isGPR(); }
    bool isFPR() const { return m_reg.isFPR(); }
    bool isJSValueRegs() const { return m_reg.isJSValueRegs(); }
    GPRReg gpr() const { return m_reg.gpr(); }
    FPRReg fpr() const { return m_reg.fpr(); }
    JSValueRegs jsValueRegs() const { return m_reg.jsValueRegs(); }

    Reg reg() const
    {
        return m_reg;
    }

    JSValue value() const
    {
        return m_value;
    }

private:
    Reg m_reg;
    JSValue m_value;
};

} }

#endif
