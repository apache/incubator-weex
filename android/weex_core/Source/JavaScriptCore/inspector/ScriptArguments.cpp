/*
 * Copyright (C) 2014 Apple Inc. All rights reserved.
 * Copyright (c) 2010 Google Inc. All rights reserved.
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
#include "ScriptArguments.h"

#include "JSCInlines.h"
#include "ProxyObject.h"
#include "ScriptValue.h"

namespace Inspector {

Ref<ScriptArguments> ScriptArguments::create(JSC::ExecState* scriptState, Vector<Deprecated::ScriptValue>& arguments)
{
    return adoptRef(*new ScriptArguments(scriptState, arguments));
}

Ref<ScriptArguments> ScriptArguments::createEmpty(JSC::ExecState* scriptState)
{
    return adoptRef(*new ScriptArguments(scriptState));
}

ScriptArguments::ScriptArguments(JSC::ExecState* execState)
    : m_globalObject(execState->vm(), execState->lexicalGlobalObject())
{
}

ScriptArguments::ScriptArguments(JSC::ExecState* execState, Vector<Deprecated::ScriptValue>& arguments)
    : m_globalObject(execState->vm(), execState->lexicalGlobalObject())
{
    m_arguments.swap(arguments);
}

ScriptArguments::~ScriptArguments()
{
}

const Deprecated::ScriptValue& ScriptArguments::argumentAt(size_t index) const
{
    ASSERT(m_arguments.size() > index);
    return m_arguments[index];
}

JSC::ExecState* ScriptArguments::globalState() const
{
    if (m_globalObject)
        return const_cast<JSC::JSGlobalObject*>(m_globalObject.get())->globalExec();

    return nullptr;
}

bool ScriptArguments::getFirstArgumentAsString(String& result)
{
    if (!argumentCount())
        return false;

    if (!globalState()) {
        ASSERT_NOT_REACHED();
        return false;
    }

    JSC::JSValue value = argumentAt(0).jsValue();
    if (JSC::jsDynamicCast<JSC::ProxyObject*>(globalState()->vm(), value)) {
        result = ASCIILiteral("[object Proxy]");
        return true;
    }

    result = argumentAt(0).toString(globalState());
    return true;
}

bool ScriptArguments::isEqual(ScriptArguments* other) const
{
    if (!other)
        return false;

    if (m_arguments.size() != other->m_arguments.size())
        return false;
    if (!globalState() && m_arguments.size())
        return false;

    for (size_t i = 0; i < m_arguments.size(); ++i) {
        if (!m_arguments[i].isEqual(other->globalState(), other->m_arguments[i]))
            return false;
    }

    return true;
}

} // namespace Inspector
