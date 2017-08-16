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

#include "config.h"
#include "InjectedScript.h"

#include "InspectorValues.h"
#include "JSCInlines.h"
#include "ScriptFunctionCall.h"
#include "ScriptObject.h"
#include <wtf/text/WTFString.h>

using Inspector::Protocol::Array;

namespace Inspector {

InjectedScript::InjectedScript()
    : InjectedScriptBase(ASCIILiteral("InjectedScript"))
{
}

InjectedScript::InjectedScript(Deprecated::ScriptObject injectedScriptObject, InspectorEnvironment* environment)
    : InjectedScriptBase(ASCIILiteral("InjectedScript"), injectedScriptObject, environment)
{
}

InjectedScript::~InjectedScript()
{
}

void InjectedScript::evaluate(ErrorString& errorString, const String& expression, const String& objectGroup, bool includeCommandLineAPI, bool returnByValue, bool generatePreview, bool saveResult, RefPtr<Inspector::Protocol::Runtime::RemoteObject>* result, Inspector::Protocol::OptOutput<bool>* wasThrown, Inspector::Protocol::OptOutput<int>* savedResultIndex)
{
    Deprecated::ScriptFunctionCall function(injectedScriptObject(), ASCIILiteral("evaluate"), inspectorEnvironment()->functionCallHandler());
    function.appendArgument(expression);
    function.appendArgument(objectGroup);
    function.appendArgument(includeCommandLineAPI);
    function.appendArgument(returnByValue);
    function.appendArgument(generatePreview);
    function.appendArgument(saveResult);
    makeEvalCall(errorString, function, result, wasThrown, savedResultIndex);
}

void InjectedScript::callFunctionOn(ErrorString& errorString, const String& objectId, const String& expression, const String& arguments, bool returnByValue, bool generatePreview, RefPtr<Inspector::Protocol::Runtime::RemoteObject>* result, Inspector::Protocol::OptOutput<bool>* wasThrown)
{
    Deprecated::ScriptFunctionCall function(injectedScriptObject(), ASCIILiteral("callFunctionOn"), inspectorEnvironment()->functionCallHandler());
    function.appendArgument(objectId);
    function.appendArgument(expression);
    function.appendArgument(arguments);
    function.appendArgument(returnByValue);
    function.appendArgument(generatePreview);
    makeEvalCall(errorString, function, result, wasThrown);
}

void InjectedScript::evaluateOnCallFrame(ErrorString& errorString, JSC::JSValue callFrames, const String& callFrameId, const String& expression, const String& objectGroup, bool includeCommandLineAPI, bool returnByValue, bool generatePreview, bool saveResult, RefPtr<Inspector::Protocol::Runtime::RemoteObject>* result, Inspector::Protocol::OptOutput<bool>* wasThrown, Inspector::Protocol::OptOutput<int>* savedResultIndex)
{
    Deprecated::ScriptFunctionCall function(injectedScriptObject(), ASCIILiteral("evaluateOnCallFrame"), inspectorEnvironment()->functionCallHandler());
    function.appendArgument(callFrames);
    function.appendArgument(callFrameId);
    function.appendArgument(expression);
    function.appendArgument(objectGroup);
    function.appendArgument(includeCommandLineAPI);
    function.appendArgument(returnByValue);
    function.appendArgument(generatePreview);
    function.appendArgument(saveResult);
    makeEvalCall(errorString, function, result, wasThrown, savedResultIndex);
}

void InjectedScript::getFunctionDetails(ErrorString& errorString, const String& functionId, RefPtr<Inspector::Protocol::Debugger::FunctionDetails>* result)
{
    Deprecated::ScriptFunctionCall function(injectedScriptObject(), ASCIILiteral("getFunctionDetails"), inspectorEnvironment()->functionCallHandler());
    function.appendArgument(functionId);

    RefPtr<InspectorValue> resultValue;
    makeCall(function, &resultValue);
    if (!resultValue || resultValue->type() != InspectorValue::Type::Object) {
        if (!resultValue->asString(errorString))
            errorString = ASCIILiteral("Internal error");
        return;
    }

    *result = BindingTraits<Inspector::Protocol::Debugger::FunctionDetails>::runtimeCast(WTFMove(resultValue));
}

void InjectedScript::functionDetails(ErrorString& errorString, JSC::JSValue value, RefPtr<Protocol::Debugger::FunctionDetails>* result)
{
    Deprecated::ScriptFunctionCall function(injectedScriptObject(), ASCIILiteral("functionDetails"), inspectorEnvironment()->functionCallHandler());
    function.appendArgument(value);
    function.appendArgument(true); // Preview only.

    RefPtr<InspectorValue> resultValue;
    makeCall(function, &resultValue);
    if (!resultValue || resultValue->type() != InspectorValue::Type::Object) {
        if (!resultValue->asString(errorString))
            errorString = ASCIILiteral("Internal error");
        return;
    }

    *result = BindingTraits<Inspector::Protocol::Debugger::FunctionDetails>::runtimeCast(WTFMove(resultValue));
}

void InjectedScript::getProperties(ErrorString& errorString, const String& objectId, bool ownProperties, bool generatePreview, RefPtr<Array<Inspector::Protocol::Runtime::PropertyDescriptor>>* properties)
{
    Deprecated::ScriptFunctionCall function(injectedScriptObject(), ASCIILiteral("getProperties"), inspectorEnvironment()->functionCallHandler());
    function.appendArgument(objectId);
    function.appendArgument(ownProperties);
    function.appendArgument(generatePreview);

    RefPtr<InspectorValue> result;
    makeCall(function, &result);
    if (!result || result->type() != InspectorValue::Type::Array) {
        errorString = ASCIILiteral("Internal error");
        return;
    }

    *properties = BindingTraits<Array<Inspector::Protocol::Runtime::PropertyDescriptor>>::runtimeCast(WTFMove(result));
}

void InjectedScript::getDisplayableProperties(ErrorString& errorString, const String& objectId, bool generatePreview, RefPtr<Array<Inspector::Protocol::Runtime::PropertyDescriptor>>* properties)
{
    Deprecated::ScriptFunctionCall function(injectedScriptObject(), ASCIILiteral("getDisplayableProperties"), inspectorEnvironment()->functionCallHandler());
    function.appendArgument(objectId);
    function.appendArgument(generatePreview);

    RefPtr<InspectorValue> result;
    makeCall(function, &result);
    if (!result || result->type() != InspectorValue::Type::Array) {
        errorString = ASCIILiteral("Internal error");
        return;
    }

    *properties = BindingTraits<Array<Inspector::Protocol::Runtime::PropertyDescriptor>>::runtimeCast(WTFMove(result));
}

void InjectedScript::getInternalProperties(ErrorString& errorString, const String& objectId, bool generatePreview, RefPtr<Array<Inspector::Protocol::Runtime::InternalPropertyDescriptor>>* properties)
{
    Deprecated::ScriptFunctionCall function(injectedScriptObject(), ASCIILiteral("getInternalProperties"), inspectorEnvironment()->functionCallHandler());
    function.appendArgument(objectId);
    function.appendArgument(generatePreview);

    RefPtr<InspectorValue> result;
    makeCall(function, &result);
    if (!result || result->type() != InspectorValue::Type::Array) {
        errorString = ASCIILiteral("Internal error");
        return;
    }

    auto array = BindingTraits<Array<Inspector::Protocol::Runtime::InternalPropertyDescriptor>>::runtimeCast(WTFMove(result));
    *properties = array->length() > 0 ? array : nullptr;
}

void InjectedScript::getCollectionEntries(ErrorString& errorString, const String& objectId, const String& objectGroup, int startIndex, int numberToFetch, RefPtr<Protocol::Array<Protocol::Runtime::CollectionEntry>>* entries)
{
    Deprecated::ScriptFunctionCall function(injectedScriptObject(), ASCIILiteral("getCollectionEntries"), inspectorEnvironment()->functionCallHandler());
    function.appendArgument(objectId);
    function.appendArgument(objectGroup);
    function.appendArgument(startIndex);
    function.appendArgument(numberToFetch);

    RefPtr<InspectorValue> result;
    makeCall(function, &result);
    if (!result || result->type() != InspectorValue::Type::Array) {
        errorString = ASCIILiteral("Internal error");
        return;
    }

    *entries = BindingTraits<Array<Protocol::Runtime::CollectionEntry>>::runtimeCast(WTFMove(result));
}

void InjectedScript::saveResult(ErrorString& errorString, const String& callArgumentJSON, Inspector::Protocol::OptOutput<int>* savedResultIndex)
{
    Deprecated::ScriptFunctionCall function(injectedScriptObject(), ASCIILiteral("saveResult"), inspectorEnvironment()->functionCallHandler());
    function.appendArgument(callArgumentJSON);

    RefPtr<InspectorValue> result;
    makeCall(function, &result);
    if (!result || result->type() != InspectorValue::Type::Integer) {
        errorString = ASCIILiteral("Internal error");
        return;
    }

    int savedResultIndexInt = 0;
    if (result->asInteger(savedResultIndexInt) && savedResultIndexInt > 0)
        *savedResultIndex = savedResultIndexInt;
}

Ref<Array<Inspector::Protocol::Debugger::CallFrame>> InjectedScript::wrapCallFrames(JSC::JSValue callFrames) const
{
    ASSERT(!hasNoValue());
    Deprecated::ScriptFunctionCall function(injectedScriptObject(), ASCIILiteral("wrapCallFrames"), inspectorEnvironment()->functionCallHandler());
    function.appendArgument(callFrames);

    bool hadException = false;
    auto callFramesValue = callFunctionWithEvalEnabled(function, hadException);
    if (!callFramesValue)
        return Array<Inspector::Protocol::Debugger::CallFrame>::create();
    ASSERT(!hadException);
    RefPtr<InspectorValue> result = toInspectorValue(*scriptState(), callFramesValue);
    if (result->type() == InspectorValue::Type::Array)
        return BindingTraits<Array<Inspector::Protocol::Debugger::CallFrame>>::runtimeCast(WTFMove(result)).releaseNonNull();

    return Array<Inspector::Protocol::Debugger::CallFrame>::create();
}

RefPtr<Inspector::Protocol::Runtime::RemoteObject> InjectedScript::wrapObject(JSC::JSValue value, const String& groupName, bool generatePreview) const
{
    ASSERT(!hasNoValue());
    Deprecated::ScriptFunctionCall wrapFunction(injectedScriptObject(), ASCIILiteral("wrapObject"), inspectorEnvironment()->functionCallHandler());
    wrapFunction.appendArgument(value);
    wrapFunction.appendArgument(groupName);
    wrapFunction.appendArgument(hasAccessToInspectedScriptState());
    wrapFunction.appendArgument(generatePreview);

    bool hadException = false;
    auto r = callFunctionWithEvalEnabled(wrapFunction, hadException);
    if (hadException)
        return nullptr;

    RefPtr<InspectorObject> resultObject;
    bool castSucceeded = toInspectorValue(*scriptState(), r)->asObject(resultObject);
    ASSERT_UNUSED(castSucceeded, castSucceeded);

    return BindingTraits<Inspector::Protocol::Runtime::RemoteObject>::runtimeCast(resultObject);
}

RefPtr<Inspector::Protocol::Runtime::RemoteObject> InjectedScript::wrapTable(JSC::JSValue table, JSC::JSValue columns) const
{
    ASSERT(!hasNoValue());
    Deprecated::ScriptFunctionCall wrapFunction(injectedScriptObject(), ASCIILiteral("wrapTable"), inspectorEnvironment()->functionCallHandler());
    wrapFunction.appendArgument(hasAccessToInspectedScriptState());
    wrapFunction.appendArgument(table);
    if (!columns)
        wrapFunction.appendArgument(false);
    else
        wrapFunction.appendArgument(columns);

    bool hadException = false;
    auto r = callFunctionWithEvalEnabled(wrapFunction, hadException);
    if (hadException)
        return nullptr;

    RefPtr<InspectorObject> resultObject;
    bool castSucceeded = toInspectorValue(*scriptState(), r)->asObject(resultObject);
    ASSERT_UNUSED(castSucceeded, castSucceeded);

    return BindingTraits<Inspector::Protocol::Runtime::RemoteObject>::runtimeCast(resultObject);
}

RefPtr<Inspector::Protocol::Runtime::ObjectPreview> InjectedScript::previewValue(JSC::JSValue value) const
{
    ASSERT(!hasNoValue());
    Deprecated::ScriptFunctionCall wrapFunction(injectedScriptObject(), ASCIILiteral("previewValue"), inspectorEnvironment()->functionCallHandler());
    wrapFunction.appendArgument(value);

    bool hadException = false;
    auto r = callFunctionWithEvalEnabled(wrapFunction, hadException);
    if (hadException)
        return nullptr;

    RefPtr<InspectorObject> resultObject;
    bool castSucceeded = toInspectorValue(*scriptState(), r)->asObject(resultObject);
    ASSERT_UNUSED(castSucceeded, castSucceeded);

    return BindingTraits<Inspector::Protocol::Runtime::ObjectPreview>::runtimeCast(resultObject);
}

void InjectedScript::setExceptionValue(JSC::JSValue value)
{
    ASSERT(!hasNoValue());
    Deprecated::ScriptFunctionCall function(injectedScriptObject(), ASCIILiteral("setExceptionValue"), inspectorEnvironment()->functionCallHandler());
    function.appendArgument(value);
    RefPtr<InspectorValue> result;
    makeCall(function, &result);
}

void InjectedScript::clearExceptionValue()
{
    ASSERT(!hasNoValue());
    Deprecated::ScriptFunctionCall function(injectedScriptObject(), ASCIILiteral("clearExceptionValue"), inspectorEnvironment()->functionCallHandler());
    RefPtr<InspectorValue> result;
    makeCall(function, &result);
}

JSC::JSValue InjectedScript::findObjectById(const String& objectId) const
{
    ASSERT(!hasNoValue());
    Deprecated::ScriptFunctionCall function(injectedScriptObject(), ASCIILiteral("findObjectById"), inspectorEnvironment()->functionCallHandler());
    function.appendArgument(objectId);

    bool hadException = false;
    auto resultValue = callFunctionWithEvalEnabled(function, hadException);
    ASSERT(!hadException);

    return resultValue;
}

void InjectedScript::inspectObject(JSC::JSValue value)
{
    ASSERT(!hasNoValue());
    Deprecated::ScriptFunctionCall function(injectedScriptObject(), ASCIILiteral("inspectObject"), inspectorEnvironment()->functionCallHandler());
    function.appendArgument(value);
    RefPtr<InspectorValue> result;
    makeCall(function, &result);
}

void InjectedScript::releaseObject(const String& objectId)
{
    Deprecated::ScriptFunctionCall function(injectedScriptObject(), ASCIILiteral("releaseObject"), inspectorEnvironment()->functionCallHandler());
    function.appendArgument(objectId);
    RefPtr<InspectorValue> result;
    makeCall(function, &result);
}

void InjectedScript::releaseObjectGroup(const String& objectGroup)
{
    ASSERT(!hasNoValue());
    Deprecated::ScriptFunctionCall releaseFunction(injectedScriptObject(), ASCIILiteral("releaseObjectGroup"), inspectorEnvironment()->functionCallHandler());
    releaseFunction.appendArgument(objectGroup);

    bool hadException = false;
    callFunctionWithEvalEnabled(releaseFunction, hadException);
    ASSERT(!hadException);
}

} // namespace Inspector

