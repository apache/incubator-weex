/*
 * Copyright (C) 2014 Apple Inc. All rights reserved.
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
#include "JSGlobalObjectConsoleClient.h"

#include "ConsoleMessage.h"
#include "InspectorConsoleAgent.h"
#include "InspectorDebuggerAgent.h"
#include "InspectorScriptProfilerAgent.h"
#include "ScriptArguments.h"
#include "ScriptCallStack.h"
#include "ScriptCallStackFactory.h"

using namespace JSC;

namespace Inspector {

#if !LOG_DISABLED
static bool sLogToSystemConsole = true;
#else
static bool sLogToSystemConsole = false;
#endif

bool JSGlobalObjectConsoleClient::logToSystemConsole()
{
    return sLogToSystemConsole;
}

void JSGlobalObjectConsoleClient::setLogToSystemConsole(bool shouldLog)
{
    sLogToSystemConsole = shouldLog;
}

JSGlobalObjectConsoleClient::JSGlobalObjectConsoleClient(InspectorConsoleAgent* consoleAgent, InspectorDebuggerAgent* debuggerAgent, InspectorScriptProfilerAgent* scriptProfilerAgent)
    : ConsoleClient()
    , m_consoleAgent(consoleAgent)
    , m_debuggerAgent(debuggerAgent)
    , m_scriptProfilerAgent(scriptProfilerAgent)
{
}

void JSGlobalObjectConsoleClient::messageWithTypeAndLevel(MessageType type, MessageLevel level, JSC::ExecState* exec, Ref<ScriptArguments>&& arguments)
{
    if (JSGlobalObjectConsoleClient::logToSystemConsole())
        ConsoleClient::printConsoleMessageWithArguments(MessageSource::ConsoleAPI, type, level, exec, arguments.copyRef());

    String message;
    arguments->getFirstArgumentAsString(message);
    m_consoleAgent->addMessageToConsole(std::make_unique<ConsoleMessage>(MessageSource::ConsoleAPI, type, level, message, WTFMove(arguments), exec));
}

void JSGlobalObjectConsoleClient::count(ExecState* exec, Ref<ScriptArguments>&& arguments)
{
    m_consoleAgent->count(exec, WTFMove(arguments));
}

void JSGlobalObjectConsoleClient::profile(JSC::ExecState*, const String& title)
{
    if (!m_consoleAgent->enabled())
        return;

    // Allow duplicate unnamed profiles. Disallow duplicate named profiles.
    if (!title.isEmpty()) {
        for (auto& existingTitle : m_profiles) {
            if (existingTitle == title) {
                // FIXME: Send an enum to the frontend for localization?
                String warning = title.isEmpty() ? ASCIILiteral("Unnamed Profile already exists") : makeString("Profile \"", title, "\" already exists");
                m_consoleAgent->addMessageToConsole(std::make_unique<ConsoleMessage>(MessageSource::ConsoleAPI, MessageType::Profile, MessageLevel::Warning, warning));
                return;
            }
        }
    }

    m_profiles.append(title);
    startConsoleProfile();
}

void JSGlobalObjectConsoleClient::profileEnd(JSC::ExecState*, const String& title)
{
    if (!m_consoleAgent->enabled())
        return;

    // Stop profiles in reverse order. If the title is empty, then stop the last profile.
    // Otherwise, match the title of the profile to stop.
    for (ptrdiff_t i = m_profiles.size() - 1; i >= 0; --i) {
        if (title.isEmpty() || m_profiles[i] == title) {
            m_profiles.remove(i);
            if (m_profiles.isEmpty())
                stopConsoleProfile();
            return;
        }
    }

    // FIXME: Send an enum to the frontend for localization?
    String warning = title.isEmpty() ? ASCIILiteral("No profiles exist") : makeString("Profile \"", title, "\" does not exist");
    m_consoleAgent->addMessageToConsole(std::make_unique<ConsoleMessage>(MessageSource::ConsoleAPI, MessageType::ProfileEnd, MessageLevel::Warning, warning));
}

void JSGlobalObjectConsoleClient::startConsoleProfile()
{
    // FIXME: <https://webkit.org/b/158753> Generalize the concept of Instruments on the backend to work equally for JSContext and Web inspection
    m_scriptProfilerAgent->programmaticCaptureStarted();

    m_profileRestoreBreakpointActiveValue = m_debuggerAgent->breakpointsActive();

    ErrorString unused;
    m_debuggerAgent->setBreakpointsActive(unused, false);

    const bool includeSamples = true;
    m_scriptProfilerAgent->startTracking(unused, &includeSamples);
}

void JSGlobalObjectConsoleClient::stopConsoleProfile()
{
    ErrorString unused;
    m_scriptProfilerAgent->stopTracking(unused);

    m_debuggerAgent->setBreakpointsActive(unused, m_profileRestoreBreakpointActiveValue);

    // FIXME: <https://webkit.org/b/158753> Generalize the concept of Instruments on the backend to work equally for JSContext and Web inspection
    m_scriptProfilerAgent->programmaticCaptureStopped();
}

void JSGlobalObjectConsoleClient::takeHeapSnapshot(JSC::ExecState*, const String& title)
{
    m_consoleAgent->takeHeapSnapshot(title);
}

void JSGlobalObjectConsoleClient::time(ExecState*, const String& title)
{
    m_consoleAgent->startTiming(title);
}

void JSGlobalObjectConsoleClient::timeEnd(ExecState* exec, const String& title)
{
    m_consoleAgent->stopTiming(title, createScriptCallStackForConsole(exec, 1));
}

void JSGlobalObjectConsoleClient::timeStamp(ExecState*, Ref<ScriptArguments>&&)
{
    // FIXME: JSContext inspection needs a timeline.
    warnUnimplemented(ASCIILiteral("console.timeStamp"));
}

void JSGlobalObjectConsoleClient::warnUnimplemented(const String& method)
{
    String message = method + " is currently ignored in JavaScript context inspection.";
    m_consoleAgent->addMessageToConsole(std::make_unique<ConsoleMessage>(MessageSource::ConsoleAPI, MessageType::Log, MessageLevel::Warning, message));
}

} // namespace Inspector
