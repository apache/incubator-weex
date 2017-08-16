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
#include "ExceptionScope.h"

#include "Exception.h"

namespace JSC {
    
#if ENABLE(EXCEPTION_SCOPE_VERIFICATION)
    
ExceptionScope::ExceptionScope(VM& vm, ExceptionEventLocation location)
    : m_vm(vm)
    , m_previousScope(vm.m_topExceptionScope)
    , m_location(location)
    , m_recursionDepth(m_previousScope ? m_previousScope->m_recursionDepth + 1 : 0)
{
    m_vm.m_topExceptionScope = this;
}

ExceptionScope::~ExceptionScope()
{
    RELEASE_ASSERT(m_vm.m_topExceptionScope);
    m_vm.m_topExceptionScope = m_previousScope;
}

#endif // ENABLE(EXCEPTION_SCOPE_VERIFICATION)
    
} // namespace JSC
