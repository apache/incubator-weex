/*
 * Copyright (C) 2006, 2007, 2008, 2013 Apple Inc. All rights reserved.
 * Copyright (c) 2011 Google Inc. All rights reserved.
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
#include "ScriptValue.h"

#include "APICast.h"
#include "InspectorValues.h"
#include "JSCInlines.h"
#include "JSLock.h"

using namespace JSC;
using namespace Inspector;

namespace Inspector {

static RefPtr<InspectorValue> jsToInspectorValue(ExecState& scriptState, JSValue value, int maxDepth)
{
    if (!value) {
        ASSERT_NOT_REACHED();
        return nullptr;
    }

    if (!maxDepth)
        return nullptr;

    maxDepth--;

    if (value.isUndefinedOrNull())
        return InspectorValue::null();
    if (value.isBoolean())
        return InspectorValue::create(value.asBoolean());
    if (value.isNumber() && value.isDouble())
        return InspectorValue::create(value.asNumber());
    if (value.isNumber() && value.isAnyInt())
        return InspectorValue::create(static_cast<int>(value.asAnyInt()));
    if (value.isString())
        return InspectorValue::create(asString(value)->value(&scriptState));

    if (value.isObject()) {
        if (isJSArray(value)) {
            auto inspectorArray = InspectorArray::create();
            auto& array = *asArray(value);
            unsigned length = array.length();
            for (unsigned i = 0; i < length; i++) {
                auto elementValue = jsToInspectorValue(scriptState, array.getIndex(&scriptState, i), maxDepth);
                if (!elementValue)
                    return nullptr;
                inspectorArray->pushValue(WTFMove(elementValue));
            }
            return WTFMove(inspectorArray);
        }
        auto inspectorObject = InspectorObject::create();
        auto& object = *value.getObject();
        PropertyNameArray propertyNames(&scriptState, PropertyNameMode::Strings);
        object.methodTable()->getOwnPropertyNames(&object, &scriptState, propertyNames, EnumerationMode());
        for (auto& name : propertyNames) {
            auto inspectorValue = jsToInspectorValue(scriptState, object.get(&scriptState, name), maxDepth);
            if (!inspectorValue)
                return nullptr;
            inspectorObject->setValue(name.string(), WTFMove(inspectorValue));
        }
        return WTFMove(inspectorObject);
    }

    ASSERT_NOT_REACHED();
    return nullptr;
}

RefPtr<InspectorValue> toInspectorValue(ExecState& state, JSValue value)
{
    // FIXME: Maybe we should move the JSLockHolder stuff to the callers since this function takes a JSValue directly.
    // Doing the locking here made sense when we were trying to abstract the difference between multiple JavaScript engines.
    JSLockHolder holder(&state);
    return jsToInspectorValue(state, value, InspectorValue::maxDepth);
}

} // namespace Inspector

namespace Deprecated {

ScriptValue::~ScriptValue()
{
}

bool ScriptValue::getString(ExecState* scriptState, String& result) const
{
    if (!m_value)
        return false;
    JSLockHolder lock(scriptState);
    if (!m_value.get().getString(scriptState, result))
        return false;
    return true;
}

String ScriptValue::toString(ExecState* scriptState) const
{
    VM& vm = scriptState->vm();
    auto scope = DECLARE_CATCH_SCOPE(vm);

    String result = m_value.get().toWTFString(scriptState);
    // Handle the case where an exception is thrown as part of invoking toString on the object.
    if (UNLIKELY(scope.exception()))
        scope.clearException();
    return result;
}

bool ScriptValue::isEqual(ExecState* scriptState, const ScriptValue& anotherValue) const
{
    if (hasNoValue())
        return anotherValue.hasNoValue();
    return JSValueIsStrictEqual(toRef(scriptState), toRef(scriptState, jsValue()), toRef(scriptState, anotherValue.jsValue()));
}

bool ScriptValue::isNull() const
{
    if (!m_value)
        return false;
    return m_value.get().isNull();
}

bool ScriptValue::isUndefined() const
{
    if (!m_value)
        return false;
    return m_value.get().isUndefined();
}

bool ScriptValue::isObject() const
{
    if (!m_value)
        return false;
    return m_value.get().isObject();
}

bool ScriptValue::isFunction() const
{
    CallData callData;
    return getCallData(m_value.get(), callData) != CallType::None;
}

RefPtr<InspectorValue> ScriptValue::toInspectorValue(ExecState* scriptState) const
{
    JSLockHolder holder(scriptState);
    return jsToInspectorValue(*scriptState, m_value.get(), InspectorValue::maxDepth);
}

} // namespace Deprecated
