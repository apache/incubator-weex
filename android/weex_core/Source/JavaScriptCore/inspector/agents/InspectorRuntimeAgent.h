/*
 * Copyright (C) 2013, 2015-2016 Apple Inc. All rights reserved.
 * Copyright (C) 2011 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "InspectorBackendDispatchers.h"
#include "InspectorFrontendDispatchers.h"
#include "inspector/InspectorAgentBase.h"
#include <wtf/Forward.h>
#include <wtf/Noncopyable.h>

namespace JSC {
class VM;
}

namespace Inspector {

class InjectedScript;
class InjectedScriptManager;
class InspectorArray;
class ScriptDebugServer;
typedef String ErrorString;

class JS_EXPORT_PRIVATE InspectorRuntimeAgent : public InspectorAgentBase, public RuntimeBackendDispatcherHandler {
    WTF_MAKE_NONCOPYABLE(InspectorRuntimeAgent);
public:
    virtual ~InspectorRuntimeAgent();

    void willDestroyFrontendAndBackend(DisconnectReason) override;

    void enable(ErrorString&) override { m_enabled = true; }
    void disable(ErrorString&) override { m_enabled = false; }
    void parse(ErrorString&, const String& expression, Inspector::Protocol::Runtime::SyntaxErrorType* result, Inspector::Protocol::OptOutput<String>* message, RefPtr<Inspector::Protocol::Runtime::ErrorRange>&) final;
    void evaluate(ErrorString&, const String& expression, const String* const objectGroup, const bool* const includeCommandLineAPI, const bool* const doNotPauseOnExceptionsAndMuteConsole, const int* const executionContextId, const bool* const returnByValue, const bool* const generatePreview, const bool* const saveResult, RefPtr<Inspector::Protocol::Runtime::RemoteObject>& result, Inspector::Protocol::OptOutput<bool>* wasThrown, Inspector::Protocol::OptOutput<int>* savedResultIndex) final;
    void callFunctionOn(ErrorString&, const String& objectId, const String& expression, const Inspector::InspectorArray* optionalArguments, const bool* const doNotPauseOnExceptionsAndMuteConsole, const bool* const returnByValue, const bool* const generatePreview, RefPtr<Inspector::Protocol::Runtime::RemoteObject>& result, Inspector::Protocol::OptOutput<bool>* wasThrown) final;
    void releaseObject(ErrorString&, const ErrorString& objectId) final;
    void getProperties(ErrorString&, const String& objectId, const bool* const ownProperties, const bool* const generatePreview, RefPtr<Inspector::Protocol::Array<Inspector::Protocol::Runtime::PropertyDescriptor>>& result, RefPtr<Inspector::Protocol::Array<Inspector::Protocol::Runtime::InternalPropertyDescriptor>>& internalProperties) final;
    void getDisplayableProperties(ErrorString&, const String& objectId, const bool* const generatePreview, RefPtr<Inspector::Protocol::Array<Inspector::Protocol::Runtime::PropertyDescriptor>>& result, RefPtr<Inspector::Protocol::Array<Inspector::Protocol::Runtime::InternalPropertyDescriptor>>& internalProperties) final;
    void getCollectionEntries(ErrorString&, const String& objectId, const String* const objectGroup, const int* const startIndex, const int* const numberToFetch, RefPtr<Inspector::Protocol::Array<Inspector::Protocol::Runtime::CollectionEntry>>& entries) final;
    void saveResult(ErrorString&, const Inspector::InspectorObject& callArgument, const int* const executionContextId, Inspector::Protocol::OptOutput<int>* savedResultIndex) final;
    void releaseObjectGroup(ErrorString&, const String& objectGroup) final;
    void getRuntimeTypesForVariablesAtOffsets(ErrorString&, const Inspector::InspectorArray& locations, RefPtr<Inspector::Protocol::Array<Inspector::Protocol::Runtime::TypeDescription>>&) override;
    void enableTypeProfiler(ErrorString&) override;
    void disableTypeProfiler(ErrorString&) override;
    void enableControlFlowProfiler(ErrorString&) override;
    void disableControlFlowProfiler(ErrorString&) override;
    void getBasicBlocks(ErrorString&, const String& in_sourceID, RefPtr<Inspector::Protocol::Array<Inspector::Protocol::Runtime::BasicBlock>>& out_basicBlocks) override;

    bool enabled() const { return m_enabled; }

protected:
    InspectorRuntimeAgent(AgentContext&);

    InjectedScriptManager& injectedScriptManager() { return m_injectedScriptManager; }

    virtual InjectedScript injectedScriptForEval(ErrorString&, const int* executionContextId) = 0;

    virtual void muteConsole() = 0;
    virtual void unmuteConsole() = 0;

private:
    void setTypeProfilerEnabledState(bool);
    void setControlFlowProfilerEnabledState(bool);

    InjectedScriptManager& m_injectedScriptManager;
    ScriptDebugServer& m_scriptDebugServer;
    JSC::VM& m_vm;
    bool m_enabled {false};
    bool m_isTypeProfilingEnabled {false};
    bool m_isControlFlowProfilingEnabled {false};
};

} // namespace Inspector
