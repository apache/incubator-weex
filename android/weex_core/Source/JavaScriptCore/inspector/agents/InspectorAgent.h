/*
 * Copyright (C) 2007-2010, 2015 Apple Inc. All rights reserved.
 * Copyright (C) 2011 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 * 3.  Neither the name of Apple Inc. ("Apple") nor the names of
 *     its contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "InspectorBackendDispatchers.h"
#include "InspectorFrontendDispatchers.h"
#include "inspector/InspectorAgentBase.h"
#include <wtf/Forward.h>
#include <wtf/Vector.h>

namespace Inspector {

class BackendDispatcher;
class InspectorEnvironment;
class InspectorObject;

typedef String ErrorString;

class JS_EXPORT_PRIVATE InspectorAgent final : public InspectorAgentBase, public InspectorBackendDispatcherHandler {
    WTF_MAKE_NONCOPYABLE(InspectorAgent);
    WTF_MAKE_FAST_ALLOCATED;
public:
    InspectorAgent(AgentContext&);
    virtual ~InspectorAgent();

    void didCreateFrontendAndBackend(FrontendRouter*, BackendDispatcher*) override;
    void willDestroyFrontendAndBackend(DisconnectReason) override;

    void enable(ErrorString&) override;
    void disable(ErrorString&) override;
    void initialized(ErrorString&) override;

    void inspect(RefPtr<Protocol::Runtime::RemoteObject>&& objectToInspect, RefPtr<InspectorObject>&& hints);
    void evaluateForTestInFrontend(const String& script);

#if ENABLE(INSPECTOR_ALTERNATE_DISPATCHERS)
    void activateExtraDomain(const String&);
    void activateExtraDomains(const Vector<String>&);
#endif

private:
    InspectorEnvironment& m_environment;
    std::unique_ptr<InspectorFrontendDispatcher> m_frontendDispatcher;
    Ref<InspectorBackendDispatcher> m_backendDispatcher;

    Vector<String> m_pendingEvaluateTestCommands;
    std::pair<RefPtr<Protocol::Runtime::RemoteObject>, RefPtr<InspectorObject>> m_pendingInspectData;
#if ENABLE(INSPECTOR_ALTERNATE_DISPATCHERS)
    RefPtr<Inspector::Protocol::Array<String>> m_pendingExtraDomainsData;
#endif
    bool m_enabled { false };
};

} // namespace Inspector
