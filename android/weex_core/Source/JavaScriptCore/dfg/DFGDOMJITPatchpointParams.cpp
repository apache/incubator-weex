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

#include "config.h"
#include "DFGDOMJITPatchpointParams.h"

#if ENABLE(DFG_JIT)

#include "DFGSlowPathGenerator.h"
#include "DFGSpeculativeJIT.h"

namespace JSC { namespace DFG {

template<typename OperationType, typename ResultType, typename Arguments, size_t... ArgumentsIndex>
static void dispatch(SpeculativeJIT* jit, CCallHelpers::JumpList from, OperationType operation, ResultType result, Arguments arguments, std::index_sequence<ArgumentsIndex...>)
{
    jit->addSlowPathGenerator(slowPathCall(from, jit, operation, result, std::get<ArgumentsIndex>(arguments)...));
}

#define JSC_DEFINE_CALL_OPERATIONS(OperationType, ResultType, ...) \
    void DOMJITPatchpointParams::addSlowPathCallImpl(CCallHelpers::JumpList from, CCallHelpers&, OperationType operation, ResultType result, std::tuple<__VA_ARGS__> args) \
    { \
        dispatch(m_jit, from, operation, result, args, std::make_index_sequence<std::tuple_size<decltype(args)>::value>()); \
    } \

DOMJIT_SLOW_PATH_CALLS(JSC_DEFINE_CALL_OPERATIONS)
#undef JSC_DEFINE_CALL_OPERATIONS

} }

#endif
