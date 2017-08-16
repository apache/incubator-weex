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
#include "DOMJITAccessCasePatchpointParams.h"

#include "LinkBuffer.h"
#include "PolymorphicAccess.h"
#include "StructureStubInfo.h"

#if ENABLE(JIT)

namespace JSC {

template<typename JumpType, typename FunctionType, typename ResultType, typename... Arguments>
class SlowPathCallGeneratorWithArguments : public DOMJITAccessCasePatchpointParams::SlowPathCallGenerator {
public:
    SlowPathCallGeneratorWithArguments(JumpType from, CCallHelpers::Label to, FunctionType function, ResultType result, std::tuple<Arguments...> arguments)
        : m_from(from)
        , m_to(to)
        , m_function(function)
        , m_result(result)
        , m_arguments(arguments)
    {
    }

    template<size_t... ArgumentsIndex>
    CCallHelpers::JumpList generateImpl(AccessGenerationState& state, const RegisterSet& usedRegistersByPatchpoint, CCallHelpers& jit, std::index_sequence<ArgumentsIndex...>)
    {
        CCallHelpers::JumpList exceptions;
        // We spill (1) the used registers by IC and (2) the used registers by DOMJIT::Patchpoint.
        AccessGenerationState::SpillState spillState = state.preserveLiveRegistersToStackForCall(usedRegistersByPatchpoint);

        jit.store32(
            CCallHelpers::TrustedImm32(state.callSiteIndexForExceptionHandlingOrOriginal().bits()),
            CCallHelpers::tagFor(static_cast<VirtualRegister>(CallFrameSlot::argumentCount)));

        jit.makeSpaceOnStackForCCall();

        // FIXME: Currently, we do not check any ARM EABI things here.
        // But it is OK because a compile error happens when you pass JSValueRegs as an argument.
        // https://bugs.webkit.org/show_bug.cgi?id=163099
        jit.setupArgumentsWithExecState(std::get<ArgumentsIndex>(m_arguments)...);

        CCallHelpers::Call operationCall = jit.call();
        auto function = m_function;
        jit.addLinkTask([=] (LinkBuffer& linkBuffer) {
            linkBuffer.link(operationCall, FunctionPtr(function));
        });

        jit.setupResults(m_result);
        jit.reclaimSpaceOnStackForCCall();

        CCallHelpers::Jump noException = jit.emitExceptionCheck(CCallHelpers::InvertedExceptionCheck);

        state.restoreLiveRegistersFromStackForCallWithThrownException(spillState);
        exceptions.append(jit.jump());

        noException.link(&jit);
        RegisterSet dontRestore;
        dontRestore.set(m_result);
        state.restoreLiveRegistersFromStackForCall(spillState, dontRestore);

        return exceptions;
    }

    CCallHelpers::JumpList generate(AccessGenerationState& state, const RegisterSet& usedRegistersByPatchpoint, CCallHelpers& jit) override
    {
        m_from.link(&jit);
        CCallHelpers::JumpList exceptions = generateImpl(state, usedRegistersByPatchpoint, jit, std::make_index_sequence<std::tuple_size<std::tuple<Arguments...>>::value>());
        jit.jump().linkTo(m_to, &jit);
        return exceptions;
    }

protected:
    JumpType m_from;
    CCallHelpers::Label m_to;
    FunctionType m_function;
    ResultType m_result;
    std::tuple<Arguments...> m_arguments;
};

#define JSC_DEFINE_CALL_OPERATIONS(OperationType, ResultType, ...) \
    void DOMJITAccessCasePatchpointParams::addSlowPathCallImpl(CCallHelpers::JumpList from, CCallHelpers& jit, OperationType operation, ResultType result, std::tuple<__VA_ARGS__> args) \
    { \
        CCallHelpers::Label to = jit.label(); \
        m_generators.append(std::make_unique<SlowPathCallGeneratorWithArguments<CCallHelpers::JumpList, OperationType, ResultType, __VA_ARGS__>>(from, to, operation, result, args)); \
    } \

DOMJIT_SLOW_PATH_CALLS(JSC_DEFINE_CALL_OPERATIONS)
#undef JSC_DEFINE_CALL_OPERATIONS

CCallHelpers::JumpList DOMJITAccessCasePatchpointParams::emitSlowPathCalls(AccessGenerationState& state, const RegisterSet& usedRegistersByPatchpoint, CCallHelpers& jit)
{
    CCallHelpers::JumpList exceptions;
    for (auto& generator : m_generators)
        exceptions.append(generator->generate(state, usedRegistersByPatchpoint, jit));
    return exceptions;
}

}

#endif
