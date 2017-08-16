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

#include "config.h"
#include "InspectorConsoleAgent.h"

#include "ConsoleMessage.h"
#include "InjectedScriptManager.h"
#include "InspectorFrontendRouter.h"
#include "InspectorHeapAgent.h"
#include "ScriptArguments.h"
#include "ScriptCallFrame.h"
#include "ScriptCallStack.h"
#include "ScriptCallStackFactory.h"
#include "ScriptObject.h"
#include <wtf/CurrentTime.h>
#include <wtf/text/WTFString.h>

namespace Inspector {

static const unsigned maximumConsoleMessages = 100;
static const int expireConsoleMessagesStep = 10;

InspectorConsoleAgent::InspectorConsoleAgent(AgentContext& context, InspectorHeapAgent* heapAgent)
    : InspectorAgentBase(ASCIILiteral("Console"))
    , m_injectedScriptManager(context.injectedScriptManager)
    , m_frontendDispatcher(std::make_unique<ConsoleFrontendDispatcher>(context.frontendRouter))
    , m_backendDispatcher(ConsoleBackendDispatcher::create(context.backendDispatcher, this))
    , m_heapAgent(heapAgent)
{
}

InspectorConsoleAgent::~InspectorConsoleAgent()
{
}

void InspectorConsoleAgent::didCreateFrontendAndBackend(FrontendRouter*, BackendDispatcher*)
{
}

void InspectorConsoleAgent::willDestroyFrontendAndBackend(DisconnectReason)
{
    String errorString;
    disable(errorString);
}

void InspectorConsoleAgent::discardValues()
{
    m_consoleMessages.clear();
}

void InspectorConsoleAgent::enable(ErrorString&)
{
    if (m_enabled)
        return;

    m_enabled = true;

    if (m_expiredConsoleMessageCount) {
        ConsoleMessage expiredMessage(MessageSource::Other, MessageType::Log, MessageLevel::Warning, String::format("%d console messages are not shown.", m_expiredConsoleMessageCount));
        expiredMessage.addToFrontend(*m_frontendDispatcher, m_injectedScriptManager, false);
    }

    size_t messageCount = m_consoleMessages.size();
    for (size_t i = 0; i < messageCount; ++i)
        m_consoleMessages[i]->addToFrontend(*m_frontendDispatcher, m_injectedScriptManager, false);
}

void InspectorConsoleAgent::disable(ErrorString&)
{
    if (!m_enabled)
        return;

    m_enabled = false;
}

void InspectorConsoleAgent::clearMessages(ErrorString&)
{
    m_consoleMessages.clear();
    m_expiredConsoleMessageCount = 0;
    m_previousMessage = nullptr;

    m_injectedScriptManager.releaseObjectGroup(ASCIILiteral("console"));

    if (m_enabled)
        m_frontendDispatcher->messagesCleared();
}

void InspectorConsoleAgent::reset()
{
    ErrorString unused;
    clearMessages(unused);

    m_times.clear();
    m_counts.clear();
}

void InspectorConsoleAgent::addMessageToConsole(std::unique_ptr<ConsoleMessage> message)
{
    if (!m_injectedScriptManager.inspectorEnvironment().developerExtrasEnabled())
        return;

    if (message->type() == MessageType::Clear) {
        ErrorString unused;
        clearMessages(unused);
    }

    addConsoleMessage(WTFMove(message));
}

void InspectorConsoleAgent::startTiming(const String& title)
{
    ASSERT(!title.isNull());
    if (title.isNull())
        return;

    auto result = m_times.add(title, monotonicallyIncreasingTime());

    if (!result.isNewEntry) {
        // FIXME: Send an enum to the frontend for localization?
        String warning = makeString("Timer \"", title, "\" already exists");
        addMessageToConsole(std::make_unique<ConsoleMessage>(MessageSource::ConsoleAPI, MessageType::Timing, MessageLevel::Warning, warning));
    }
}

void InspectorConsoleAgent::stopTiming(const String& title, Ref<ScriptCallStack>&& callStack)
{
    ASSERT(!title.isNull());
    if (title.isNull())
        return;

    auto it = m_times.find(title);
    if (it == m_times.end()) {
        // FIXME: Send an enum to the frontend for localization?
        String warning = makeString("Timer \"", title, "\" does not exist");
        addMessageToConsole(std::make_unique<ConsoleMessage>(MessageSource::ConsoleAPI, MessageType::Timing, MessageLevel::Warning, warning));
        return;
    }

    double startTime = it->value;
    m_times.remove(it);

    double elapsed = monotonicallyIncreasingTime() - startTime;
    String message = title + String::format(": %.3fms", elapsed * 1000);
    addMessageToConsole(std::make_unique<ConsoleMessage>(MessageSource::ConsoleAPI, MessageType::Timing, MessageLevel::Debug, message, WTFMove(callStack)));
}

void InspectorConsoleAgent::takeHeapSnapshot(const String& title)
{
    if (!m_injectedScriptManager.inspectorEnvironment().developerExtrasEnabled())
        return;

    ErrorString ignored;
    double timestamp;
    String snapshotData;
    m_heapAgent->snapshot(ignored, &timestamp, &snapshotData);

    m_frontendDispatcher->heapSnapshot(timestamp, snapshotData, title.isEmpty() ? nullptr : &title);
}

void InspectorConsoleAgent::count(JSC::ExecState* state, Ref<ScriptArguments>&& arguments)
{
    Ref<ScriptCallStack> callStack = createScriptCallStackForConsole(state, ScriptCallStack::maxCallStackSizeToCapture);

    String title;
    String identifier;
    if (!arguments->argumentCount()) {
        // '@' prefix for engine generated labels.
        title = ASCIILiteral("Global");
        identifier = makeString('@', title);
    } else {
        // '#' prefix for user labels.
        arguments->getFirstArgumentAsString(title);
        identifier = makeString('#', title);
    }

    auto result = m_counts.add(identifier, 1);
    if (!result.isNewEntry)
        result.iterator->value += 1;

    // FIXME: Web Inspector should have a better UI for counters, but for now we just log an updated counter value.

    String message = makeString(title, ": ", String::number(result.iterator->value));
    addMessageToConsole(std::make_unique<ConsoleMessage>(MessageSource::ConsoleAPI, MessageType::Log, MessageLevel::Debug, message, WTFMove(callStack)));
}

static bool isGroupMessage(MessageType type)
{
    return type == MessageType::StartGroup
        || type == MessageType::StartGroupCollapsed
        || type == MessageType::EndGroup;
}

void InspectorConsoleAgent::addConsoleMessage(std::unique_ptr<ConsoleMessage> consoleMessage)
{
    ASSERT(m_injectedScriptManager.inspectorEnvironment().developerExtrasEnabled());
    ASSERT_ARG(consoleMessage, consoleMessage);

    if (m_previousMessage && !isGroupMessage(m_previousMessage->type()) && m_previousMessage->isEqual(consoleMessage.get())) {
        m_previousMessage->incrementCount();
        if (m_enabled)
            m_previousMessage->updateRepeatCountInConsole(*m_frontendDispatcher);
    } else {
        m_previousMessage = consoleMessage.get();
        m_consoleMessages.append(WTFMove(consoleMessage));
        if (m_enabled)
            m_previousMessage->addToFrontend(*m_frontendDispatcher, m_injectedScriptManager, true);
    }

    if (m_consoleMessages.size() >= maximumConsoleMessages) {
        m_expiredConsoleMessageCount += expireConsoleMessagesStep;
        m_consoleMessages.remove(0, expireConsoleMessagesStep);
    }
}

} // namespace Inspector
