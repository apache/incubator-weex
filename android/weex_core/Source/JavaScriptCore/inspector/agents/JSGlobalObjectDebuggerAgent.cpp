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
#include "JSGlobalObjectDebuggerAgent.h"

#include "ConsoleMessage.h"
#include "InjectedScriptManager.h"
#include "InspectorConsoleAgent.h"
#include "JSGlobalObject.h"
#include "ScriptArguments.h"
#include "ScriptCallStack.h"
#include "ScriptCallStackFactory.h"

using namespace JSC;

namespace Inspector {

JSGlobalObjectDebuggerAgent::JSGlobalObjectDebuggerAgent(JSAgentContext& context, InspectorConsoleAgent* consoleAgent)
    : InspectorDebuggerAgent(context)
    , m_consoleAgent(consoleAgent)
{
}

InjectedScript JSGlobalObjectDebuggerAgent::injectedScriptForEval(ErrorString& error, const int* executionContextId)
{
    if (executionContextId) {
        error = ASCIILiteral("Execution context id is not supported for JSContext inspection as there is only one execution context.");
        return InjectedScript();
    }

    ExecState* exec = static_cast<JSGlobalObjectScriptDebugServer&>(scriptDebugServer()).globalObject().globalExec();
    return injectedScriptManager().injectedScriptFor(exec);
}

void JSGlobalObjectDebuggerAgent::breakpointActionLog(JSC::ExecState& state, const String& message)
{
    m_consoleAgent->addMessageToConsole(std::make_unique<ConsoleMessage>(MessageSource::JS, MessageType::Log, MessageLevel::Log, message, createScriptCallStack(&state, ScriptCallStack::maxCallStackSizeToCapture), 0));
}

} // namespace Inspector
