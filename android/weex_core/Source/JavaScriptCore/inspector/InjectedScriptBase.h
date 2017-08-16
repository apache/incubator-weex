/*
 * Copyright (C) 2013 Apple Inc. All rights reserved.
 * Copyright (C) 2012 Google Inc. All rights reserved.
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

#pragma once

#include "InspectorEnvironment.h"
#include "InspectorProtocolObjects.h"
#include "bindings/ScriptObject.h"
#include <wtf/Forward.h>
#include <wtf/RefPtr.h>

namespace Deprecated {
class ScriptFunctionCall;
}

namespace Inspector {

typedef String ErrorString;

class JS_EXPORT_PRIVATE InjectedScriptBase {
public:
    virtual ~InjectedScriptBase();

    const String& name() const { return m_name; }
    bool hasNoValue() const { return m_injectedScriptObject.hasNoValue(); }
    JSC::ExecState* scriptState() const { return m_injectedScriptObject.scriptState(); }

protected:
    InjectedScriptBase(const String& name);
    InjectedScriptBase(const String& name, Deprecated::ScriptObject, InspectorEnvironment*);

    InspectorEnvironment* inspectorEnvironment() const { return m_environment; }

    bool hasAccessToInspectedScriptState() const;

    const Deprecated::ScriptObject& injectedScriptObject() const;
    JSC::JSValue callFunctionWithEvalEnabled(Deprecated::ScriptFunctionCall&, bool& hadException) const;
    void makeCall(Deprecated::ScriptFunctionCall&, RefPtr<InspectorValue>* result);
    void makeEvalCall(ErrorString&, Deprecated::ScriptFunctionCall&, RefPtr<Protocol::Runtime::RemoteObject>* result, Protocol::OptOutput<bool>* wasThrown, Protocol::OptOutput<int>* savedResult = nullptr);

private:
    String m_name;
    Deprecated::ScriptObject m_injectedScriptObject;
    InspectorEnvironment* m_environment;
};

} // namespace Inspector
