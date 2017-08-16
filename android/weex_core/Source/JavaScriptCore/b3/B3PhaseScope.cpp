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
#include "B3PhaseScope.h"

#if ENABLE(B3_JIT)

#include "B3Common.h"
#include "B3Procedure.h"
#include "B3Validate.h"
#include <wtf/DataLog.h>
#include <wtf/StringPrintStream.h>

namespace JSC { namespace B3 {

PhaseScope::PhaseScope(Procedure& procedure, const char* name)
    : m_procedure(procedure)
    , m_name(name)
    , m_timingScope(name)
{
    if (shouldDumpIRAtEachPhase(B3Mode)) {
        dataLog("B3 after ", procedure.lastPhaseName(), ", before ", name, ":\n");
        dataLog(procedure);
    }

    if (shouldSaveIRBeforePhase())
        m_dumpBefore = toCString(procedure);
}

PhaseScope::~PhaseScope()
{
    m_procedure.setLastPhaseName(m_name);
    if (shouldValidateIRAtEachPhase())
        validate(m_procedure, m_dumpBefore.data());
}

} } // namespace JSC::B3

#endif // ENABLE(B3_JIT)
