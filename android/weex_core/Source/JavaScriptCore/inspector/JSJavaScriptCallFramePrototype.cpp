/*
 * Copyright (C) 2014, 2016 Apple Inc. All rights reserved.
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
#include "JSJavaScriptCallFramePrototype.h"

#include "Error.h"
#include "Identifier.h"
#include "JSCInlines.h"
#include "JSFunction.h"
#include "JSJavaScriptCallFrame.h"

using namespace JSC;

namespace Inspector {

// Functions.
static EncodedJSValue JSC_HOST_CALL jsJavaScriptCallFramePrototypeFunctionEvaluateWithScopeExtension(ExecState*);
static EncodedJSValue JSC_HOST_CALL jsJavaScriptCallFramePrototypeFunctionScopeDescriptions(ExecState*);

// Attributes.
static EncodedJSValue JSC_HOST_CALL jsJavaScriptCallFrameAttributeCaller(ExecState*);
static EncodedJSValue JSC_HOST_CALL jsJavaScriptCallFrameAttributeSourceID(ExecState*);
static EncodedJSValue JSC_HOST_CALL jsJavaScriptCallFrameAttributeLine(ExecState*);
static EncodedJSValue JSC_HOST_CALL jsJavaScriptCallFrameAttributeColumn(ExecState*);
static EncodedJSValue JSC_HOST_CALL jsJavaScriptCallFrameAttributeFunctionName(ExecState*);
static EncodedJSValue JSC_HOST_CALL jsJavaScriptCallFrameAttributeScopeChain(ExecState*);
static EncodedJSValue JSC_HOST_CALL jsJavaScriptCallFrameAttributeThisObject(ExecState*);
static EncodedJSValue JSC_HOST_CALL jsJavaScriptCallFrameAttributeType(ExecState*);
static EncodedJSValue JSC_HOST_CALL jsJavaScriptCallFrameIsTailDeleted(ExecState*);

const ClassInfo JSJavaScriptCallFramePrototype::s_info = { "JavaScriptCallFrame", &Base::s_info, 0, CREATE_METHOD_TABLE(JSJavaScriptCallFramePrototype) };

void JSJavaScriptCallFramePrototype::finishCreation(VM& vm, JSGlobalObject* globalObject)
{
    Base::finishCreation(vm);
    ASSERT(inherits(vm, info()));
    vm.prototypeMap.addPrototype(this);

    JSC_NATIVE_FUNCTION_WITHOUT_TRANSITION("evaluateWithScopeExtension", jsJavaScriptCallFramePrototypeFunctionEvaluateWithScopeExtension, DontEnum, 1);
    JSC_NATIVE_FUNCTION_WITHOUT_TRANSITION("scopeDescriptions", jsJavaScriptCallFramePrototypeFunctionScopeDescriptions, DontEnum, 0);

    JSC_NATIVE_GETTER("caller", jsJavaScriptCallFrameAttributeCaller, DontEnum | Accessor);
    JSC_NATIVE_GETTER("sourceID", jsJavaScriptCallFrameAttributeSourceID, DontEnum | Accessor);
    JSC_NATIVE_GETTER("line", jsJavaScriptCallFrameAttributeLine, DontEnum | Accessor);
    JSC_NATIVE_GETTER("column", jsJavaScriptCallFrameAttributeColumn, DontEnum | Accessor);
    JSC_NATIVE_GETTER("functionName", jsJavaScriptCallFrameAttributeFunctionName, DontEnum | Accessor);
    JSC_NATIVE_GETTER("scopeChain", jsJavaScriptCallFrameAttributeScopeChain, DontEnum | Accessor);
    JSC_NATIVE_GETTER("thisObject", jsJavaScriptCallFrameAttributeThisObject, DontEnum | Accessor);
    JSC_NATIVE_GETTER("type", jsJavaScriptCallFrameAttributeType, DontEnum | Accessor);
    JSC_NATIVE_GETTER("isTailDeleted", jsJavaScriptCallFrameIsTailDeleted, DontEnum | Accessor);
}

EncodedJSValue JSC_HOST_CALL jsJavaScriptCallFramePrototypeFunctionEvaluateWithScopeExtension(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    JSValue thisValue = exec->thisValue();
    JSJavaScriptCallFrame* castedThis = jsDynamicCast<JSJavaScriptCallFrame*>(vm, thisValue);
    if (!castedThis)
        return throwVMTypeError(exec, scope);

    return JSValue::encode(castedThis->evaluateWithScopeExtension(exec));
}

EncodedJSValue JSC_HOST_CALL jsJavaScriptCallFramePrototypeFunctionScopeDescriptions(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    JSValue thisValue = exec->thisValue();
    JSJavaScriptCallFrame* castedThis = jsDynamicCast<JSJavaScriptCallFrame*>(vm, thisValue);
    if (!castedThis)
        return throwVMTypeError(exec, scope);

    return JSValue::encode(castedThis->scopeDescriptions(exec));
}

EncodedJSValue JSC_HOST_CALL jsJavaScriptCallFrameAttributeCaller(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    JSValue thisValue = exec->thisValue();
    JSJavaScriptCallFrame* castedThis = jsDynamicCast<JSJavaScriptCallFrame*>(vm, thisValue);
    if (!castedThis)
        return throwVMTypeError(exec, scope);

    return JSValue::encode(castedThis->caller(exec));
}

EncodedJSValue JSC_HOST_CALL jsJavaScriptCallFrameAttributeSourceID(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    JSValue thisValue = exec->thisValue();
    JSJavaScriptCallFrame* castedThis = jsDynamicCast<JSJavaScriptCallFrame*>(vm, thisValue);
    if (!castedThis)
        return throwVMTypeError(exec, scope);

    return JSValue::encode(castedThis->sourceID(exec));
}

EncodedJSValue JSC_HOST_CALL jsJavaScriptCallFrameAttributeLine(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    JSValue thisValue = exec->thisValue();
    JSJavaScriptCallFrame* castedThis = jsDynamicCast<JSJavaScriptCallFrame*>(vm, thisValue);
    if (!castedThis)
        return throwVMTypeError(exec, scope);

    return JSValue::encode(castedThis->line(exec));
}

EncodedJSValue JSC_HOST_CALL jsJavaScriptCallFrameAttributeColumn(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    JSValue thisValue = exec->thisValue();
    JSJavaScriptCallFrame* castedThis = jsDynamicCast<JSJavaScriptCallFrame*>(vm, thisValue);
    if (!castedThis)
        return throwVMTypeError(exec, scope);

    return JSValue::encode(castedThis->column(exec));
}

EncodedJSValue JSC_HOST_CALL jsJavaScriptCallFrameAttributeFunctionName(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    JSValue thisValue = exec->thisValue();
    JSJavaScriptCallFrame* castedThis = jsDynamicCast<JSJavaScriptCallFrame*>(vm, thisValue);
    if (!castedThis)
        return throwVMTypeError(exec, scope);

    return JSValue::encode(castedThis->functionName(exec));
}

EncodedJSValue JSC_HOST_CALL jsJavaScriptCallFrameAttributeScopeChain(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    JSValue thisValue = exec->thisValue();
    JSJavaScriptCallFrame* castedThis = jsDynamicCast<JSJavaScriptCallFrame*>(vm, thisValue);
    if (!castedThis)
        return throwVMTypeError(exec, scope);

    return JSValue::encode(castedThis->scopeChain(exec));
}

EncodedJSValue JSC_HOST_CALL jsJavaScriptCallFrameAttributeThisObject(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    JSValue thisValue = exec->thisValue();
    JSJavaScriptCallFrame* castedThis = jsDynamicCast<JSJavaScriptCallFrame*>(vm, thisValue);
    if (!castedThis)
        return throwVMTypeError(exec, scope);

    return JSValue::encode(castedThis->thisObject(exec));
}

EncodedJSValue JSC_HOST_CALL jsJavaScriptCallFrameAttributeType(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    JSValue thisValue = exec->thisValue();
    JSJavaScriptCallFrame* castedThis = jsDynamicCast<JSJavaScriptCallFrame*>(vm, thisValue);
    if (!castedThis)
        return throwVMTypeError(exec, scope);

    return JSValue::encode(castedThis->type(exec));
}

EncodedJSValue JSC_HOST_CALL jsJavaScriptCallFrameIsTailDeleted(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    JSValue thisValue = exec->thisValue();
    JSJavaScriptCallFrame* castedThis = jsDynamicCast<JSJavaScriptCallFrame*>(vm, thisValue);
    if (!castedThis)
        return throwVMTypeError(exec, scope);

    return JSValue::encode(castedThis->isTailDeleted(exec));
}

} // namespace Inspector
