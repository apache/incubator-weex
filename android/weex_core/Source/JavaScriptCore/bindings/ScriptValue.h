/*
 * Copyright (C) 2013 Apple Inc. All rights reserved.
 * Copyright (c) 2008, 2011 Google Inc. All rights reserved.
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

#include "JSCJSValue.h"
#include "JSCJSValueInlines.h"
#include "Operations.h"
#include "Strong.h"
#include "StrongInlines.h"
#include <wtf/text/WTFString.h>

namespace Inspector {

class InspectorValue;

JS_EXPORT_PRIVATE RefPtr<InspectorValue> toInspectorValue(JSC::ExecState&, JSC::JSValue);

}

namespace Deprecated {

class JS_EXPORT_PRIVATE ScriptValue {
public:
    ScriptValue() { }
    ScriptValue(JSC::VM& vm, JSC::JSValue value) : m_value(vm, value) { }
    virtual ~ScriptValue();

    operator JSC::JSValue() const { return jsValue(); }
    JSC::JSValue jsValue() const { return m_value.get(); }
    bool getString(JSC::ExecState*, String& result) const;
    String toString(JSC::ExecState*) const;
    bool isEqual(JSC::ExecState*, const ScriptValue&) const;
    bool isNull() const;
    bool isUndefined() const;
    bool isObject() const;
    bool isFunction() const;
    bool hasNoValue() const { return !m_value; }

    void clear() { m_value.clear(); }

    bool operator==(const ScriptValue& other) const { return m_value == other.m_value; }

    RefPtr<Inspector::InspectorValue> toInspectorValue(JSC::ExecState*) const;

private:
    JSC::Strong<JSC::Unknown> m_value;
};

} // namespace Deprecated
