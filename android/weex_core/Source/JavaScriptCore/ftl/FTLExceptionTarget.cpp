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
#include "FTLExceptionTarget.h"

#if ENABLE(FTL_JIT)

#include "FTLState.h"

namespace JSC { namespace FTL {

ExceptionTarget::~ExceptionTarget()
{
}

CodeLocationLabel ExceptionTarget::label(LinkBuffer& linkBuffer)
{
    if (m_isDefaultHandler)
        return linkBuffer.locationOf(*m_defaultHandler);
    return linkBuffer.locationOf(m_handle->label);
}

Box<CCallHelpers::JumpList> ExceptionTarget::jumps(CCallHelpers& jit)
{
    Box<CCallHelpers::JumpList> result = Box<CCallHelpers::JumpList>::create();
    if (m_isDefaultHandler) {
        Box<CCallHelpers::Label> defaultHandler = m_defaultHandler;
        jit.addLinkTask(
            [=] (LinkBuffer& linkBuffer) {
                linkBuffer.link(*result, linkBuffer.locationOf(*defaultHandler));
            });
    } else {
        RefPtr<OSRExitHandle> handle = m_handle;
        jit.addLinkTask(
            [=] (LinkBuffer& linkBuffer) {
                linkBuffer.link(*result, linkBuffer.locationOf(handle->label));
            });
    }
    return result;
}

ExceptionTarget::ExceptionTarget(
    bool isDefaultHandler, Box<CCallHelpers::Label> defaultHandler, RefPtr<OSRExitHandle> handle)
    : m_isDefaultHandler(isDefaultHandler)
    , m_defaultHandler(defaultHandler)
    , m_handle(handle)
{
}

} } // namespace JSC::FTL

#endif // ENABLE(FTL_JIT)

