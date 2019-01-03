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
