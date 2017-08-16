/*
 * Copyright (C) 2010, 2013, 2015 Apple Inc. All rights reserved.
 * Copyright (C) 2010, 2011 Google Inc. All rights reserved.
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

#include "config.h"
#include "InspectorDebuggerAgent.h"

#include "AsyncStackTrace.h"
#include "ContentSearchUtilities.h"
#include "InjectedScript.h"
#include "InjectedScriptManager.h"
#include "InspectorFrontendRouter.h"
#include "InspectorValues.h"
#include "JSCInlines.h"
#include "RegularExpression.h"
#include "ScriptCallStackFactory.h"
#include "ScriptDebugServer.h"
#include "ScriptObject.h"
#include "ScriptValue.h"
#include <wtf/NeverDestroyed.h>
#include <wtf/Stopwatch.h>
#include <wtf/text/WTFString.h>

namespace Inspector {

const char* InspectorDebuggerAgent::backtraceObjectGroup = "backtrace";

// Objects created and retained by evaluating breakpoint actions are put into object groups
// according to the breakpoint action identifier assigned by the frontend. A breakpoint may
// have several object groups, and objects from several backend breakpoint action instances may
// create objects in the same group.
static String objectGroupForBreakpointAction(const ScriptBreakpointAction& action)
{
    static NeverDestroyed<String> objectGroup(ASCIILiteral("breakpoint-action-"));
    return makeString(objectGroup.get(), String::number(action.identifier));
}

InspectorDebuggerAgent::InspectorDebuggerAgent(AgentContext& context)
    : InspectorAgentBase(ASCIILiteral("Debugger"))
    , m_injectedScriptManager(context.injectedScriptManager)
    , m_frontendDispatcher(std::make_unique<DebuggerFrontendDispatcher>(context.frontendRouter))
    , m_backendDispatcher(DebuggerBackendDispatcher::create(context.backendDispatcher, this))
    , m_scriptDebugServer(context.environment.scriptDebugServer())
    , m_continueToLocationBreakpointID(JSC::noBreakpointID)
{
    // FIXME: make breakReason optional so that there was no need to init it with "other".
    clearBreakDetails();
}

InspectorDebuggerAgent::~InspectorDebuggerAgent()
{
}

void InspectorDebuggerAgent::didCreateFrontendAndBackend(FrontendRouter*, BackendDispatcher*)
{
}

void InspectorDebuggerAgent::willDestroyFrontendAndBackend(DisconnectReason reason)
{
    bool skipRecompile = reason == DisconnectReason::InspectedTargetDestroyed;
    disable(skipRecompile);
}

void InspectorDebuggerAgent::enable()
{
    if (m_enabled)
        return;

    m_scriptDebugServer.addListener(this);

    if (m_listener)
        m_listener->debuggerWasEnabled();

    m_enabled = true;
}

void InspectorDebuggerAgent::disable(bool isBeingDestroyed)
{
    if (!m_enabled)
        return;

    m_scriptDebugServer.removeListener(this, isBeingDestroyed);
    clearInspectorBreakpointState();

    if (!isBeingDestroyed)
        m_scriptDebugServer.deactivateBreakpoints();

    ASSERT(m_javaScriptBreakpoints.isEmpty());

    if (m_listener)
        m_listener->debuggerWasDisabled();

    clearAsyncStackTraceData();

    m_pauseOnAssertionFailures = false;

    m_enabled = false;
}

void InspectorDebuggerAgent::enable(ErrorString&)
{
    enable();
}

void InspectorDebuggerAgent::disable(ErrorString&)
{
    disable(false);
}

bool InspectorDebuggerAgent::breakpointsActive() const
{
    return m_scriptDebugServer.breakpointsActive();
}

void InspectorDebuggerAgent::setAsyncStackTraceDepth(ErrorString& errorString, int depth)
{
    if (m_asyncStackTraceDepth == depth)
        return;

    if (depth < 0) {
        errorString = ASCIILiteral("depth must be a positive number.");
        return;
    }

    m_asyncStackTraceDepth = depth;

    if (!m_asyncStackTraceDepth)
        clearAsyncStackTraceData();
}

void InspectorDebuggerAgent::setBreakpointsActive(ErrorString&, bool active)
{
    if (active)
        m_scriptDebugServer.activateBreakpoints();
    else
        m_scriptDebugServer.deactivateBreakpoints();
}

bool InspectorDebuggerAgent::isPaused() const
{
    return m_scriptDebugServer.isPaused();
}

void InspectorDebuggerAgent::setSuppressAllPauses(bool suppress)
{
    m_scriptDebugServer.setSuppressAllPauses(suppress);
}

static RefPtr<InspectorObject> buildAssertPauseReason(const String& message)
{
    auto reason = Inspector::Protocol::Debugger::AssertPauseReason::create().release();
    if (!message.isNull())
        reason->setMessage(message);
    return reason->openAccessors();
}

static RefPtr<InspectorObject> buildCSPViolationPauseReason(const String& directiveText)
{
    auto reason = Inspector::Protocol::Debugger::CSPViolationPauseReason::create()
        .setDirective(directiveText)
        .release();
    return reason->openAccessors();
}

RefPtr<InspectorObject> InspectorDebuggerAgent::buildBreakpointPauseReason(JSC::BreakpointID debuggerBreakpointIdentifier)
{
    ASSERT(debuggerBreakpointIdentifier != JSC::noBreakpointID);
    auto it = m_debuggerBreakpointIdentifierToInspectorBreakpointIdentifier.find(debuggerBreakpointIdentifier);
    if (it == m_debuggerBreakpointIdentifierToInspectorBreakpointIdentifier.end())
        return nullptr;

    auto reason = Inspector::Protocol::Debugger::BreakpointPauseReason::create()
        .setBreakpointId(it->value)
        .release();
    return reason->openAccessors();
}

RefPtr<InspectorObject> InspectorDebuggerAgent::buildExceptionPauseReason(JSC::JSValue exception, const InjectedScript& injectedScript)
{
    ASSERT(exception);
    if (!exception)
        return nullptr;

    ASSERT(!injectedScript.hasNoValue());
    if (injectedScript.hasNoValue())
        return nullptr;

    return injectedScript.wrapObject(exception, InspectorDebuggerAgent::backtraceObjectGroup)->openAccessors();
}

void InspectorDebuggerAgent::handleConsoleAssert(const String& message)
{
    if (!m_scriptDebugServer.breakpointsActive())
        return;

    if (m_pauseOnAssertionFailures)
        breakProgram(DebuggerFrontendDispatcher::Reason::Assert, buildAssertPauseReason(message));
}

void InspectorDebuggerAgent::didScheduleAsyncCall(JSC::ExecState* exec, int asyncCallType, int callbackIdentifier, bool singleShot)
{
    if (!m_asyncStackTraceDepth)
        return;

    if (!m_scriptDebugServer.breakpointsActive())
        return;

    Ref<ScriptCallStack> callStack = createScriptCallStack(exec, m_asyncStackTraceDepth);
    ASSERT(callStack->size());
    if (!callStack->size())
        return;

    RefPtr<AsyncStackTrace> parentStackTrace;
    if (m_currentAsyncCallIdentifier) {
        auto it = m_pendingAsyncCalls.find(m_currentAsyncCallIdentifier.value());
        ASSERT(it != m_pendingAsyncCalls.end());
        parentStackTrace = it->value;
    }

    auto identifier = std::make_pair(asyncCallType, callbackIdentifier);
    auto asyncStackTrace = AsyncStackTrace::create(WTFMove(callStack), singleShot, WTFMove(parentStackTrace));

    m_pendingAsyncCalls.set(identifier, WTFMove(asyncStackTrace));
}

void InspectorDebuggerAgent::didCancelAsyncCall(int asyncCallType, int callbackIdentifier)
{
    if (!m_asyncStackTraceDepth)
        return;

    auto identifier = std::make_pair(asyncCallType, callbackIdentifier);
    auto it = m_pendingAsyncCalls.find(identifier);
    if (it == m_pendingAsyncCalls.end())
        return;

    auto& asyncStackTrace = it->value;
    asyncStackTrace->didCancelAsyncCall();

    if (m_currentAsyncCallIdentifier && m_currentAsyncCallIdentifier.value() == identifier)
        return;

    m_pendingAsyncCalls.remove(identifier);
}

void InspectorDebuggerAgent::willDispatchAsyncCall(int asyncCallType, int callbackIdentifier)
{
    if (!m_asyncStackTraceDepth)
        return;

    if (m_currentAsyncCallIdentifier)
        return;

    // A call can be scheduled before the Inspector is opened, or while async stack
    // traces are disabled. If no call data exists, do nothing.
    auto identifier = std::make_pair(asyncCallType, callbackIdentifier);
    auto it = m_pendingAsyncCalls.find(identifier);
    if (it == m_pendingAsyncCalls.end())
        return;

    auto& asyncStackTrace = it->value;
    asyncStackTrace->willDispatchAsyncCall(m_asyncStackTraceDepth);

    m_currentAsyncCallIdentifier = identifier;
}

void InspectorDebuggerAgent::didDispatchAsyncCall()
{
    if (!m_asyncStackTraceDepth)
        return;

    if (!m_currentAsyncCallIdentifier)
        return;

    auto identifier = m_currentAsyncCallIdentifier.value();
    auto it = m_pendingAsyncCalls.find(identifier);
    ASSERT(it != m_pendingAsyncCalls.end());

    auto& asyncStackTrace = it->value;
    asyncStackTrace->didDispatchAsyncCall();

    m_currentAsyncCallIdentifier = std::nullopt;

    if (!asyncStackTrace->isPending())
        m_pendingAsyncCalls.remove(identifier);
}

static Ref<InspectorObject> buildObjectForBreakpointCookie(const String& url, int lineNumber, int columnNumber, const String& condition, RefPtr<InspectorArray>& actions, bool isRegex, bool autoContinue, unsigned ignoreCount)
{
    Ref<InspectorObject> breakpointObject = InspectorObject::create();
    breakpointObject->setString(ASCIILiteral("url"), url);
    breakpointObject->setInteger(ASCIILiteral("lineNumber"), lineNumber);
    breakpointObject->setInteger(ASCIILiteral("columnNumber"), columnNumber);
    breakpointObject->setString(ASCIILiteral("condition"), condition);
    breakpointObject->setBoolean(ASCIILiteral("isRegex"), isRegex);
    breakpointObject->setBoolean(ASCIILiteral("autoContinue"), autoContinue);
    breakpointObject->setInteger(ASCIILiteral("ignoreCount"), ignoreCount);

    if (actions)
        breakpointObject->setArray(ASCIILiteral("actions"), actions);

    return breakpointObject;
}

static bool matches(const String& url, const String& pattern, bool isRegex)
{
    if (isRegex) {
        JSC::Yarr::RegularExpression regex(pattern, TextCaseSensitive);
        return regex.match(url) != -1;
    }
    return url == pattern;
}

static bool breakpointActionTypeForString(const String& typeString, ScriptBreakpointActionType* output)
{
    if (typeString == Inspector::Protocol::InspectorHelpers::getEnumConstantValue(Inspector::Protocol::Debugger::BreakpointAction::Type::Log)) {
        *output = ScriptBreakpointActionTypeLog;
        return true;
    }
    if (typeString == Inspector::Protocol::InspectorHelpers::getEnumConstantValue(Inspector::Protocol::Debugger::BreakpointAction::Type::Evaluate)) {
        *output = ScriptBreakpointActionTypeEvaluate;
        return true;
    }
    if (typeString == Inspector::Protocol::InspectorHelpers::getEnumConstantValue(Inspector::Protocol::Debugger::BreakpointAction::Type::Sound)) {
        *output = ScriptBreakpointActionTypeSound;
        return true;
    }
    if (typeString == Inspector::Protocol::InspectorHelpers::getEnumConstantValue(Inspector::Protocol::Debugger::BreakpointAction::Type::Probe)) {
        *output = ScriptBreakpointActionTypeProbe;
        return true;
    }

    return false;
}

bool InspectorDebuggerAgent::breakpointActionsFromProtocol(ErrorString& errorString, RefPtr<InspectorArray>& actions, BreakpointActions* result)
{
    if (!actions)
        return true;

    unsigned actionsLength = actions->length();
    if (!actionsLength)
        return true;

    result->reserveCapacity(actionsLength);
    for (unsigned i = 0; i < actionsLength; ++i) {
        RefPtr<InspectorValue> value = actions->get(i);
        RefPtr<InspectorObject> object;
        if (!value->asObject(object)) {
            errorString = ASCIILiteral("BreakpointAction of incorrect type, expected object");
            return false;
        }

        String typeString;
        if (!object->getString(ASCIILiteral("type"), typeString)) {
            errorString = ASCIILiteral("BreakpointAction had type missing");
            return false;
        }

        ScriptBreakpointActionType type;
        if (!breakpointActionTypeForString(typeString, &type)) {
            errorString = ASCIILiteral("BreakpointAction had unknown type");
            return false;
        }

        // Specifying an identifier is optional. They are used to correlate probe samples
        // in the frontend across multiple backend probe actions and segregate object groups.
        int identifier = 0;
        object->getInteger(ASCIILiteral("id"), identifier);

        String data;
        object->getString(ASCIILiteral("data"), data);

        result->append(ScriptBreakpointAction(type, identifier, data));
    }

    return true;
}

static RefPtr<Inspector::Protocol::Debugger::Location> buildDebuggerLocation(const JSC::Breakpoint& breakpoint)
{
    ASSERT(breakpoint.resolved);

    auto location = Inspector::Protocol::Debugger::Location::create()
        .setScriptId(String::number(breakpoint.sourceID))
        .setLineNumber(breakpoint.line)
        .release();
    location->setColumnNumber(breakpoint.column);

    return WTFMove(location);
}

static bool parseLocation(ErrorString& errorString, const InspectorObject& location, JSC::SourceID& sourceID, unsigned& lineNumber, unsigned& columnNumber)
{
    String scriptIDStr;
    if (!location.getString(ASCIILiteral("scriptId"), scriptIDStr) || !location.getInteger(ASCIILiteral("lineNumber"), lineNumber)) {
        sourceID = JSC::noSourceID;
        errorString = ASCIILiteral("scriptId and lineNumber are required.");
        return false;
    }

    sourceID = scriptIDStr.toIntPtr();
    columnNumber = 0;
    location.getInteger(ASCIILiteral("columnNumber"), columnNumber);
    return true;
}

void InspectorDebuggerAgent::setBreakpointByUrl(ErrorString& errorString, int lineNumber, const String* const optionalURL, const String* const optionalURLRegex, const int* const optionalColumnNumber, const InspectorObject* options, Inspector::Protocol::Debugger::BreakpointId* outBreakpointIdentifier, RefPtr<Inspector::Protocol::Array<Inspector::Protocol::Debugger::Location>>& locations)
{
    locations = Inspector::Protocol::Array<Inspector::Protocol::Debugger::Location>::create();
    if (!optionalURL == !optionalURLRegex) {
        errorString = ASCIILiteral("Either url or urlRegex must be specified.");
        return;
    }

    String url = optionalURL ? *optionalURL : *optionalURLRegex;
    int columnNumber = optionalColumnNumber ? *optionalColumnNumber : 0;
    bool isRegex = optionalURLRegex;

    String breakpointIdentifier = (isRegex ? "/" + url + "/" : url) + ':' + String::number(lineNumber) + ':' + String::number(columnNumber);
    if (m_javaScriptBreakpoints.contains(breakpointIdentifier)) {
        errorString = ASCIILiteral("Breakpoint at specified location already exists.");
        return;
    }

    String condition = emptyString();
    bool autoContinue = false;
    unsigned ignoreCount = 0;
    RefPtr<InspectorArray> actions;
    if (options) {
        options->getString(ASCIILiteral("condition"), condition);
        options->getBoolean(ASCIILiteral("autoContinue"), autoContinue);
        options->getArray(ASCIILiteral("actions"), actions);
        options->getInteger(ASCIILiteral("ignoreCount"), ignoreCount);
    }

    BreakpointActions breakpointActions;
    if (!breakpointActionsFromProtocol(errorString, actions, &breakpointActions))
        return;

    m_javaScriptBreakpoints.set(breakpointIdentifier, buildObjectForBreakpointCookie(url, lineNumber, columnNumber, condition, actions, isRegex, autoContinue, ignoreCount));

    for (auto& entry : m_scripts) {
        Script& script = entry.value;
        String scriptURLForBreakpoints = !script.sourceURL.isEmpty() ? script.sourceURL : script.url;
        if (!matches(scriptURLForBreakpoints, url, isRegex))
            continue;

        JSC::SourceID sourceID = entry.key;
        JSC::Breakpoint breakpoint(sourceID, lineNumber, columnNumber, condition, autoContinue, ignoreCount);
        resolveBreakpoint(script, breakpoint);
        if (!breakpoint.resolved)
            continue;

        bool existing;
        setBreakpoint(breakpoint, existing);
        if (existing)
            continue;

        ScriptBreakpoint scriptBreakpoint(breakpoint.line, breakpoint.column, condition, breakpointActions, autoContinue, ignoreCount);
        didSetBreakpoint(breakpoint, breakpointIdentifier, scriptBreakpoint);

        locations->addItem(buildDebuggerLocation(breakpoint));
    }

    *outBreakpointIdentifier = breakpointIdentifier;
}

void InspectorDebuggerAgent::setBreakpoint(ErrorString& errorString, const InspectorObject& location, const InspectorObject* options, Inspector::Protocol::Debugger::BreakpointId* outBreakpointIdentifier, RefPtr<Inspector::Protocol::Debugger::Location>& actualLocation)
{
    JSC::SourceID sourceID;
    unsigned lineNumber;
    unsigned columnNumber;
    if (!parseLocation(errorString, location, sourceID, lineNumber, columnNumber))
        return;

    String condition = emptyString();
    bool autoContinue = false;
    unsigned ignoreCount = 0;
    RefPtr<InspectorArray> actions;
    if (options) {
        options->getString(ASCIILiteral("condition"), condition);
        options->getBoolean(ASCIILiteral("autoContinue"), autoContinue);
        options->getArray(ASCIILiteral("actions"), actions);
        options->getInteger(ASCIILiteral("ignoreCount"), ignoreCount);
    }

    BreakpointActions breakpointActions;
    if (!breakpointActionsFromProtocol(errorString, actions, &breakpointActions))
        return;

    auto scriptIterator = m_scripts.find(sourceID);
    if (scriptIterator == m_scripts.end()) {
        errorString = ASCIILiteral("No script for id: ") + String::number(sourceID);
        return;
    }

    Script& script = scriptIterator->value;
    JSC::Breakpoint breakpoint(sourceID, lineNumber, columnNumber, condition, autoContinue, ignoreCount);
    resolveBreakpoint(script, breakpoint);
    if (!breakpoint.resolved) {
        errorString = ASCIILiteral("Could not resolve breakpoint");
        return;
    }

    bool existing;
    setBreakpoint(breakpoint, existing);
    if (existing) {
        errorString = ASCIILiteral("Breakpoint at specified location already exists");
        return;
    }

    String breakpointIdentifier = String::number(sourceID) + ':' + String::number(breakpoint.line) + ':' + String::number(breakpoint.column);
    ScriptBreakpoint scriptBreakpoint(breakpoint.line, breakpoint.column, condition, breakpointActions, autoContinue, ignoreCount);
    didSetBreakpoint(breakpoint, breakpointIdentifier, scriptBreakpoint);

    actualLocation = buildDebuggerLocation(breakpoint);
    *outBreakpointIdentifier = breakpointIdentifier;
}

void InspectorDebuggerAgent::didSetBreakpoint(const JSC::Breakpoint& breakpoint, const String& breakpointIdentifier, const ScriptBreakpoint& scriptBreakpoint)
{
    JSC::BreakpointID id = breakpoint.id;
    m_scriptDebugServer.setBreakpointActions(id, scriptBreakpoint);

    auto debugServerBreakpointIDsIterator = m_breakpointIdentifierToDebugServerBreakpointIDs.find(breakpointIdentifier);
    if (debugServerBreakpointIDsIterator == m_breakpointIdentifierToDebugServerBreakpointIDs.end())
        debugServerBreakpointIDsIterator = m_breakpointIdentifierToDebugServerBreakpointIDs.set(breakpointIdentifier, Vector<JSC::BreakpointID>()).iterator;
    debugServerBreakpointIDsIterator->value.append(id);

    m_debuggerBreakpointIdentifierToInspectorBreakpointIdentifier.set(id, breakpointIdentifier);
}

void InspectorDebuggerAgent::resolveBreakpoint(const Script& script, JSC::Breakpoint& breakpoint)
{
    if (breakpoint.line < static_cast<unsigned>(script.startLine) || static_cast<unsigned>(script.endLine) < breakpoint.line)
        return;

    m_scriptDebugServer.resolveBreakpoint(breakpoint, script.sourceProvider.get());
}

void InspectorDebuggerAgent::setBreakpoint(JSC::Breakpoint& breakpoint, bool& existing)
{
    JSC::JSLockHolder locker(m_scriptDebugServer.vm());
    m_scriptDebugServer.setBreakpoint(breakpoint, existing);
}

void InspectorDebuggerAgent::removeBreakpoint(ErrorString&, const String& breakpointIdentifier)
{
    m_javaScriptBreakpoints.remove(breakpointIdentifier);

    for (JSC::BreakpointID breakpointID : m_breakpointIdentifierToDebugServerBreakpointIDs.take(breakpointIdentifier)) {
        m_debuggerBreakpointIdentifierToInspectorBreakpointIdentifier.remove(breakpointID);

        const BreakpointActions& breakpointActions = m_scriptDebugServer.getActionsForBreakpoint(breakpointID);
        for (auto& action : breakpointActions)
            m_injectedScriptManager.releaseObjectGroup(objectGroupForBreakpointAction(action));

        JSC::JSLockHolder locker(m_scriptDebugServer.vm());
        m_scriptDebugServer.removeBreakpointActions(breakpointID);
        m_scriptDebugServer.removeBreakpoint(breakpointID);
    }
}

void InspectorDebuggerAgent::continueUntilNextRunLoop(ErrorString& errorString)
{
    if (!assertPaused(errorString))
        return;

    resume(errorString);

    m_enablePauseWhenIdle = true;

    registerIdleHandler();
}

void InspectorDebuggerAgent::continueToLocation(ErrorString& errorString, const InspectorObject& location)
{
    if (!assertPaused(errorString))
        return;

    if (m_continueToLocationBreakpointID != JSC::noBreakpointID) {
        m_scriptDebugServer.removeBreakpoint(m_continueToLocationBreakpointID);
        m_continueToLocationBreakpointID = JSC::noBreakpointID;
    }

    JSC::SourceID sourceID;
    unsigned lineNumber;
    unsigned columnNumber;
    if (!parseLocation(errorString, location, sourceID, lineNumber, columnNumber))
        return;

    auto scriptIterator = m_scripts.find(sourceID);
    if (scriptIterator == m_scripts.end()) {
        m_scriptDebugServer.continueProgram();
        m_frontendDispatcher->resumed();
        errorString = ASCIILiteral("No script for id: ") + String::number(sourceID);
        return;
    }

    String condition;
    bool autoContinue = false;
    unsigned ignoreCount = 0;
    JSC::Breakpoint breakpoint(sourceID, lineNumber, columnNumber, condition, autoContinue, ignoreCount);
    Script& script = scriptIterator->value;
    resolveBreakpoint(script, breakpoint);
    if (!breakpoint.resolved) {
        m_scriptDebugServer.continueProgram();
        m_frontendDispatcher->resumed();
        errorString = ASCIILiteral("Could not resolve breakpoint");
        return;
    }

    bool existing;
    setBreakpoint(breakpoint, existing);
    if (existing) {
        // There is an existing breakpoint at this location. Instead of
        // acting like a series of steps, just resume and we will either
        // hit this new breakpoint or not.
        m_scriptDebugServer.continueProgram();
        m_frontendDispatcher->resumed();
        return;
    }

    m_continueToLocationBreakpointID = breakpoint.id;

    // Treat this as a series of steps until reaching the new breakpoint.
    // So don't issue a resumed event unless we exit the VM without pausing.
    willStepAndMayBecomeIdle();
    m_scriptDebugServer.continueProgram();
}

void InspectorDebuggerAgent::searchInContent(ErrorString& error, const String& scriptIDStr, const String& query, const bool* optionalCaseSensitive, const bool* optionalIsRegex, RefPtr<Inspector::Protocol::Array<Inspector::Protocol::GenericTypes::SearchMatch>>& results)
{
    JSC::SourceID sourceID = scriptIDStr.toIntPtr();
    auto it = m_scripts.find(sourceID);
    if (it == m_scripts.end()) {
        error = ASCIILiteral("No script for id: ") + scriptIDStr;
        return;
    }

    bool isRegex = optionalIsRegex ? *optionalIsRegex : false;
    bool caseSensitive = optionalCaseSensitive ? *optionalCaseSensitive : false;
    results = ContentSearchUtilities::searchInTextByLines(it->value.source, query, caseSensitive, isRegex);
}

void InspectorDebuggerAgent::getScriptSource(ErrorString& error, const String& scriptIDStr, String* scriptSource)
{
    JSC::SourceID sourceID = scriptIDStr.toIntPtr();
    ScriptsMap::iterator it = m_scripts.find(sourceID);
    if (it != m_scripts.end())
        *scriptSource = it->value.source;
    else
        error = ASCIILiteral("No script for id: ") + scriptIDStr;
}

void InspectorDebuggerAgent::getFunctionDetails(ErrorString& errorString, const String& functionId, RefPtr<Inspector::Protocol::Debugger::FunctionDetails>& details)
{
    InjectedScript injectedScript = m_injectedScriptManager.injectedScriptForObjectId(functionId);
    if (injectedScript.hasNoValue()) {
        errorString = ASCIILiteral("Function object id is obsolete");
        return;
    }

    injectedScript.getFunctionDetails(errorString, functionId, &details);
}

void InspectorDebuggerAgent::schedulePauseOnNextStatement(DebuggerFrontendDispatcher::Reason breakReason, RefPtr<InspectorObject>&& data)
{
    if (m_javaScriptPauseScheduled)
        return;

    m_javaScriptPauseScheduled = true;

    m_breakReason = breakReason;
    m_breakAuxData = WTFMove(data);

    JSC::JSLockHolder locker(m_scriptDebugServer.vm());
    m_scriptDebugServer.setPauseOnNextStatement(true);
}

void InspectorDebuggerAgent::cancelPauseOnNextStatement()
{
    if (!m_javaScriptPauseScheduled)
        return;

    m_javaScriptPauseScheduled = false;

    clearBreakDetails();
    m_scriptDebugServer.setPauseOnNextStatement(false);
    m_enablePauseWhenIdle = false;
}

void InspectorDebuggerAgent::pause(ErrorString&)
{
    schedulePauseOnNextStatement(DebuggerFrontendDispatcher::Reason::PauseOnNextStatement, nullptr);
}

void InspectorDebuggerAgent::resume(ErrorString& errorString)
{
    if (!m_pausedScriptState && !m_javaScriptPauseScheduled) {
        errorString = ASCIILiteral("Was not paused or waiting to pause");
        return;
    }

    cancelPauseOnNextStatement();
    m_scriptDebugServer.continueProgram();
    m_conditionToDispatchResumed = ShouldDispatchResumed::WhenContinued;
}

void InspectorDebuggerAgent::stepOver(ErrorString& errorString)
{
    if (!assertPaused(errorString))
        return;

    willStepAndMayBecomeIdle();
    m_scriptDebugServer.stepOverStatement();
}

void InspectorDebuggerAgent::stepInto(ErrorString& errorString)
{
    if (!assertPaused(errorString))
        return;

    willStepAndMayBecomeIdle();
    m_scriptDebugServer.stepIntoStatement();
}

void InspectorDebuggerAgent::stepOut(ErrorString& errorString)
{
    if (!assertPaused(errorString))
        return;

    willStepAndMayBecomeIdle();
    m_scriptDebugServer.stepOutOfFunction();
}

void InspectorDebuggerAgent::registerIdleHandler()
{
    if (!m_registeredIdleCallback) {
        m_registeredIdleCallback = true;
        JSC::VM& vm = m_scriptDebugServer.vm();
        vm.whenIdle([this]() {
            didBecomeIdle();
        });
    }
}

void InspectorDebuggerAgent::willStepAndMayBecomeIdle()
{
    // When stepping the backend must eventually trigger a "paused" or "resumed" event.
    // If the step causes us to exit the VM, then we should issue "resumed".
    m_conditionToDispatchResumed = ShouldDispatchResumed::WhenIdle;

    registerIdleHandler();
}

void InspectorDebuggerAgent::didBecomeIdle()
{
    m_registeredIdleCallback = false;

    if (m_conditionToDispatchResumed == ShouldDispatchResumed::WhenIdle) {
        cancelPauseOnNextStatement();
        m_scriptDebugServer.continueProgram();
        m_frontendDispatcher->resumed();
    }

    m_conditionToDispatchResumed = ShouldDispatchResumed::No;

    if (m_enablePauseWhenIdle) {
        ErrorString ignored;
        pause(ignored);
    }
}

void InspectorDebuggerAgent::setPauseOnExceptions(ErrorString& errorString, const String& stringPauseState)
{
    JSC::Debugger::PauseOnExceptionsState pauseState;
    if (stringPauseState == "none")
        pauseState = JSC::Debugger::DontPauseOnExceptions;
    else if (stringPauseState == "all")
        pauseState = JSC::Debugger::PauseOnAllExceptions;
    else if (stringPauseState == "uncaught")
        pauseState = JSC::Debugger::PauseOnUncaughtExceptions;
    else {
        errorString = ASCIILiteral("Unknown pause on exceptions mode: ") + stringPauseState;
        return;
    }

    m_scriptDebugServer.setPauseOnExceptionsState(static_cast<JSC::Debugger::PauseOnExceptionsState>(pauseState));
    if (m_scriptDebugServer.pauseOnExceptionsState() != pauseState)
        errorString = ASCIILiteral("Internal error. Could not change pause on exceptions state");
}

void InspectorDebuggerAgent::setPauseOnAssertions(ErrorString&, bool enabled)
{
    m_pauseOnAssertionFailures = enabled;
}

void InspectorDebuggerAgent::evaluateOnCallFrame(ErrorString& errorString, const String& callFrameId, const String& expression, const String* const objectGroup, const bool* const includeCommandLineAPI, const bool* const doNotPauseOnExceptionsAndMuteConsole, const bool* const returnByValue, const bool* generatePreview, const bool* saveResult, RefPtr<Inspector::Protocol::Runtime::RemoteObject>& result, Inspector::Protocol::OptOutput<bool>* wasThrown, Inspector::Protocol::OptOutput<int>* savedResultIndex)
{
    if (m_currentCallStack.hasNoValue()) {
        errorString = ASCIILiteral("Not paused");
        return;
    }

    InjectedScript injectedScript = m_injectedScriptManager.injectedScriptForObjectId(callFrameId);
    if (injectedScript.hasNoValue()) {
        errorString = ASCIILiteral("Could not find InjectedScript for callFrameId");
        return;
    }

    JSC::Debugger::PauseOnExceptionsState previousPauseOnExceptionsState = m_scriptDebugServer.pauseOnExceptionsState();
    if (doNotPauseOnExceptionsAndMuteConsole ? *doNotPauseOnExceptionsAndMuteConsole : false) {
        if (previousPauseOnExceptionsState != JSC::Debugger::DontPauseOnExceptions)
            m_scriptDebugServer.setPauseOnExceptionsState(JSC::Debugger::DontPauseOnExceptions);
        muteConsole();
    }

    injectedScript.evaluateOnCallFrame(errorString, m_currentCallStack, callFrameId, expression, objectGroup ? *objectGroup : "", includeCommandLineAPI ? *includeCommandLineAPI : false, returnByValue ? *returnByValue : false, generatePreview ? *generatePreview : false, saveResult ? *saveResult : false, &result, wasThrown, savedResultIndex);

    if (doNotPauseOnExceptionsAndMuteConsole ? *doNotPauseOnExceptionsAndMuteConsole : false) {
        unmuteConsole();
        if (m_scriptDebugServer.pauseOnExceptionsState() != previousPauseOnExceptionsState)
            m_scriptDebugServer.setPauseOnExceptionsState(previousPauseOnExceptionsState);
    }
}

void InspectorDebuggerAgent::setOverlayMessage(ErrorString&, const String*)
{
}

void InspectorDebuggerAgent::scriptExecutionBlockedByCSP(const String& directiveText)
{
    if (m_scriptDebugServer.pauseOnExceptionsState() != JSC::Debugger::DontPauseOnExceptions)
        breakProgram(DebuggerFrontendDispatcher::Reason::CSPViolation, buildCSPViolationPauseReason(directiveText));
}

Ref<Inspector::Protocol::Array<Inspector::Protocol::Debugger::CallFrame>> InspectorDebuggerAgent::currentCallFrames(const InjectedScript& injectedScript)
{
    ASSERT(!injectedScript.hasNoValue());
    if (injectedScript.hasNoValue())
        return Inspector::Protocol::Array<Inspector::Protocol::Debugger::CallFrame>::create();

    return injectedScript.wrapCallFrames(m_currentCallStack);
}

String InspectorDebuggerAgent::sourceMapURLForScript(const Script& script)
{
    return script.sourceMappingURL;
}

static bool isWebKitInjectedScript(const String& sourceURL)
{
    return sourceURL.startsWith("__InjectedScript_") && sourceURL.endsWith(".js");
}

void InspectorDebuggerAgent::didParseSource(JSC::SourceID sourceID, const Script& script)
{
    String scriptIDStr = String::number(sourceID);
    bool hasSourceURL = !script.sourceURL.isEmpty();
    String sourceURL = script.sourceURL;
    String sourceMappingURL = sourceMapURLForScript(script);

    const bool isModule = script.sourceProvider->sourceType() == JSC::SourceProviderSourceType::Module;
    const bool* isContentScript = script.isContentScript ? &script.isContentScript : nullptr;
    String* sourceURLParam = hasSourceURL ? &sourceURL : nullptr;
    String* sourceMapURLParam = sourceMappingURL.isEmpty() ? nullptr : &sourceMappingURL;

    m_frontendDispatcher->scriptParsed(scriptIDStr, script.url, script.startLine, script.startColumn, script.endLine, script.endColumn, isContentScript, sourceURLParam, sourceMapURLParam, isModule ? &isModule : nullptr);

    m_scripts.set(sourceID, script);

    if (hasSourceURL && isWebKitInjectedScript(sourceURL))
        m_scriptDebugServer.addToBlacklist(sourceID);

    String scriptURLForBreakpoints = hasSourceURL ? script.sourceURL : script.url;
    if (scriptURLForBreakpoints.isEmpty())
        return;

    for (auto& entry : m_javaScriptBreakpoints) {
        RefPtr<InspectorObject> breakpointObject = entry.value;

        bool isRegex;
        String url;
        breakpointObject->getBoolean(ASCIILiteral("isRegex"), isRegex);
        breakpointObject->getString(ASCIILiteral("url"), url);
        if (!matches(scriptURLForBreakpoints, url, isRegex))
            continue;

        ScriptBreakpoint scriptBreakpoint;
        breakpointObject->getInteger(ASCIILiteral("lineNumber"), scriptBreakpoint.lineNumber);
        breakpointObject->getInteger(ASCIILiteral("columnNumber"), scriptBreakpoint.columnNumber);
        breakpointObject->getString(ASCIILiteral("condition"), scriptBreakpoint.condition);
        breakpointObject->getBoolean(ASCIILiteral("autoContinue"), scriptBreakpoint.autoContinue);
        breakpointObject->getInteger(ASCIILiteral("ignoreCount"), scriptBreakpoint.ignoreCount);
        ErrorString errorString;
        RefPtr<InspectorArray> actions;
        breakpointObject->getArray(ASCIILiteral("actions"), actions);
        if (!breakpointActionsFromProtocol(errorString, actions, &scriptBreakpoint.actions)) {
            ASSERT_NOT_REACHED();
            continue;
        }

        JSC::Breakpoint breakpoint(sourceID, scriptBreakpoint.lineNumber, scriptBreakpoint.columnNumber, scriptBreakpoint.condition, scriptBreakpoint.autoContinue, scriptBreakpoint.ignoreCount);
        resolveBreakpoint(script, breakpoint);
        if (!breakpoint.resolved)
            continue;

        bool existing;
        setBreakpoint(breakpoint, existing);
        if (existing)
            continue;

        String breakpointIdentifier = entry.key;
        didSetBreakpoint(breakpoint, breakpointIdentifier, scriptBreakpoint);

        m_frontendDispatcher->breakpointResolved(breakpointIdentifier, buildDebuggerLocation(breakpoint));
    }
}

void InspectorDebuggerAgent::failedToParseSource(const String& url, const String& data, int firstLine, int errorLine, const String& errorMessage)
{
    m_frontendDispatcher->scriptFailedToParse(url, data, firstLine, errorLine, errorMessage);
}

void InspectorDebuggerAgent::didPause(JSC::ExecState& scriptState, JSC::JSValue callFrames, JSC::JSValue exceptionOrCaughtValue)
{
    ASSERT(!m_pausedScriptState);
    m_pausedScriptState = &scriptState;
    m_currentCallStack = { scriptState.vm(), callFrames };

    InjectedScript injectedScript = m_injectedScriptManager.injectedScriptFor(&scriptState);

    // If a high level pause pause reason is not already set, try to infer a reason from the debugger.
    if (m_breakReason == DebuggerFrontendDispatcher::Reason::Other) {
        switch (m_scriptDebugServer.reasonForPause()) {
        case JSC::Debugger::PausedForBreakpoint: {
            JSC::BreakpointID debuggerBreakpointId = m_scriptDebugServer.pausingBreakpointID();
            if (debuggerBreakpointId != m_continueToLocationBreakpointID) {
                m_breakReason = DebuggerFrontendDispatcher::Reason::Breakpoint;
                m_breakAuxData = buildBreakpointPauseReason(debuggerBreakpointId);
            }
            break;
        }
        case JSC::Debugger::PausedForDebuggerStatement:
            m_breakReason = DebuggerFrontendDispatcher::Reason::DebuggerStatement;
            m_breakAuxData = nullptr;
            break;
        case JSC::Debugger::PausedForException:
            m_breakReason = DebuggerFrontendDispatcher::Reason::Exception;
            m_breakAuxData = buildExceptionPauseReason(exceptionOrCaughtValue, injectedScript);
            break;
        case JSC::Debugger::PausedAtStatement:
        case JSC::Debugger::PausedAtExpression:
        case JSC::Debugger::PausedBeforeReturn:
        case JSC::Debugger::PausedAtEndOfProgram:
            // Pause was just stepping. Nothing to report.
            break;
        case JSC::Debugger::NotPaused:
            ASSERT_NOT_REACHED();
            break;
        }
    }

    // Set $exception to the exception or caught value.
    if (exceptionOrCaughtValue && !injectedScript.hasNoValue()) {
        injectedScript.setExceptionValue(exceptionOrCaughtValue);
        m_hasExceptionValue = true;
    }

    m_conditionToDispatchResumed = ShouldDispatchResumed::No;
    m_enablePauseWhenIdle = false;

    RefPtr<Inspector::Protocol::Console::StackTrace> asyncStackTrace;
    if (m_currentAsyncCallIdentifier) {
        auto it = m_pendingAsyncCalls.find(m_currentAsyncCallIdentifier.value());
        if (it != m_pendingAsyncCalls.end())
            asyncStackTrace = it->value->buildInspectorObject();
    }

    m_frontendDispatcher->paused(currentCallFrames(injectedScript), m_breakReason, m_breakAuxData, asyncStackTrace);

    m_javaScriptPauseScheduled = false;

    if (m_continueToLocationBreakpointID != JSC::noBreakpointID) {
        m_scriptDebugServer.removeBreakpoint(m_continueToLocationBreakpointID);
        m_continueToLocationBreakpointID = JSC::noBreakpointID;
    }

    RefPtr<Stopwatch> stopwatch = m_injectedScriptManager.inspectorEnvironment().executionStopwatch();
    if (stopwatch && stopwatch->isActive()) {
        stopwatch->stop();
        m_didPauseStopwatch = true;
    }
}

void InspectorDebuggerAgent::breakpointActionSound(int breakpointActionIdentifier)
{
    m_frontendDispatcher->playBreakpointActionSound(breakpointActionIdentifier);
}

void InspectorDebuggerAgent::breakpointActionProbe(JSC::ExecState& scriptState, const ScriptBreakpointAction& action, unsigned batchId, unsigned sampleId, JSC::JSValue sample)
{
    InjectedScript injectedScript = m_injectedScriptManager.injectedScriptFor(&scriptState);
    auto payload = injectedScript.wrapObject(sample, objectGroupForBreakpointAction(action), true);
    auto result = Protocol::Debugger::ProbeSample::create()
        .setProbeId(action.identifier)
        .setBatchId(batchId)
        .setSampleId(sampleId)
        .setTimestamp(m_injectedScriptManager.inspectorEnvironment().executionStopwatch()->elapsedTime())
        .setPayload(WTFMove(payload))
        .release();
    m_frontendDispatcher->didSampleProbe(WTFMove(result));
}

void InspectorDebuggerAgent::didContinue()
{
    if (m_didPauseStopwatch) {
        m_didPauseStopwatch = false;
        m_injectedScriptManager.inspectorEnvironment().executionStopwatch()->start();
    }

    m_pausedScriptState = nullptr;
    m_currentCallStack = { };
    m_injectedScriptManager.releaseObjectGroup(InspectorDebuggerAgent::backtraceObjectGroup);
    clearBreakDetails();
    clearExceptionValue();

    if (m_conditionToDispatchResumed == ShouldDispatchResumed::WhenContinued)
        m_frontendDispatcher->resumed();
}

void InspectorDebuggerAgent::breakProgram(DebuggerFrontendDispatcher::Reason breakReason, RefPtr<InspectorObject>&& data)
{
    m_breakReason = breakReason;
    m_breakAuxData = WTFMove(data);
    m_scriptDebugServer.breakProgram();
}

void InspectorDebuggerAgent::clearInspectorBreakpointState()
{
    ErrorString dummyError;
    Vector<String> breakpointIdentifiers;
    copyKeysToVector(m_breakpointIdentifierToDebugServerBreakpointIDs, breakpointIdentifiers);
    for (const String& identifier : breakpointIdentifiers)
        removeBreakpoint(dummyError, identifier);

    m_javaScriptBreakpoints.clear();

    clearDebuggerBreakpointState();
}

void InspectorDebuggerAgent::clearDebuggerBreakpointState()
{
    {
        JSC::JSLockHolder holder(m_scriptDebugServer.vm());
        m_scriptDebugServer.clearBreakpointActions();
        m_scriptDebugServer.clearBreakpoints();
        m_scriptDebugServer.clearBlacklist();
    }

    m_pausedScriptState = nullptr;
    m_currentCallStack = { };
    m_scripts.clear();
    m_breakpointIdentifierToDebugServerBreakpointIDs.clear();
    m_debuggerBreakpointIdentifierToInspectorBreakpointIdentifier.clear();
    m_continueToLocationBreakpointID = JSC::noBreakpointID;
    clearBreakDetails();
    m_javaScriptPauseScheduled = false;
    m_hasExceptionValue = false;

    if (isPaused()) {
        m_scriptDebugServer.continueProgram();
        m_frontendDispatcher->resumed();
    }
}

void InspectorDebuggerAgent::didClearGlobalObject()
{
    // Clear breakpoints from the debugger, but keep the inspector's model of which
    // pages have what breakpoints, as the mapping is only sent to DebuggerAgent once.
    clearDebuggerBreakpointState();

    clearAsyncStackTraceData();

    m_frontendDispatcher->globalObjectCleared();
}

bool InspectorDebuggerAgent::assertPaused(ErrorString& errorString)
{
    if (!m_pausedScriptState) {
        errorString = ASCIILiteral("Can only perform operation while paused.");
        return false;
    }

    return true;
}

void InspectorDebuggerAgent::clearBreakDetails()
{
    m_breakReason = DebuggerFrontendDispatcher::Reason::Other;
    m_breakAuxData = nullptr;
}

void InspectorDebuggerAgent::clearExceptionValue()
{
    if (m_hasExceptionValue) {
        m_injectedScriptManager.clearExceptionValue();
        m_hasExceptionValue = false;
    }
}

void InspectorDebuggerAgent::clearAsyncStackTraceData()
{
    m_pendingAsyncCalls.clear();
    m_currentAsyncCallIdentifier = std::nullopt;
}

} // namespace Inspector
