/*
 * Copyright (C) 2007, 2008, 2015 Apple Inc. All rights reserved.
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

#pragma once

#include "ConsoleTypes.h"
#include "InspectorFrontendDispatchers.h"
#include <wtf/Forward.h>

namespace JSC {
class ExecState;
}

namespace Inspector {

class InjectedScriptManager;
class ScriptArguments;
class ScriptCallStack;

class JS_EXPORT_PRIVATE ConsoleMessage {
    WTF_MAKE_NONCOPYABLE(ConsoleMessage);
    WTF_MAKE_FAST_ALLOCATED;
public:
    ConsoleMessage(MessageSource, MessageType, MessageLevel, const String& message, unsigned long requestIdentifier = 0);
    ConsoleMessage(MessageSource, MessageType, MessageLevel, const String& message, const String& url, unsigned line, unsigned column, JSC::ExecState* = nullptr, unsigned long requestIdentifier = 0);
    ConsoleMessage(MessageSource, MessageType, MessageLevel, const String& message, Ref<ScriptCallStack>&&, unsigned long requestIdentifier = 0);
    ConsoleMessage(MessageSource, MessageType, MessageLevel, const String& message, Ref<ScriptArguments>&&, JSC::ExecState*, unsigned long requestIdentifier = 0);
    ~ConsoleMessage();

    void addToFrontend(ConsoleFrontendDispatcher&, InjectedScriptManager&, bool generatePreview);
    void updateRepeatCountInConsole(ConsoleFrontendDispatcher&);

    MessageSource source() const { return m_source; }
    MessageType type() const { return m_type; }
    MessageLevel level() const { return m_level; }
    const String& message() const { return m_message; }
    const String& url() const { return m_url; }
    unsigned line() const { return m_line; }
    unsigned column() const { return m_column; }

    JSC::ExecState* scriptState() const;

    void incrementCount() { ++m_repeatCount; }

    unsigned argumentCount() const;

    bool isEqual(ConsoleMessage* msg) const;

    void clear();

private:
    void autogenerateMetadata(JSC::ExecState* = nullptr);

    MessageSource m_source;
    MessageType m_type;
    MessageLevel m_level;
    String m_message;
    RefPtr<ScriptArguments> m_arguments;
    RefPtr<ScriptCallStack> m_callStack;
    String m_url;
    unsigned m_line { 0 };
    unsigned m_column { 0 };
    unsigned m_repeatCount { 1 };
    String m_requestId;
};

} // namespace Inspector
