/*
 * Copyright (C) 2014, 2015 Apple Inc. All rights reserved.
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

#include "InspectorAgentRegistry.h"
#include "InspectorEnvironment.h"
#include "InspectorFrontendRouter.h"
#include "JSGlobalObjectScriptDebugServer.h"
#include <wtf/Forward.h>
#include <wtf/Noncopyable.h>
#include <wtf/text/WTFString.h>

#if ENABLE(INSPECTOR_ALTERNATE_DISPATCHERS)
#include "AugmentableInspectorController.h"
#endif

namespace JSC {
class ConsoleClient;
class Exception;
class ExecState;
class JSGlobalObject;
}

namespace Inspector {

class BackendDispatcher;
class FrontendChannel;
class InjectedScriptManager;
class InspectorAgent;
class InspectorConsoleAgent;
class InspectorDebuggerAgent;
class InspectorScriptProfilerAgent;
class JSGlobalObjectConsoleClient;
class ScriptCallStack;

class JSGlobalObjectInspectorController final
    : public InspectorEnvironment
#if ENABLE(INSPECTOR_ALTERNATE_DISPATCHERS)
    , public AugmentableInspectorController
#endif
{
    WTF_MAKE_NONCOPYABLE(JSGlobalObjectInspectorController);
    WTF_MAKE_FAST_ALLOCATED;
public:
    JSGlobalObjectInspectorController(JSC::JSGlobalObject&);
    ~JSGlobalObjectInspectorController();

    void connectFrontend(FrontendChannel*, bool isAutomaticInspection);
    void disconnectFrontend(FrontendChannel*);

    void dispatchMessageFromFrontend(const String&);

    void globalObjectDestroyed();

    bool includesNativeCallStackWhenReportingExceptions() const { return m_includeNativeCallStackWithExceptions; }
    void setIncludesNativeCallStackWhenReportingExceptions(bool includesNativeCallStack) { m_includeNativeCallStackWithExceptions = includesNativeCallStack; }

    void pause();
    void reportAPIException(JSC::ExecState*, JSC::Exception*);

    JSC::ConsoleClient* consoleClient() const;

    bool developerExtrasEnabled() const override;
    bool canAccessInspectedScriptState(JSC::ExecState*) const override { return true; }
    InspectorFunctionCallHandler functionCallHandler() const override;
    InspectorEvaluateHandler evaluateHandler() const override;
    void frontendInitialized() override;
    Ref<WTF::Stopwatch> executionStopwatch() override;
    JSGlobalObjectScriptDebugServer& scriptDebugServer() override;
    JSC::VM& vm() override;

#if ENABLE(INSPECTOR_ALTERNATE_DISPATCHERS)
    AugmentableInspectorControllerClient* augmentableInspectorControllerClient() const override { return m_augmentingClient; } 
    void setAugmentableInspectorControllerClient(AugmentableInspectorControllerClient* client) override { m_augmentingClient = client; }

    const FrontendRouter& frontendRouter() const override { return m_frontendRouter.get(); }
    BackendDispatcher& backendDispatcher() override { return m_backendDispatcher.get(); }
    void appendExtraAgent(std::unique_ptr<InspectorAgentBase>) override;
#endif

private:
    void appendAPIBacktrace(ScriptCallStack&);

    JSC::JSGlobalObject& m_globalObject;
    std::unique_ptr<InjectedScriptManager> m_injectedScriptManager;
    std::unique_ptr<JSGlobalObjectConsoleClient> m_consoleClient;
    Ref<WTF::Stopwatch> m_executionStopwatch;
    JSGlobalObjectScriptDebugServer m_scriptDebugServer;

    AgentRegistry m_agents;
    InspectorAgent* m_inspectorAgent { nullptr };
    InspectorConsoleAgent* m_consoleAgent { nullptr };
    InspectorDebuggerAgent* m_debuggerAgent { nullptr };

    Ref<FrontendRouter> m_frontendRouter;
    Ref<BackendDispatcher> m_backendDispatcher;

    // Used to keep the JSGlobalObject and VM alive while we are debugging it.
    JSC::Strong<JSC::JSGlobalObject> m_strongGlobalObject;
    RefPtr<JSC::VM> m_strongVM;

    bool m_includeNativeCallStackWithExceptions { true };
    bool m_isAutomaticInspection { false };

#if ENABLE(INSPECTOR_ALTERNATE_DISPATCHERS)
    AugmentableInspectorControllerClient* m_augmentingClient { nullptr };
#endif
};

} // namespace Inspector
