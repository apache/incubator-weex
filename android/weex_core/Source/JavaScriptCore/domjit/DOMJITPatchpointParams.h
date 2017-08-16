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

#if ENABLE(JIT)

#include "CCallHelpers.h"
#include "DOMJITSlowPathCalls.h"
#include "DOMJITValue.h"
#include "JITOperations.h"
#include "RegisterSet.h"

namespace JSC { namespace DOMJIT {

class PatchpointParams {
WTF_MAKE_NONCOPYABLE(PatchpointParams);
public:
    virtual ~PatchpointParams() { }

    unsigned size() const { return m_regs.size(); }
    const Value& at(unsigned index) const { return m_regs[index]; }
    const Value& operator[](unsigned index) const { return at(index); }

    GPRReg gpScratch(unsigned index) const { return m_gpScratch[index]; }
    FPRReg fpScratch(unsigned index) const { return m_fpScratch[index]; }

    PatchpointParams(Vector<Value>&& regs, Vector<GPRReg>&& gpScratch, Vector<FPRReg>&& fpScratch)
        : m_regs(WTFMove(regs))
        , m_gpScratch(WTFMove(gpScratch))
        , m_fpScratch(WTFMove(fpScratch))
    {
    }

    template<typename FunctionType, typename ResultType, typename... Arguments>
    void addSlowPathCall(CCallHelpers::JumpList from, CCallHelpers& jit, FunctionType function, ResultType result, Arguments... arguments)
    {
        addSlowPathCallImpl(from, jit, function, result, std::make_tuple(arguments...));
    }

private:
#define JSC_DEFINE_CALL_OPERATIONS(OperationType, ResultType, ...) JS_EXPORT_PRIVATE virtual void addSlowPathCallImpl(CCallHelpers::JumpList, CCallHelpers&, OperationType, ResultType, std::tuple<__VA_ARGS__> args) = 0;
    DOMJIT_SLOW_PATH_CALLS(JSC_DEFINE_CALL_OPERATIONS)
#undef JSC_DEFINE_CALL_OPERATIONS

    Vector<Value> m_regs;
    Vector<GPRReg> m_gpScratch;
    Vector<FPRReg> m_fpScratch;
};

} }

#endif
