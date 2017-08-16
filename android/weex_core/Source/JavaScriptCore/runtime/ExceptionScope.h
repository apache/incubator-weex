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

#include "VM.h"

namespace JSC {
    
class Exception;
    
#if ENABLE(EXCEPTION_SCOPE_VERIFICATION)
    
class ExceptionScope {
public:
    VM& vm() const { return m_vm; }
    unsigned recursionDepth() const { return m_recursionDepth; }
    Exception* exception() { return m_vm.exception(); }
    
protected:
    ExceptionScope(VM&, ExceptionEventLocation);
    ExceptionScope(const ExceptionScope&) = delete;
    ExceptionScope(ExceptionScope&&) = default;
    ~ExceptionScope();

    VM& m_vm;
    ExceptionScope* m_previousScope;
    ExceptionEventLocation m_location;
    unsigned m_recursionDepth;
};
    
#else // not ENABLE(EXCEPTION_SCOPE_VERIFICATION)
    
class ExceptionScope {
public:
    ALWAYS_INLINE VM& vm() const { return m_vm; }
    ALWAYS_INLINE Exception* exception() { return m_vm.exception(); }

protected:
    ALWAYS_INLINE ExceptionScope(VM& vm)
        : m_vm(vm)
    { }
    ExceptionScope(const ExceptionScope&) = delete;
    ExceptionScope(ExceptionScope&&) = default;

    VM& m_vm;
};
    
#endif // ENABLE(EXCEPTION_SCOPE_VERIFICATION)

#define RETURN_IF_EXCEPTION(scope__, value__) do { \
        if (UNLIKELY((scope__).exception())) \
            return value__; \
    } while (false)

} // namespace JSC
