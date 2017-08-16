/*
 * Copyright (C) 2015-2016 Apple Inc. All rights reserved.
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

#include "HeapSnapshot.h"
#include "InspectorBackendDispatchers.h"
#include "InspectorFrontendDispatchers.h"
#include "heap/HeapObserver.h"
#include "inspector/InspectorAgentBase.h"
#include <wtf/Forward.h>
#include <wtf/Noncopyable.h>

namespace Inspector {

class InjectedScriptManager;
typedef String ErrorString;

class JS_EXPORT_PRIVATE InspectorHeapAgent : public InspectorAgentBase, public HeapBackendDispatcherHandler, public JSC::HeapObserver {
    WTF_MAKE_NONCOPYABLE(InspectorHeapAgent);
public:
    InspectorHeapAgent(AgentContext&);
    virtual ~InspectorHeapAgent();

    void didCreateFrontendAndBackend(FrontendRouter*, BackendDispatcher*) override;
    void willDestroyFrontendAndBackend(DisconnectReason) override;

    // HeapBackendDispatcherHandler
    void enable(ErrorString&) override;
    void disable(ErrorString&) override;
    void gc(ErrorString&) final;
    void snapshot(ErrorString&, double* timestamp, String* snapshotData) final;
    void startTracking(ErrorString&) final;
    void stopTracking(ErrorString&) final;
    void getPreview(ErrorString&, int heapObjectId, Inspector::Protocol::OptOutput<String>* resultString, RefPtr<Inspector::Protocol::Debugger::FunctionDetails>& functionDetails, RefPtr<Inspector::Protocol::Runtime::ObjectPreview>& objectPreview) final;
    void getRemoteObject(ErrorString&, int heapObjectId, const String* const optionalObjectGroup, RefPtr<Inspector::Protocol::Runtime::RemoteObject>& result) final;

    // HeapObserver
    void willGarbageCollect() override;
    void didGarbageCollect(JSC::CollectionScope) override;

protected:
    void clearHeapSnapshots();

    virtual void dispatchGarbageCollectedEvent(Inspector::Protocol::Heap::GarbageCollection::Type, double startTime, double endTime);

private:
    std::optional<JSC::HeapSnapshotNode> nodeForHeapObjectIdentifier(ErrorString&, unsigned heapObjectIdentifier);

    InjectedScriptManager& m_injectedScriptManager;
    std::unique_ptr<HeapFrontendDispatcher> m_frontendDispatcher;
    RefPtr<HeapBackendDispatcher> m_backendDispatcher;
    InspectorEnvironment& m_environment;

    bool m_enabled { false };
    bool m_tracking { false };
    double m_gcStartTime { NAN };
};

} // namespace Inspector
