/*
 * Copyright (C) 2013-2015 Apple Inc. All rights reserved.
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

#include "config.h"
#include "InspectorRuntimeAgent.h"

#include "Completion.h"
#include "DFGWorklist.h"
#include "HeapIterationScope.h"
#include "InjectedScript.h"
#include "InjectedScriptManager.h"
#include "InspectorFrontendRouter.h"
#include "InspectorValues.h"
#include "JSLock.h"
#include "ParserError.h"
#include "ScriptDebugServer.h"
#include "SourceCode.h"
#include "TypeProfiler.h"
#include "TypeProfilerLog.h"
#include <wtf/CurrentTime.h>

using namespace JSC;

namespace Inspector {

static bool asBool(const bool* const b)
{
    return b ? *b : false;
}

InspectorRuntimeAgent::InspectorRuntimeAgent(AgentContext& context)
    : InspectorAgentBase(ASCIILiteral("Runtime"))
    , m_injectedScriptManager(context.injectedScriptManager)
    , m_scriptDebugServer(context.environment.scriptDebugServer())
    , m_vm(context.environment.vm())
{
}

InspectorRuntimeAgent::~InspectorRuntimeAgent()
{
}

static ScriptDebugServer::PauseOnExceptionsState setPauseOnExceptionsState(ScriptDebugServer& scriptDebugServer, ScriptDebugServer::PauseOnExceptionsState newState)
{
    ScriptDebugServer::PauseOnExceptionsState presentState = scriptDebugServer.pauseOnExceptionsState();
    if (presentState != newState)
        scriptDebugServer.setPauseOnExceptionsState(newState);
    return presentState;
}

static Ref<Inspector::Protocol::Runtime::ErrorRange> buildErrorRangeObject(const JSTokenLocation& tokenLocation)
{
    return Inspector::Protocol::Runtime::ErrorRange::create()
        .setStartOffset(tokenLocation.startOffset)
        .setEndOffset(tokenLocation.endOffset)
        .release();
}

void InspectorRuntimeAgent::parse(ErrorString&, const String& expression, Inspector::Protocol::Runtime::SyntaxErrorType* result, Inspector::Protocol::OptOutput<String>* message, RefPtr<Inspector::Protocol::Runtime::ErrorRange>& range)
{
    JSLockHolder lock(m_vm);

    ParserError error;
    checkSyntax(m_vm, JSC::makeSource(expression, { }), error);

    switch (error.syntaxErrorType()) {
    case ParserError::SyntaxErrorNone:
        *result = Inspector::Protocol::Runtime::SyntaxErrorType::None;
        break;
    case ParserError::SyntaxErrorIrrecoverable:
        *result = Inspector::Protocol::Runtime::SyntaxErrorType::Irrecoverable;
        break;
    case ParserError::SyntaxErrorUnterminatedLiteral:
        *result = Inspector::Protocol::Runtime::SyntaxErrorType::UnterminatedLiteral;
        break;
    case ParserError::SyntaxErrorRecoverable:
        *result = Inspector::Protocol::Runtime::SyntaxErrorType::Recoverable;
        break;
    }

    if (error.syntaxErrorType() != ParserError::SyntaxErrorNone) {
        *message = error.message();
        range = buildErrorRangeObject(error.token().m_location);
    }
}

void InspectorRuntimeAgent::evaluate(ErrorString& errorString, const String& expression, const String* const objectGroup, const bool* const includeCommandLineAPI, const bool* const doNotPauseOnExceptionsAndMuteConsole, const int* executionContextId, const bool* const returnByValue, const bool* generatePreview, const bool* saveResult, RefPtr<Inspector::Protocol::Runtime::RemoteObject>& result, Inspector::Protocol::OptOutput<bool>* wasThrown, Inspector::Protocol::OptOutput<int>* savedResultIndex)
{
    InjectedScript injectedScript = injectedScriptForEval(errorString, executionContextId);
    if (injectedScript.hasNoValue())
        return;

    ScriptDebugServer::PauseOnExceptionsState previousPauseOnExceptionsState = ScriptDebugServer::DontPauseOnExceptions;
    if (asBool(doNotPauseOnExceptionsAndMuteConsole))
        previousPauseOnExceptionsState = setPauseOnExceptionsState(m_scriptDebugServer, ScriptDebugServer::DontPauseOnExceptions);
    if (asBool(doNotPauseOnExceptionsAndMuteConsole))
        muteConsole();

    injectedScript.evaluate(errorString, expression, objectGroup ? *objectGroup : String(), asBool(includeCommandLineAPI), asBool(returnByValue), asBool(generatePreview), asBool(saveResult), &result, wasThrown, savedResultIndex);

    if (asBool(doNotPauseOnExceptionsAndMuteConsole)) {
        unmuteConsole();
        setPauseOnExceptionsState(m_scriptDebugServer, previousPauseOnExceptionsState);
    }
}

void InspectorRuntimeAgent::callFunctionOn(ErrorString& errorString, const String& objectId, const String& expression, const InspectorArray* optionalArguments, const bool* const doNotPauseOnExceptionsAndMuteConsole, const bool* const returnByValue, const bool* generatePreview, RefPtr<Inspector::Protocol::Runtime::RemoteObject>& result, Inspector::Protocol::OptOutput<bool>* wasThrown)
{
    InjectedScript injectedScript = m_injectedScriptManager.injectedScriptForObjectId(objectId);
    if (injectedScript.hasNoValue()) {
        errorString = ASCIILiteral("Could not find InjectedScript for objectId");
        return;
    }

    String arguments;
    if (optionalArguments)
        arguments = optionalArguments->toJSONString();

    ScriptDebugServer::PauseOnExceptionsState previousPauseOnExceptionsState = ScriptDebugServer::DontPauseOnExceptions;
    if (asBool(doNotPauseOnExceptionsAndMuteConsole))
        previousPauseOnExceptionsState = setPauseOnExceptionsState(m_scriptDebugServer, ScriptDebugServer::DontPauseOnExceptions);
    if (asBool(doNotPauseOnExceptionsAndMuteConsole))
        muteConsole();

    injectedScript.callFunctionOn(errorString, objectId, expression, arguments, asBool(returnByValue), asBool(generatePreview), &result, wasThrown);

    if (asBool(doNotPauseOnExceptionsAndMuteConsole)) {
        unmuteConsole();
        setPauseOnExceptionsState(m_scriptDebugServer, previousPauseOnExceptionsState);
    }
}

void InspectorRuntimeAgent::getProperties(ErrorString& errorString, const String& objectId, const bool* const ownProperties, const bool* const generatePreview, RefPtr<Inspector::Protocol::Array<Inspector::Protocol::Runtime::PropertyDescriptor>>& result, RefPtr<Inspector::Protocol::Array<Inspector::Protocol::Runtime::InternalPropertyDescriptor>>& internalProperties)
{
    InjectedScript injectedScript = m_injectedScriptManager.injectedScriptForObjectId(objectId);
    if (injectedScript.hasNoValue()) {
        errorString = ASCIILiteral("Could not find InjectedScript for objectId");
        return;
    }

    ScriptDebugServer::PauseOnExceptionsState previousPauseOnExceptionsState = setPauseOnExceptionsState(m_scriptDebugServer, ScriptDebugServer::DontPauseOnExceptions);
    muteConsole();

    injectedScript.getProperties(errorString, objectId, asBool(ownProperties), asBool(generatePreview), &result);
    injectedScript.getInternalProperties(errorString, objectId, asBool(generatePreview), &internalProperties);

    unmuteConsole();
    setPauseOnExceptionsState(m_scriptDebugServer, previousPauseOnExceptionsState);
}

void InspectorRuntimeAgent::getDisplayableProperties(ErrorString& errorString, const String& objectId, const bool* const generatePreview, RefPtr<Inspector::Protocol::Array<Inspector::Protocol::Runtime::PropertyDescriptor>>& result, RefPtr<Inspector::Protocol::Array<Inspector::Protocol::Runtime::InternalPropertyDescriptor>>& internalProperties)
{
    InjectedScript injectedScript = m_injectedScriptManager.injectedScriptForObjectId(objectId);
    if (injectedScript.hasNoValue()) {
        errorString = ASCIILiteral("Could not find InjectedScript for objectId");
        return;
    }

    ScriptDebugServer::PauseOnExceptionsState previousPauseOnExceptionsState = setPauseOnExceptionsState(m_scriptDebugServer, ScriptDebugServer::DontPauseOnExceptions);
    muteConsole();

    injectedScript.getDisplayableProperties(errorString, objectId, asBool(generatePreview), &result);
    injectedScript.getInternalProperties(errorString, objectId, asBool(generatePreview), &internalProperties);

    unmuteConsole();
    setPauseOnExceptionsState(m_scriptDebugServer, previousPauseOnExceptionsState);
}

void InspectorRuntimeAgent::getCollectionEntries(ErrorString& errorString, const String& objectId, const String* objectGroup, const int* startIndex, const int* numberToFetch, RefPtr<Inspector::Protocol::Array<Inspector::Protocol::Runtime::CollectionEntry>>& entries)
{
    InjectedScript injectedScript = m_injectedScriptManager.injectedScriptForObjectId(objectId);
    if (injectedScript.hasNoValue()) {
        errorString = ASCIILiteral("Could not find InjectedScript for objectId");
        return;
    }

    int start = startIndex && *startIndex >= 0 ? *startIndex : 0;
    int fetch = numberToFetch && *numberToFetch >= 0 ? *numberToFetch : 0;

    injectedScript.getCollectionEntries(errorString, objectId, objectGroup ? *objectGroup : String(), start, fetch, &entries);
}

void InspectorRuntimeAgent::saveResult(ErrorString& errorString, const Inspector::InspectorObject& callArgument, const int* executionContextId, Inspector::Protocol::OptOutput<int>* savedResultIndex)
{
    InjectedScript injectedScript;

    String objectId;
    if (callArgument.getString(ASCIILiteral("objectId"), objectId)) {
        injectedScript = m_injectedScriptManager.injectedScriptForObjectId(objectId);
        if (injectedScript.hasNoValue()) {
            errorString = ASCIILiteral("Could not find InjectedScript for objectId");
            return;
        }
    } else {
        injectedScript = injectedScriptForEval(errorString, executionContextId);
        if (injectedScript.hasNoValue())
            return;
    }

    injectedScript.saveResult(errorString, callArgument.toJSONString(), savedResultIndex);
}

void InspectorRuntimeAgent::releaseObject(ErrorString&, const String& objectId)
{
    InjectedScript injectedScript = m_injectedScriptManager.injectedScriptForObjectId(objectId);
    if (!injectedScript.hasNoValue())
        injectedScript.releaseObject(objectId);
}

void InspectorRuntimeAgent::releaseObjectGroup(ErrorString&, const String& objectGroup)
{
    m_injectedScriptManager.releaseObjectGroup(objectGroup);
}

void InspectorRuntimeAgent::getRuntimeTypesForVariablesAtOffsets(ErrorString& errorString, const Inspector::InspectorArray& locations, RefPtr<Inspector::Protocol::Array<Inspector::Protocol::Runtime::TypeDescription>>& typeDescriptions)
{
    static const bool verbose = false;

    typeDescriptions = Inspector::Protocol::Array<Inspector::Protocol::Runtime::TypeDescription>::create();
    if (!m_vm.typeProfiler()) {
        errorString = ASCIILiteral("The VM does not currently have Type Information.");
        return;
    }

    double start = currentTimeMS();
    m_vm.typeProfilerLog()->processLogEntries(ASCIILiteral("User Query"));

    for (size_t i = 0; i < locations.length(); i++) {
        RefPtr<Inspector::InspectorValue> value = locations.get(i);
        RefPtr<InspectorObject> location;
        if (!value->asObject(location)) {
            errorString = ASCIILiteral("Array of TypeLocation objects has an object that does not have type of TypeLocation.");
            return;
        }

        int descriptor;
        String sourceIDAsString;
        int divot;
        location->getInteger(ASCIILiteral("typeInformationDescriptor"), descriptor);
        location->getString(ASCIILiteral("sourceID"), sourceIDAsString);
        location->getInteger(ASCIILiteral("divot"), divot);

        bool okay;
        TypeLocation* typeLocation = m_vm.typeProfiler()->findLocation(divot, sourceIDAsString.toIntPtrStrict(&okay), static_cast<TypeProfilerSearchDescriptor>(descriptor), m_vm);
        ASSERT(okay);

        RefPtr<TypeSet> typeSet;
        if (typeLocation) {
            if (typeLocation->m_globalTypeSet && typeLocation->m_globalVariableID != TypeProfilerNoGlobalIDExists)
                typeSet = typeLocation->m_globalTypeSet;
            else
                typeSet = typeLocation->m_instructionTypeSet;
        }

        bool isValid = typeLocation && typeSet && !typeSet->isEmpty();
        auto description = Inspector::Protocol::Runtime::TypeDescription::create()
            .setIsValid(isValid)
            .release();

        if (isValid) {
            description->setLeastCommonAncestor(typeSet->leastCommonAncestor());
            description->setStructures(typeSet->allStructureRepresentations());
            description->setTypeSet(typeSet->inspectorTypeSet());
            description->setIsTruncated(typeSet->isOverflown());
        }

        typeDescriptions->addItem(WTFMove(description));
    }

    double end = currentTimeMS();
    if (verbose)
        dataLogF("Inspector::getRuntimeTypesForVariablesAtOffsets took %lfms\n", end - start);
}

void InspectorRuntimeAgent::willDestroyFrontendAndBackend(DisconnectReason reason)
{
    if (reason != DisconnectReason::InspectedTargetDestroyed && m_isTypeProfilingEnabled)
        setTypeProfilerEnabledState(false);
}

void InspectorRuntimeAgent::enableTypeProfiler(ErrorString&)
{
    setTypeProfilerEnabledState(true);
}

void InspectorRuntimeAgent::disableTypeProfiler(ErrorString&)
{
    setTypeProfilerEnabledState(false);
}

void InspectorRuntimeAgent::enableControlFlowProfiler(ErrorString&)
{
    setControlFlowProfilerEnabledState(true);
}

void InspectorRuntimeAgent::disableControlFlowProfiler(ErrorString&)
{
    setControlFlowProfilerEnabledState(false);
}

void InspectorRuntimeAgent::setTypeProfilerEnabledState(bool isTypeProfilingEnabled)
{
    if (m_isTypeProfilingEnabled == isTypeProfilingEnabled)
        return;
    m_isTypeProfilingEnabled = isTypeProfilingEnabled;

    VM& vm = m_vm;
    vm.whenIdle([&vm, isTypeProfilingEnabled] () {
        bool shouldRecompileFromTypeProfiler = (isTypeProfilingEnabled ? vm.enableTypeProfiler() : vm.disableTypeProfiler());
        if (shouldRecompileFromTypeProfiler)
            vm.deleteAllCode(PreventCollectionAndDeleteAllCode);
    });
}

void InspectorRuntimeAgent::setControlFlowProfilerEnabledState(bool isControlFlowProfilingEnabled)
{
    if (m_isControlFlowProfilingEnabled == isControlFlowProfilingEnabled)
        return;
    m_isControlFlowProfilingEnabled = isControlFlowProfilingEnabled;

    VM& vm = m_vm;
    vm.whenIdle([&vm, isControlFlowProfilingEnabled] () {
        bool shouldRecompileFromControlFlowProfiler = (isControlFlowProfilingEnabled ? vm.enableControlFlowProfiler() : vm.disableControlFlowProfiler());

        if (shouldRecompileFromControlFlowProfiler)
            vm.deleteAllCode(PreventCollectionAndDeleteAllCode);
    });
}

void InspectorRuntimeAgent::getBasicBlocks(ErrorString& errorString, const String& sourceIDAsString, RefPtr<Inspector::Protocol::Array<Inspector::Protocol::Runtime::BasicBlock>>& basicBlocks)
{
    if (!m_vm.controlFlowProfiler()) {
        errorString = ASCIILiteral("The VM does not currently have a Control Flow Profiler.");
        return;
    }

    bool okay;
    intptr_t sourceID = sourceIDAsString.toIntPtrStrict(&okay);
    ASSERT(okay);
    const Vector<BasicBlockRange>& basicBlockRanges = m_vm.controlFlowProfiler()->getBasicBlocksForSourceID(sourceID, m_vm);
    basicBlocks = Inspector::Protocol::Array<Inspector::Protocol::Runtime::BasicBlock>::create();
    for (const BasicBlockRange& block : basicBlockRanges) {
        Ref<Inspector::Protocol::Runtime::BasicBlock> location = Inspector::Protocol::Runtime::BasicBlock::create()
            .setStartOffset(block.m_startOffset)
            .setEndOffset(block.m_endOffset)
            .setHasExecuted(block.m_hasExecuted)
            .setExecutionCount(block.m_executionCount)
            .release();
        basicBlocks->addItem(WTFMove(location));
    }
}

} // namespace Inspector
