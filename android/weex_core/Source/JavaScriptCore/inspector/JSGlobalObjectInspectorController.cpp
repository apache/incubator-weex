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

#include "config.h"
#include "JSGlobalObjectInspectorController.h"

#include "Completion.h"
#include "ConsoleMessage.h"
#include "ErrorHandlingScope.h"
#include "Exception.h"
#include "InjectedScriptHost.h"
#include "InjectedScriptManager.h"
#include "InspectorAgent.h"
#include "InspectorBackendDispatcher.h"
#include "InspectorFrontendChannel.h"
#include "InspectorFrontendRouter.h"
#include "InspectorHeapAgent.h"
#include "InspectorScriptProfilerAgent.h"
#include "JSCInlines.h"
#include "JSGlobalObject.h"
#include "JSGlobalObjectConsoleAgent.h"
#include "JSGlobalObjectConsoleClient.h"
#include "JSGlobalObjectDebuggerAgent.h"
#include "JSGlobalObjectRuntimeAgent.h"
#include "ScriptArguments.h"
#include "ScriptCallStack.h"
#include "ScriptCallStackFactory.h"
#include <wtf/Stopwatch.h>

#if OS(DARWIN) || (OS(LINUX) && !PLATFORM(GTK))
#include <cxxabi.h>
#include <dlfcn.h>
// #include <execinfo.h>
#endif

#if ENABLE(REMOTE_INSPECTOR)
#include "JSGlobalObjectDebuggable.h"
#include "RemoteInspector.h"
#endif

using namespace JSC;

namespace Inspector {

JSGlobalObjectInspectorController::JSGlobalObjectInspectorController(JSGlobalObject& globalObject)
    : m_globalObject(globalObject)
    , m_injectedScriptManager(std::make_unique<InjectedScriptManager>(*this, InjectedScriptHost::create()))
    , m_executionStopwatch(Stopwatch::create())
    , m_scriptDebugServer(globalObject)
    , m_frontendRouter(FrontendRouter::create())
    , m_backendDispatcher(BackendDispatcher::create(m_frontendRouter.copyRef()))
{
    AgentContext baseContext = {
        *this,
        *m_injectedScriptManager,
        m_frontendRouter.get(),
        m_backendDispatcher.get()
    };

    JSAgentContext context = {
        baseContext,
        globalObject
    };

    auto inspectorAgent = std::make_unique<InspectorAgent>(context);
    auto runtimeAgent = std::make_unique<JSGlobalObjectRuntimeAgent>(context);
    auto heapAgent = std::make_unique<InspectorHeapAgent>(context);
    auto consoleAgent = std::make_unique<JSGlobalObjectConsoleAgent>(context, heapAgent.get());
    auto debuggerAgent = std::make_unique<JSGlobalObjectDebuggerAgent>(context, consoleAgent.get());
    auto scriptProfilerAgent = std::make_unique<InspectorScriptProfilerAgent>(context);

    m_inspectorAgent = inspectorAgent.get();
    m_debuggerAgent = debuggerAgent.get();
    m_consoleAgent = consoleAgent.get();
    m_consoleClient = std::make_unique<JSGlobalObjectConsoleClient>(m_consoleAgent, m_debuggerAgent, scriptProfilerAgent.get());

    m_agents.append(WTFMove(inspectorAgent));
    m_agents.append(WTFMove(runtimeAgent));
    m_agents.append(WTFMove(consoleAgent));
    m_agents.append(WTFMove(debuggerAgent));
    m_agents.append(WTFMove(heapAgent));
    m_agents.append(WTFMove(scriptProfilerAgent));

    m_executionStopwatch->start();
}

JSGlobalObjectInspectorController::~JSGlobalObjectInspectorController()
{
#if ENABLE(INSPECTOR_ALTERNATE_DISPATCHERS)
    if (m_augmentingClient)
        m_augmentingClient->inspectorControllerDestroyed();
#endif
}

void JSGlobalObjectInspectorController::globalObjectDestroyed()
{
    ASSERT(!m_frontendRouter->hasFrontends());

    m_injectedScriptManager->disconnect();

    m_agents.discardValues();
}

void JSGlobalObjectInspectorController::connectFrontend(FrontendChannel* frontendChannel, bool isAutomaticInspection)
{
    ASSERT_ARG(frontendChannel, frontendChannel);

    m_isAutomaticInspection = isAutomaticInspection;

    bool connectedFirstFrontend = !m_frontendRouter->hasFrontends();
    m_frontendRouter->connectFrontend(frontendChannel);

    if (!connectedFirstFrontend)
        return;

    // Keep the JSGlobalObject and VM alive while we are debugging it.
    m_strongVM = &m_globalObject.vm();
    m_strongGlobalObject.set(m_globalObject.vm(), &m_globalObject);

    // FIXME: change this to notify agents which frontend has connected (by id).
    m_agents.didCreateFrontendAndBackend(nullptr, nullptr);

#if ENABLE(INSPECTOR_ALTERNATE_DISPATCHERS)
    m_inspectorAgent->activateExtraDomains(m_agents.extraDomains());

    if (m_augmentingClient)
        m_augmentingClient->inspectorConnected();
#endif
}

void JSGlobalObjectInspectorController::disconnectFrontend(FrontendChannel* frontendChannel)
{
    ASSERT_ARG(frontendChannel, frontendChannel);

    // FIXME: change this to notify agents which frontend has disconnected (by id).
    m_agents.willDestroyFrontendAndBackend(DisconnectReason::InspectorDestroyed);

    m_frontendRouter->disconnectFrontend(frontendChannel);

    m_isAutomaticInspection = false;

    bool disconnectedLastFrontend = !m_frontendRouter->hasFrontends();
    if (!disconnectedLastFrontend)
        return;

#if ENABLE(INSPECTOR_ALTERNATE_DISPATCHERS)
    if (m_augmentingClient)
        m_augmentingClient->inspectorDisconnected();
#endif

    // Remove our JSGlobalObject and VM references, we are done debugging it.
    m_strongGlobalObject.clear();
    m_strongVM = nullptr;
}

void JSGlobalObjectInspectorController::dispatchMessageFromFrontend(const String& message)
{
    m_backendDispatcher->dispatch(message);
}

void JSGlobalObjectInspectorController::pause()
{
    ErrorString dummyError;
    m_debuggerAgent->enable(dummyError);
    m_debuggerAgent->pause(dummyError);
}

void JSGlobalObjectInspectorController::appendAPIBacktrace(ScriptCallStack& callStack)
{
#if OS(DARWIN) || (OS(LINUX) && !PLATFORM(GTK))
    static const int framesToShow = 31;
    static const int framesToSkip = 3; // WTFGetBacktrace, appendAPIBacktrace, reportAPIException.

    void* samples[framesToShow + framesToSkip];
    int frames = framesToShow + framesToSkip;
    WTFGetBacktrace(samples, &frames);

    void** stack = samples + framesToSkip;
    int size = frames - framesToSkip;
    for (int i = 0; i < size; ++i) {
        const char* mangledName = nullptr;
        char* cxaDemangled = nullptr;
        Dl_info info;
        if (dladdr(stack[i], &info) && info.dli_sname)
            mangledName = info.dli_sname;
        if (mangledName)
            cxaDemangled = abi::__cxa_demangle(mangledName, nullptr, nullptr, nullptr);
        if (mangledName || cxaDemangled)
            callStack.append(ScriptCallFrame(cxaDemangled ? cxaDemangled : mangledName, ASCIILiteral("[native code]"), noSourceID, 0, 0));
        else
            callStack.append(ScriptCallFrame(ASCIILiteral("?"), ASCIILiteral("[native code]"), noSourceID, 0, 0));
        free(cxaDemangled);
    }
#else
    UNUSED_PARAM(callStack);
#endif
}

void JSGlobalObjectInspectorController::reportAPIException(ExecState* exec, Exception* exception)
{
    VM& vm = exec->vm();
    if (isTerminatedExecutionException(vm, exception))
        return;

    auto scope = DECLARE_CATCH_SCOPE(vm);
    ErrorHandlingScope errorScope(vm);

    Ref<ScriptCallStack> callStack = createScriptCallStackFromException(exec, exception, ScriptCallStack::maxCallStackSizeToCapture);
    if (includesNativeCallStackWhenReportingExceptions())
        appendAPIBacktrace(callStack.get());

    // FIXME: <http://webkit.org/b/115087> Web Inspector: Should not evaluate JavaScript handling exceptions
    // If this is a custom exception object, call toString on it to try and get a nice string representation for the exception.
    String errorMessage = exception->value().toWTFString(exec);
    scope.clearException();

    if (JSGlobalObjectConsoleClient::logToSystemConsole()) {
        if (callStack->size()) {
            const ScriptCallFrame& callFrame = callStack->at(0);
            ConsoleClient::printConsoleMessage(MessageSource::JS, MessageType::Log, MessageLevel::Error, errorMessage, callFrame.sourceURL(), callFrame.lineNumber(), callFrame.columnNumber());
        } else
            ConsoleClient::printConsoleMessage(MessageSource::JS, MessageType::Log, MessageLevel::Error, errorMessage, String(), 0, 0);
    }

    m_consoleAgent->addMessageToConsole(std::make_unique<ConsoleMessage>(MessageSource::JS, MessageType::Log, MessageLevel::Error, errorMessage, WTFMove(callStack)));
}

ConsoleClient* JSGlobalObjectInspectorController::consoleClient() const
{
    return m_consoleClient.get();
}

bool JSGlobalObjectInspectorController::developerExtrasEnabled() const
{
#if ENABLE(REMOTE_INSPECTOR)
    if (!RemoteInspector::singleton().enabled())
        return false;

    if (!m_globalObject.inspectorDebuggable().remoteDebuggingAllowed())
        return false;
#endif

    return true;
}

InspectorFunctionCallHandler JSGlobalObjectInspectorController::functionCallHandler() const
{
    return JSC::call;
}

InspectorEvaluateHandler JSGlobalObjectInspectorController::evaluateHandler() const
{
    return JSC::evaluate;
}

void JSGlobalObjectInspectorController::frontendInitialized()
{
#if ENABLE(REMOTE_INSPECTOR)
    if (m_isAutomaticInspection)
        m_globalObject.inspectorDebuggable().unpauseForInitializedInspector();
#endif
}

Ref<Stopwatch> JSGlobalObjectInspectorController::executionStopwatch()
{
    return m_executionStopwatch.copyRef();
}

JSGlobalObjectScriptDebugServer& JSGlobalObjectInspectorController::scriptDebugServer()
{
    return m_scriptDebugServer;
}

VM& JSGlobalObjectInspectorController::vm()
{
    return m_globalObject.vm();
}

#if ENABLE(INSPECTOR_ALTERNATE_DISPATCHERS)
void JSGlobalObjectInspectorController::appendExtraAgent(std::unique_ptr<InspectorAgentBase> agent)
{
    String domainName = agent->domainName();

    // FIXME: change this to notify agents which frontend has connected (by id).
    agent->didCreateFrontendAndBackend(nullptr, nullptr);

    m_agents.appendExtraAgent(WTFMove(agent));

    m_inspectorAgent->activateExtraDomain(domainName);
}
#endif

} // namespace Inspector
