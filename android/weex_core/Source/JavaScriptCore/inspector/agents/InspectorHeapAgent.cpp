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

#include "config.h"
#include "InspectorHeapAgent.h"

#include "HeapProfiler.h"
#include "InjectedScript.h"
#include "InjectedScriptManager.h"
#include "InspectorEnvironment.h"
#include "JSCInlines.h"
#include "VM.h"
#include <wtf/Stopwatch.h>

using namespace JSC;

namespace Inspector {

InspectorHeapAgent::InspectorHeapAgent(AgentContext& context)
    : InspectorAgentBase(ASCIILiteral("Heap"))
    , m_injectedScriptManager(context.injectedScriptManager)
    , m_frontendDispatcher(std::make_unique<HeapFrontendDispatcher>(context.frontendRouter))
    , m_backendDispatcher(HeapBackendDispatcher::create(context.backendDispatcher, this))
    , m_environment(context.environment)
{
}

InspectorHeapAgent::~InspectorHeapAgent()
{
}

void InspectorHeapAgent::didCreateFrontendAndBackend(FrontendRouter*, BackendDispatcher*)
{
}

void InspectorHeapAgent::willDestroyFrontendAndBackend(DisconnectReason)
{
    // Stop tracking without taking a snapshot.
    m_tracking = false;

    ErrorString ignored;
    disable(ignored);
}

void InspectorHeapAgent::enable(ErrorString&)
{
    if (m_enabled)
        return;

    m_enabled = true;

    m_environment.vm().heap.addObserver(this);
}

void InspectorHeapAgent::disable(ErrorString&)
{
    if (!m_enabled)
        return;

    m_enabled = false;

    m_environment.vm().heap.removeObserver(this);

    clearHeapSnapshots();
}

void InspectorHeapAgent::gc(ErrorString&)
{
    VM& vm = m_environment.vm();
    JSLockHolder lock(vm);
    sanitizeStackForVM(&vm);
    vm.heap.collectAllGarbage();
}

void InspectorHeapAgent::snapshot(ErrorString&, double* timestamp, String* snapshotData)
{
    VM& vm = m_environment.vm();
    JSLockHolder lock(vm);

    HeapSnapshotBuilder snapshotBuilder(vm.ensureHeapProfiler());
    snapshotBuilder.buildSnapshot();

    *timestamp = m_environment.executionStopwatch()->elapsedTime();
    *snapshotData = snapshotBuilder.json([&] (const HeapSnapshotNode& node) {
        if (Structure* structure = node.cell->structure(vm)) {
            if (JSGlobalObject* globalObject = structure->globalObject()) {
                if (!m_environment.canAccessInspectedScriptState(globalObject->globalExec()))
                    return false;
            }
        }
        return true;
    });
}

void InspectorHeapAgent::startTracking(ErrorString& errorString)
{
    if (m_tracking)
        return;

    m_tracking = true;

    double timestamp;
    String snapshotData;
    snapshot(errorString, &timestamp, &snapshotData);

    m_frontendDispatcher->trackingStart(timestamp, snapshotData);
}

void InspectorHeapAgent::stopTracking(ErrorString& errorString)
{
    if (!m_tracking)
        return;

    m_tracking = false;

    double timestamp;
    String snapshotData;
    snapshot(errorString, &timestamp, &snapshotData);

    m_frontendDispatcher->trackingComplete(timestamp, snapshotData);
}

std::optional<HeapSnapshotNode> InspectorHeapAgent::nodeForHeapObjectIdentifier(ErrorString& errorString, unsigned heapObjectIdentifier)
{
    HeapProfiler* heapProfiler = m_environment.vm().heapProfiler();
    if (!heapProfiler) {
        errorString = ASCIILiteral("No heap snapshot");
        return std::nullopt;
    }

    HeapSnapshot* snapshot = heapProfiler->mostRecentSnapshot();
    if (!snapshot) {
        errorString = ASCIILiteral("No heap snapshot");
        return std::nullopt;
    }

    const std::optional<HeapSnapshotNode> optionalNode = snapshot->nodeForObjectIdentifier(heapObjectIdentifier);
    if (!optionalNode) {
        errorString = ASCIILiteral("No object for identifier, it may have been collected");
        return std::nullopt;
    }

    return optionalNode;
}

void InspectorHeapAgent::getPreview(ErrorString& errorString, int heapObjectId, Inspector::Protocol::OptOutput<String>* resultString, RefPtr<Inspector::Protocol::Debugger::FunctionDetails>& functionDetails, RefPtr<Inspector::Protocol::Runtime::ObjectPreview>& objectPreview)
{
    // Prevent the cell from getting collected as we look it up.
    VM& vm = m_environment.vm();
    JSLockHolder lock(vm);
    DeferGC deferGC(vm.heap);

    unsigned heapObjectIdentifier = static_cast<unsigned>(heapObjectId);
    const std::optional<HeapSnapshotNode> optionalNode = nodeForHeapObjectIdentifier(errorString, heapObjectIdentifier);
    if (!optionalNode)
        return;

    // String preview.
    JSCell* cell = optionalNode->cell;
    if (cell->isString()) {
        *resultString = asString(cell)->tryGetValue();
        return;
    }

    // FIXME: Provide preview information for Internal Objects? CodeBlock, Executable, etc.

    Structure* structure = cell->structure(vm);
    if (!structure) {
        errorString = ASCIILiteral("Unable to get object details - Structure");
        return;
    }

    JSGlobalObject* globalObject = structure->globalObject();
    if (!globalObject) {
        errorString = ASCIILiteral("Unable to get object details - GlobalObject");
        return;
    }

    InjectedScript injectedScript = m_injectedScriptManager.injectedScriptFor(globalObject->globalExec());
    if (injectedScript.hasNoValue()) {
        errorString = ASCIILiteral("Unable to get object details - InjectedScript");
        return;
    }

    // Function preview.
    if (cell->inherits(vm, JSFunction::info())) {
        injectedScript.functionDetails(errorString, cell, &functionDetails);
        return;
    }

    // Object preview.
    objectPreview = injectedScript.previewValue(cell);
}

void InspectorHeapAgent::getRemoteObject(ErrorString& errorString, int heapObjectId, const String* const optionalObjectGroup, RefPtr<Inspector::Protocol::Runtime::RemoteObject>& result)
{
    // Prevent the cell from getting collected as we look it up.
    VM& vm = m_environment.vm();
    JSLockHolder lock(vm);
    DeferGC deferGC(vm.heap);

    unsigned heapObjectIdentifier = static_cast<unsigned>(heapObjectId);
    const std::optional<HeapSnapshotNode> optionalNode = nodeForHeapObjectIdentifier(errorString, heapObjectIdentifier);
    if (!optionalNode)
        return;

    JSCell* cell = optionalNode->cell;
    Structure* structure = cell->structure(vm);
    if (!structure) {
        errorString = ASCIILiteral("Unable to get object details");
        return;
    }

    JSGlobalObject* globalObject = structure->globalObject();
    if (!globalObject) {
        errorString = ASCIILiteral("Unable to get object details");
        return;
    }

    InjectedScript injectedScript = m_injectedScriptManager.injectedScriptFor(globalObject->globalExec());
    if (injectedScript.hasNoValue()) {
        errorString = ASCIILiteral("Unable to get object details - InjectedScript");
        return;
    }

    String objectGroup = optionalObjectGroup ? *optionalObjectGroup : String();
    result = injectedScript.wrapObject(cell, objectGroup, true);
}

static Inspector::Protocol::Heap::GarbageCollection::Type protocolTypeForHeapOperation(CollectionScope scope)
{
    switch (scope) {
    case CollectionScope::Full:
        return Inspector::Protocol::Heap::GarbageCollection::Type::Full;
    case CollectionScope::Eden:
        return Inspector::Protocol::Heap::GarbageCollection::Type::Partial;
    }
    ASSERT_NOT_REACHED();
    return Inspector::Protocol::Heap::GarbageCollection::Type::Full;
}

void InspectorHeapAgent::willGarbageCollect()
{
    if (!m_enabled)
        return;

    m_gcStartTime = m_environment.executionStopwatch()->elapsedTime();
}

void InspectorHeapAgent::didGarbageCollect(CollectionScope scope)
{
    if (!m_enabled) {
        m_gcStartTime = NAN;
        return;
    }

    if (std::isnan(m_gcStartTime)) {
        // We were not enabled when the GC began.
        return;
    }

    // FIXME: Include number of bytes freed by collection.

    double endTime = m_environment.executionStopwatch()->elapsedTime();
    dispatchGarbageCollectedEvent(protocolTypeForHeapOperation(scope), m_gcStartTime, endTime);

    m_gcStartTime = NAN;
}

void InspectorHeapAgent::clearHeapSnapshots()
{
    VM& vm = m_environment.vm();
    JSLockHolder lock(vm);

    if (HeapProfiler* heapProfiler = vm.heapProfiler()) {
        heapProfiler->clearSnapshots();
        HeapSnapshotBuilder::resetNextAvailableObjectIdentifier();
    }
}

void InspectorHeapAgent::dispatchGarbageCollectedEvent(Inspector::Protocol::Heap::GarbageCollection::Type type, double startTime, double endTime)
{
    auto protocolObject = Inspector::Protocol::Heap::GarbageCollection::create()
        .setType(type)
        .setStartTime(startTime)
        .setEndTime(endTime)
        .release();

    m_frontendDispatcher->garbageCollected(WTFMove(protocolObject));
}

} // namespace Inspector
