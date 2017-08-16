/*
 * Copyright (C) 2007, 2008, 2014, 2015 Apple Inc. All rights reserved.
 * Copyright (C) 2008 Matt Lilek <webkit@mattlilek.com>
 * Copyright (C) 2009, 2010 Google Inc. All rights reserved.
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
#include "ConsoleMessage.h"

#include "IdentifiersFactory.h"
#include "InjectedScript.h"
#include "InjectedScriptManager.h"
#include "InspectorValues.h"
#include "ScriptArguments.h"
#include "ScriptCallFrame.h"
#include "ScriptCallStack.h"
#include "ScriptCallStackFactory.h"
#include "ScriptValue.h"

namespace Inspector {

ConsoleMessage::ConsoleMessage(MessageSource source, MessageType type, MessageLevel level, const String& message, unsigned long requestIdentifier)
    : m_source(source)
    , m_type(type)
    , m_level(level)
    , m_message(message)
    , m_url()
    , m_requestId(IdentifiersFactory::requestId(requestIdentifier))
{
}

ConsoleMessage::ConsoleMessage(MessageSource source, MessageType type, MessageLevel level, const String& message, const String& url, unsigned line, unsigned column, JSC::ExecState* state, unsigned long requestIdentifier)
    : m_source(source)
    , m_type(type)
    , m_level(level)
    , m_message(message)
    , m_url(url)
    , m_line(line)
    , m_column(column)
    , m_requestId(IdentifiersFactory::requestId(requestIdentifier))
{
    autogenerateMetadata(state);
}

ConsoleMessage::ConsoleMessage(MessageSource source, MessageType type, MessageLevel level, const String& message, Ref<ScriptCallStack>&& callStack, unsigned long requestIdentifier)
    : m_source(source)
    , m_type(type)
    , m_level(level)
    , m_message(message)
    , m_callStack(WTFMove(callStack))
    , m_url()
    , m_requestId(IdentifiersFactory::requestId(requestIdentifier))
{
    const ScriptCallFrame* frame = m_callStack ? m_callStack->firstNonNativeCallFrame() : nullptr;
    if (frame) {
        m_url = frame->sourceURL();
        m_line = frame->lineNumber();
        m_column = frame->columnNumber();
    }
}

ConsoleMessage::ConsoleMessage(MessageSource source, MessageType type, MessageLevel level, const String& message, Ref<ScriptArguments>&& arguments, JSC::ExecState* state, unsigned long requestIdentifier)
    : m_source(source)
    , m_type(type)
    , m_level(level)
    , m_message(message)
    , m_arguments(WTFMove(arguments))
    , m_url()
    , m_requestId(IdentifiersFactory::requestId(requestIdentifier))
{
    autogenerateMetadata(state);
}

ConsoleMessage::~ConsoleMessage()
{
}

void ConsoleMessage::autogenerateMetadata(JSC::ExecState* state)
{
    if (!state)
        return;

    if (m_type == MessageType::EndGroup)
        return;

    // FIXME: Should this really be using "for console" in the generic ConsoleMessage autogeneration? This can skip the first frame.
    m_callStack = createScriptCallStackForConsole(state, ScriptCallStack::maxCallStackSizeToCapture);

    if (const ScriptCallFrame* frame = m_callStack->firstNonNativeCallFrame()) {
        m_url = frame->sourceURL();
        m_line = frame->lineNumber();
        m_column = frame->columnNumber();
        return;
    }
}

static Inspector::Protocol::Console::ConsoleMessage::Source messageSourceValue(MessageSource source)
{
    switch (source) {
    case MessageSource::XML: return Inspector::Protocol::Console::ConsoleMessage::Source::XML;
    case MessageSource::JS: return Inspector::Protocol::Console::ConsoleMessage::Source::Javascript;
    case MessageSource::Network: return Inspector::Protocol::Console::ConsoleMessage::Source::Network;
    case MessageSource::ConsoleAPI: return Inspector::Protocol::Console::ConsoleMessage::Source::ConsoleAPI;
    case MessageSource::Storage: return Inspector::Protocol::Console::ConsoleMessage::Source::Storage;
    case MessageSource::AppCache: return Inspector::Protocol::Console::ConsoleMessage::Source::Appcache;
    case MessageSource::Rendering: return Inspector::Protocol::Console::ConsoleMessage::Source::Rendering;
    case MessageSource::CSS: return Inspector::Protocol::Console::ConsoleMessage::Source::CSS;
    case MessageSource::Security: return Inspector::Protocol::Console::ConsoleMessage::Source::Security;
    case MessageSource::ContentBlocker: return Inspector::Protocol::Console::ConsoleMessage::Source::ContentBlocker;
    case MessageSource::Other: return Inspector::Protocol::Console::ConsoleMessage::Source::Other;
    }
    return Inspector::Protocol::Console::ConsoleMessage::Source::Other;
}

static Inspector::Protocol::Console::ConsoleMessage::Type messageTypeValue(MessageType type)
{
    switch (type) {
    case MessageType::Log: return Inspector::Protocol::Console::ConsoleMessage::Type::Log;
    case MessageType::Clear: return Inspector::Protocol::Console::ConsoleMessage::Type::Clear;
    case MessageType::Dir: return Inspector::Protocol::Console::ConsoleMessage::Type::Dir;
    case MessageType::DirXML: return Inspector::Protocol::Console::ConsoleMessage::Type::DirXML;
    case MessageType::Table: return Inspector::Protocol::Console::ConsoleMessage::Type::Table;
    case MessageType::Trace: return Inspector::Protocol::Console::ConsoleMessage::Type::Trace;
    case MessageType::StartGroup: return Inspector::Protocol::Console::ConsoleMessage::Type::StartGroup;
    case MessageType::StartGroupCollapsed: return Inspector::Protocol::Console::ConsoleMessage::Type::StartGroupCollapsed;
    case MessageType::EndGroup: return Inspector::Protocol::Console::ConsoleMessage::Type::EndGroup;
    case MessageType::Assert: return Inspector::Protocol::Console::ConsoleMessage::Type::Assert;
    case MessageType::Timing: return Inspector::Protocol::Console::ConsoleMessage::Type::Timing;
    case MessageType::Profile: return Inspector::Protocol::Console::ConsoleMessage::Type::Profile;
    case MessageType::ProfileEnd: return Inspector::Protocol::Console::ConsoleMessage::Type::ProfileEnd;
    }
    return Inspector::Protocol::Console::ConsoleMessage::Type::Log;
}

static Inspector::Protocol::Console::ConsoleMessage::Level messageLevelValue(MessageLevel level)
{
    switch (level) {
    case MessageLevel::Log: return Inspector::Protocol::Console::ConsoleMessage::Level::Log;
    case MessageLevel::Info: return Inspector::Protocol::Console::ConsoleMessage::Level::Info;
    case MessageLevel::Warning: return Inspector::Protocol::Console::ConsoleMessage::Level::Warning;
    case MessageLevel::Error: return Inspector::Protocol::Console::ConsoleMessage::Level::Error;
    case MessageLevel::Debug: return Inspector::Protocol::Console::ConsoleMessage::Level::Debug;
    }
    return Inspector::Protocol::Console::ConsoleMessage::Level::Log;
}

void ConsoleMessage::addToFrontend(ConsoleFrontendDispatcher& consoleFrontendDispatcher, InjectedScriptManager& injectedScriptManager, bool generatePreview)
{
    Ref<Inspector::Protocol::Console::ConsoleMessage> jsonObj = Inspector::Protocol::Console::ConsoleMessage::create()
        .setSource(messageSourceValue(m_source))
        .setLevel(messageLevelValue(m_level))
        .setText(m_message)
        .release();

    // FIXME: only send out type for ConsoleAPI source messages.
    jsonObj->setType(messageTypeValue(m_type));
    jsonObj->setLine(static_cast<int>(m_line));
    jsonObj->setColumn(static_cast<int>(m_column));
    jsonObj->setUrl(m_url);
    jsonObj->setRepeatCount(static_cast<int>(m_repeatCount));

    if (m_source == MessageSource::Network && !m_requestId.isEmpty())
        jsonObj->setNetworkRequestId(m_requestId);

    if (m_arguments && m_arguments->argumentCount()) {
        InjectedScript injectedScript = injectedScriptManager.injectedScriptFor(m_arguments->globalState());
        if (!injectedScript.hasNoValue()) {
            Ref<Inspector::Protocol::Array<Inspector::Protocol::Runtime::RemoteObject>> jsonArgs = Inspector::Protocol::Array<Inspector::Protocol::Runtime::RemoteObject>::create();
            if (m_type == MessageType::Table && generatePreview && m_arguments->argumentCount()) {
                Deprecated::ScriptValue table = m_arguments->argumentAt(0);
                Deprecated::ScriptValue columns = m_arguments->argumentCount() > 1 ? m_arguments->argumentAt(1) : Deprecated::ScriptValue();
                RefPtr<Inspector::Protocol::Runtime::RemoteObject> inspectorValue = injectedScript.wrapTable(table, columns);
                if (!inspectorValue) {
                    ASSERT_NOT_REACHED();
                    return;
                }
                jsonArgs->addItem(inspectorValue.copyRef());
                if (m_arguments->argumentCount() > 1)
                    jsonArgs->addItem(injectedScript.wrapObject(columns, ASCIILiteral("console"), true));
            } else {
                for (unsigned i = 0; i < m_arguments->argumentCount(); ++i) {
                    RefPtr<Inspector::Protocol::Runtime::RemoteObject> inspectorValue = injectedScript.wrapObject(m_arguments->argumentAt(i), ASCIILiteral("console"), generatePreview);
                    if (!inspectorValue) {
                        ASSERT_NOT_REACHED();
                        return;
                    }
                    jsonArgs->addItem(inspectorValue.copyRef());
                }
            }
            jsonObj->setParameters(WTFMove(jsonArgs));
        }
    }

    if (m_callStack)
        jsonObj->setStackTrace(m_callStack->buildInspectorArray());

    consoleFrontendDispatcher.messageAdded(WTFMove(jsonObj));
}

void ConsoleMessage::updateRepeatCountInConsole(ConsoleFrontendDispatcher& consoleFrontendDispatcher)
{
    consoleFrontendDispatcher.messageRepeatCountUpdated(m_repeatCount);
}

bool ConsoleMessage::isEqual(ConsoleMessage* msg) const
{
    if (m_arguments) {
        if (!m_arguments->isEqual(msg->m_arguments.get()))
            return false;

        // Never treat objects as equal - their properties might change over time.
        for (size_t i = 0; i < m_arguments->argumentCount(); ++i) {
            if (m_arguments->argumentAt(i).isObject())
                return false;
        }
    } else if (msg->m_arguments)
        return false;

    if (m_callStack) {
        if (!m_callStack->isEqual(msg->m_callStack.get()))
            return false;
    } else if (msg->m_callStack)
        return false;

    return msg->m_source == m_source
        && msg->m_type == m_type
        && msg->m_level == m_level
        && msg->m_message == m_message
        && msg->m_line == m_line
        && msg->m_column == m_column
        && msg->m_url == m_url
        && msg->m_requestId == m_requestId;
}

void ConsoleMessage::clear()
{
    if (!m_message)
        m_message = ASCIILiteral("<message collected>");

    if (m_arguments)
        m_arguments = nullptr;
}

JSC::ExecState* ConsoleMessage::scriptState() const
{
    if (m_arguments)
        return m_arguments->globalState();

    return nullptr;
}

unsigned ConsoleMessage::argumentCount() const
{
    if (m_arguments)
        return m_arguments->argumentCount();

    return 0;
}

} // namespace Inspector
