/*
 * Copyright (C) 2013, 2015 Apple Inc. All rights reserved.
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

#if ENABLE(DFG_JIT)

#include "CodeOrigin.h"
#include "DFGExitProfile.h"

namespace JSC { namespace DFG {

struct BasicBlock;
struct Node;

// Provides for the OSR exit profiling functionality that is common between the DFG
// and the FTL.

struct OSRExitBase {
    OSRExitBase(ExitKind kind, CodeOrigin origin, CodeOrigin originForProfile, bool wasHoisted)
        : m_kind(kind)
        , m_wasHoisted(wasHoisted)
        , m_codeOrigin(origin)
        , m_codeOriginForExitProfile(originForProfile)
    {
        ASSERT(m_codeOrigin.isSet());
        ASSERT(m_codeOriginForExitProfile.isSet());
    }

    uint32_t m_count { 0 };
    ExitKind m_kind;
    bool m_wasHoisted;
    
    CodeOrigin m_codeOrigin;
    CodeOrigin m_codeOriginForExitProfile;
    CallSiteIndex m_exceptionHandlerCallSiteIndex;

    ALWAYS_INLINE bool isExceptionHandler() const
    {
        return m_kind == ExceptionCheck || m_kind == GenericUnwind;
    }

    // True if this exit is used as an exception handler for unwinding. This happens to only be set when
    // isExceptionHandler is true, but all this actually means is that the OSR exit will assume that the
    // machine state is as it would be coming out of genericUnwind.
    ALWAYS_INLINE bool isGenericUnwindHandler() const
    {
        return m_kind == GenericUnwind;
    }

protected:
    void considerAddingAsFrequentExitSite(CodeBlock* profiledCodeBlock, ExitingJITType jitType)
    {
        if (m_count)
            considerAddingAsFrequentExitSiteSlow(profiledCodeBlock, jitType);
    }

private:
    void considerAddingAsFrequentExitSiteSlow(CodeBlock* profiledCodeBlock, ExitingJITType);
};

} } // namespace JSC::DFG

#endif // ENABLE(DFG_JIT)
