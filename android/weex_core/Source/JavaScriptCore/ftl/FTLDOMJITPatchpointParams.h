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

#if ENABLE(FTL_JIT)

#include "B3StackmapGenerationParams.h"
#include "DOMJITPatchpointParams.h"

namespace JSC { namespace FTL {

class State;

class DOMJITPatchpointParams : public DOMJIT::PatchpointParams {
public:
    DOMJITPatchpointParams(State& state, const B3::StackmapGenerationParams& params, DFG::Node* node, Box<CCallHelpers::JumpList> exceptions, Vector<DOMJIT::Value>&& regs, Vector<GPRReg>&& gpScratch, Vector<FPRReg>&& fpScratch)
        : DOMJIT::PatchpointParams(WTFMove(regs), WTFMove(gpScratch), WTFMove(fpScratch))
        , m_state(state)
        , m_params(params)
        , m_node(node)
        , m_exceptions(exceptions)
    {
    }

private:
#define JSC_DEFINE_CALL_OPERATIONS(OperationType, ResultType, ...) void addSlowPathCallImpl(CCallHelpers::JumpList, CCallHelpers&, OperationType, ResultType, std::tuple<__VA_ARGS__> args) override;
    DOMJIT_SLOW_PATH_CALLS(JSC_DEFINE_CALL_OPERATIONS)
#undef JSC_DEFINE_CALL_OPERATIONS

    State& m_state;
    const B3::StackmapGenerationParams& m_params;
    DFG::Node* m_node;
    Box<CCallHelpers::JumpList> m_exceptions;
};

} }

#endif
