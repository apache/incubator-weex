/*
 * Copyright (C) 2016 Apple Inc. All rights reserved.
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
