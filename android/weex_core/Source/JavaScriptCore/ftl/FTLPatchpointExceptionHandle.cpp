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
#include "FTLPatchpointExceptionHandle.h"

#if ENABLE(FTL_JIT)

#include "B3StackmapGenerationParams.h"
#include "FTLExceptionTarget.h"
#include "FTLOSRExit.h"
#include "FTLOSRExitHandle.h"
#include "FTLState.h"

namespace JSC { namespace FTL {

using namespace DFG;

RefPtr<PatchpointExceptionHandle> PatchpointExceptionHandle::create(
    State& state, OSRExitDescriptor* descriptor, NodeOrigin origin, unsigned offset,
    const HandlerInfo& handler)
{
    return adoptRef(new PatchpointExceptionHandle(state, descriptor, origin, offset, handler));
}

RefPtr<PatchpointExceptionHandle> PatchpointExceptionHandle::defaultHandle(State& state)
{
    if (!state.defaultExceptionHandle) {
        state.defaultExceptionHandle = adoptRef(
            new PatchpointExceptionHandle(state, nullptr, NodeOrigin(), 0, HandlerInfo()));
    }
    return state.defaultExceptionHandle;
}

PatchpointExceptionHandle::~PatchpointExceptionHandle()
{
}

RefPtr<ExceptionTarget> PatchpointExceptionHandle::scheduleExitCreation(
    const B3::StackmapGenerationParams& params)
{
    if (!m_descriptor) {
        // NOTE: This object could be a singleton, however usually we toss the ExceptionHandler
        // object shortly after creation.
        bool isDefaultHandler = true;
        return adoptRef(
            new ExceptionTarget(isDefaultHandler, m_state.exceptionHandler, nullptr));
    }
    bool isDefaultHandler = false;
    return adoptRef(new ExceptionTarget(isDefaultHandler, { }, createHandle(ExceptionCheck, params)));
}

void PatchpointExceptionHandle::scheduleExitCreationForUnwind(
    const B3::StackmapGenerationParams& params, CallSiteIndex callSiteIndex)
{
    if (!m_descriptor)
        return;
    
    RefPtr<OSRExitHandle> handle = createHandle(GenericUnwind, params);

    handle->exit.m_exceptionHandlerCallSiteIndex = callSiteIndex;

    HandlerInfo handler = m_handler;
    params.addLatePath(
        [handle, handler, callSiteIndex] (CCallHelpers& jit) {
            CodeBlock* codeBlock = jit.codeBlock();
            jit.addLinkTask(
                [=] (LinkBuffer& linkBuffer) {
                    HandlerInfo newHandler = handler;
                    newHandler.start = callSiteIndex.bits();
                    newHandler.end = callSiteIndex.bits() + 1;
                    newHandler.nativeCode = linkBuffer.locationOf(handle->label);
                    codeBlock->appendExceptionHandler(newHandler);
                });
        });
}

PatchpointExceptionHandle::PatchpointExceptionHandle(
    State& state, OSRExitDescriptor* descriptor, NodeOrigin origin, unsigned offset,
    const HandlerInfo& handler)
    : m_state(state)
    , m_descriptor(descriptor)
    , m_origin(origin)
    , m_offset(offset)
    , m_handler(handler)
{
}

RefPtr<OSRExitHandle> PatchpointExceptionHandle::createHandle(
    ExitKind kind, const B3::StackmapGenerationParams& params)
{
    return m_descriptor->emitOSRExitLater(
        m_state, kind, m_origin, params, m_offset);
}

} } // namespace JSC::FTL

#endif // ENABLE(FTL_JIT)

