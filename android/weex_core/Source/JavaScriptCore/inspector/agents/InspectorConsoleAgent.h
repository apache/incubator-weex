/*
* Copyright (C) 2014, 2015 Apple Inc. All rights reserved.
* Copyright (C) 2011 Google Inc. All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions
* are met:
* 1.  Redistributions of source code must retain the above copyright
*     notice, this list of conditions and the following disclaimer.
* 2.  Redistributions in binary form must reproduce the above copyright
*     notice, this list of conditions and the following disclaimer in the
*     documentation and/or other materials provided with the distribution.
*
* THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS'' AND ANY
* EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS BE LIABLE FOR ANY
* DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
* ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#pragma once

#include "InspectorBackendDispatchers.h"
#include "InspectorFrontendDispatchers.h"
#include "inspector/InspectorAgentBase.h"
#include "runtime/ConsoleTypes.h"
#include <wtf/Forward.h>
#include <wtf/HashMap.h>
#include <wtf/Noncopyable.h>
#include <wtf/Vector.h>
#include <wtf/text/StringHash.h>

namespace JSC {
class ExecState;
}

namespace Inspector {

class ConsoleMessage;
class InjectedScriptManager;
class InspectorHeapAgent;
class ScriptArguments;
class ScriptCallStack;
typedef String ErrorString;

class JS_EXPORT_PRIVATE InspectorConsoleAgent : public InspectorAgentBase, public ConsoleBackendDispatcherHandler {
    WTF_MAKE_NONCOPYABLE(InspectorConsoleAgent);
    WTF_MAKE_FAST_ALLOCATED;
public:
    InspectorConsoleAgent(AgentContext&, InspectorHeapAgent*);
    virtual ~InspectorConsoleAgent();

    void didCreateFrontendAndBackend(FrontendRouter*, BackendDispatcher*) override;
    void willDestroyFrontendAndBackend(DisconnectReason) override;
    void discardValues() override;

    void enable(ErrorString&) override;
    void disable(ErrorString&) override;
    void clearMessages(ErrorString&) override;
    void setMonitoringXHREnabled(ErrorString&, bool enabled) override = 0;
    void addInspectedNode(ErrorString&, int nodeId) override = 0;

    bool enabled() const { return m_enabled; }
    void reset();

    void addMessageToConsole(std::unique_ptr<ConsoleMessage>);

    void startTiming(const String& title);
    void stopTiming(const String& title, Ref<ScriptCallStack>&&);
    void takeHeapSnapshot(const String& title);
    void count(JSC::ExecState*, Ref<ScriptArguments>&&);

protected:
    void addConsoleMessage(std::unique_ptr<ConsoleMessage>);

    InjectedScriptManager& m_injectedScriptManager;
    std::unique_ptr<ConsoleFrontendDispatcher> m_frontendDispatcher;
    RefPtr<ConsoleBackendDispatcher> m_backendDispatcher;
    InspectorHeapAgent* m_heapAgent;

    ConsoleMessage* m_previousMessage { nullptr };
    Vector<std::unique_ptr<ConsoleMessage>> m_consoleMessages;
    int m_expiredConsoleMessageCount { 0 };
    HashMap<String, unsigned> m_counts;
    HashMap<String, double> m_times;
    bool m_enabled { false };
};

} // namespace Inspector
