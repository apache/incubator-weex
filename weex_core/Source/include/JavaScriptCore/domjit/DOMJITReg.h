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

#include "Reg.h"
#include <wtf/Variant.h>

#if ENABLE(JIT)

namespace JSC { namespace DOMJIT {

// It is quite unfortunate that 32 bit environment exists on DFG! This means that JSValueRegs contains 2 registers
// in such an environment. If we use GPRReg and FPRReg in DOMJITPatchpointParams, DOMJITPatchpointParams may contain
// different number of registers in 32bit and 64bit environments when we pass JSValueRegs, it is confusing.
// Therefore, we introduce an abstraction that DOMJIT::Reg, which is a polymorphic register class. It can refer FPRReg,
// GPRReg, and "JSValueRegs". Note that isGPR() will return false if the target Reg is "JSValueRegs" even if the
// environment is 64bit.
//
// FIXME: Eventually we should move this class into JSC and make is available for other JIT code.
// https://bugs.webkit.org/show_bug.cgi?id=162990
class Reg {
public:
    enum class Type : uint8_t {
        GPR = 0,
        FPR = 1,
        JSValue = 2,
    };

    Reg(GPRReg reg)
        : m_variant(reg)
    {
    }

    Reg(FPRReg reg)
        : m_variant(reg)
    {
    }

    Reg(JSValueRegs regs)
        : m_variant(regs)
    {
    }

    bool isGPR() const { return m_variant.index() == static_cast<unsigned>(Type::GPR); }
    bool isFPR() const { return m_variant.index() == static_cast<unsigned>(Type::FPR); }
    bool isJSValueRegs() const { return m_variant.index() == static_cast<unsigned>(Type::JSValue); }

    GPRReg gpr() const
    {
        ASSERT(isGPR());
        return WTF::get<GPRReg>(m_variant);
    }
    FPRReg fpr() const
    {
        ASSERT(isFPR());
        return WTF::get<FPRReg>(m_variant);
    }
    JSValueRegs jsValueRegs() const
    {
        ASSERT(isJSValueRegs());
        return WTF::get<JSValueRegs>(m_variant);
    }

private:
    Variant<GPRReg, FPRReg, JSValueRegs> m_variant;
};

} }

#endif
