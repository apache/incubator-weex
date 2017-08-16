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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Debugger.h"
#include "JSGlobalObject.h"
#include <wtf/Optional.h>

namespace JSC {

class ScriptProfilingScope {
public:
    ScriptProfilingScope(JSGlobalObject* globalObject, ProfilingReason reason)
        : m_globalObject(globalObject)
        , m_reason(reason)
    {
        if (shouldStartProfile())
            m_startTime = m_globalObject->debugger()->willEvaluateScript();
    }

    ~ScriptProfilingScope()
    {
        if (shouldEndProfile())
            m_globalObject->debugger()->didEvaluateScript(m_startTime.value(), m_reason);
    }

private:
    bool shouldStartProfile() const
    {
        if (!m_globalObject)
            return false;

        if (!m_globalObject->hasDebugger())
            return false;

        if (!m_globalObject->debugger()->hasProfilingClient())
            return false;

        if (m_globalObject->debugger()->isAlreadyProfiling())
            return false;

        return true;
    }

    bool shouldEndProfile() const
    {
        // Did not start a profile.
        if (!m_startTime)
            return false;

        // Debugger may have been removed.
        if (!m_globalObject->hasDebugger())
            return false;

        // Profiling Client may have been removed.
        if (!m_globalObject->debugger()->hasProfilingClient())
            return false;

        return true;
    }

    JSGlobalObject* m_globalObject { nullptr };
    std::optional<double> m_startTime;
    ProfilingReason m_reason;
};

} // namespace JSC
