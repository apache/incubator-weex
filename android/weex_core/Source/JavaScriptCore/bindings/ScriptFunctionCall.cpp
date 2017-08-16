/*
 * Copyright (C) 2013 Apple Inc. All rights reserved.
 * Copyright (C) 2009 Google Inc. All rights reserved.
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
#include "ScriptFunctionCall.h"

#include "JSCInlines.h"
#include "JSLock.h"
#include "ScriptValue.h"
#include <wtf/text/WTFString.h>

using namespace JSC;

namespace Deprecated {

void ScriptCallArgumentHandler::appendArgument(const String& argument)
{
    JSLockHolder lock(m_exec);
    m_arguments.append(jsString(m_exec, argument));
}

void ScriptCallArgumentHandler::appendArgument(const char* argument)
{
    JSLockHolder lock(m_exec);
    m_arguments.append(jsString(m_exec, String(argument)));
}

void ScriptCallArgumentHandler::appendArgument(JSValue argument)
{
    m_arguments.append(argument);
}

void ScriptCallArgumentHandler::appendArgument(long argument)
{
    JSLockHolder lock(m_exec);
    m_arguments.append(jsNumber(argument));
}

void ScriptCallArgumentHandler::appendArgument(long long argument)
{
    JSLockHolder lock(m_exec);
    m_arguments.append(jsNumber(argument));
}

void ScriptCallArgumentHandler::appendArgument(unsigned int argument)
{
    JSLockHolder lock(m_exec);
    m_arguments.append(jsNumber(argument));
}

void ScriptCallArgumentHandler::appendArgument(unsigned long argument)
{
    JSLockHolder lock(m_exec);
    m_arguments.append(jsNumber(argument));
}

void ScriptCallArgumentHandler::appendArgument(int argument)
{
    JSLockHolder lock(m_exec);
    m_arguments.append(jsNumber(argument));
}

void ScriptCallArgumentHandler::appendArgument(bool argument)
{
    m_arguments.append(jsBoolean(argument));
}

ScriptFunctionCall::ScriptFunctionCall(const Deprecated::ScriptObject& thisObject, const String& name, ScriptFunctionCallHandler callHandler)
    : ScriptCallArgumentHandler(thisObject.scriptState())
    , m_callHandler(callHandler)
    , m_thisObject(thisObject)
    , m_name(name)
{
}

JSValue ScriptFunctionCall::call(bool& hadException)
{
    JSObject* thisObject = m_thisObject.jsObject();

    VM& vm = m_exec->vm();
    JSLockHolder lock(vm);
    auto scope = DECLARE_THROW_SCOPE(vm);

    JSValue function = thisObject->get(m_exec, Identifier::fromString(m_exec, m_name));
    if (UNLIKELY(scope.exception())) {
        hadException = true;
        return { };
    }

    CallData callData;
    CallType callType = getCallData(function, callData);
    if (callType == CallType::None)
        return { };

    JSValue result;
    NakedPtr<Exception> exception;
    if (m_callHandler)
        result = m_callHandler(m_exec, function, callType, callData, thisObject, m_arguments, exception);
    else
        result = JSC::call(m_exec, function, callType, callData, thisObject, m_arguments, exception);

    if (exception) {
        // Do not treat a terminated execution exception as having an exception. Just treat it as an empty result.
        hadException = !isTerminatedExecutionException(vm, exception);
        return { };
    }

    return result;
}

JSC::JSValue ScriptFunctionCall::call()
{
    bool hadException;
    return call(hadException);
}

} // namespace Deprecated
