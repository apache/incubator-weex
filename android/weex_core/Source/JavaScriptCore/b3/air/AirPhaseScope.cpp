/*
 * Copyright (C) 2015 Apple Inc. All rights reserved.
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
#include "AirPhaseScope.h"

#if ENABLE(B3_JIT)

#include "AirCode.h"
#include "AirValidate.h"
#include "B3Common.h"

namespace JSC { namespace B3 { namespace Air {

PhaseScope::PhaseScope(Code& code, const char* name)
    : m_code(code)
    , m_name(name)
    , m_timingScope(name)
{
    if (shouldDumpIRAtEachPhase(AirMode)) {
        dataLog("Air after ", code.lastPhaseName(), ", before ", name, ":\n");
        dataLog(code);
    }

    if (shouldSaveIRBeforePhase())
        m_dumpBefore = toCString(code);
}

PhaseScope::~PhaseScope()
{
    m_code.setLastPhaseName(m_name);
    if (shouldValidateIRAtEachPhase())
        validate(m_code, m_dumpBefore.data());
}

} } } // namespace JSC::B3::Air

#endif // ENABLE(B3_JIT)
