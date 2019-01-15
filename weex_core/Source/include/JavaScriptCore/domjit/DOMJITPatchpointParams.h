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
