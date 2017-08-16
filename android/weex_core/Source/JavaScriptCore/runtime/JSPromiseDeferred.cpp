/*
 * Copyright (C) 2013, 2016 Apple Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "JSPromiseDeferred.h"

#include "BuiltinNames.h"
#include "Error.h"
#include "Exception.h"
#include "JSCInlines.h"
#include "JSObjectInlines.h"
#include "JSPromise.h"
#include "JSPromiseConstructor.h"

namespace JSC {

const ClassInfo JSPromiseDeferred::s_info = { "JSPromiseDeferred", 0, 0, CREATE_METHOD_TABLE(JSPromiseDeferred) };

JSValue newPromiseCapability(ExecState* exec, JSGlobalObject* globalObject, JSPromiseConstructor* promiseConstructor)
{
    JSFunction* newPromiseCapabilityFunction = globalObject->newPromiseCapabilityFunction();
    CallData callData;
    CallType callType = JSC::getCallData(newPromiseCapabilityFunction, callData);
    ASSERT(callType != CallType::None);

    MarkedArgumentBuffer arguments;
    arguments.append(promiseConstructor);
    return call(exec, newPromiseCapabilityFunction, callType, callData, jsUndefined(), arguments);
}


JSPromiseDeferred* JSPromiseDeferred::create(ExecState* exec, JSGlobalObject* globalObject)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    JSValue deferred = newPromiseCapability(exec, globalObject, globalObject->promiseConstructor());
    RETURN_IF_EXCEPTION(scope, nullptr);

    JSValue promise = deferred.get(exec, vm.propertyNames->builtinNames().promisePrivateName());
    ASSERT(promise.inherits(vm, JSPromise::info()));
    JSValue resolve = deferred.get(exec, vm.propertyNames->builtinNames().resolvePrivateName());
    JSValue reject = deferred.get(exec, vm.propertyNames->builtinNames().rejectPrivateName());

    return JSPromiseDeferred::create(vm, jsCast<JSPromise*>(promise), resolve, reject);
}

JSPromiseDeferred* JSPromiseDeferred::create(VM& vm, JSObject* promise, JSValue resolve, JSValue reject)
{
    JSPromiseDeferred* deferred = new (NotNull, allocateCell<JSPromiseDeferred>(vm.heap)) JSPromiseDeferred(vm);
    deferred->finishCreation(vm, promise, resolve, reject);
    return deferred;
}

JSPromiseDeferred::JSPromiseDeferred(VM& vm)
    : JSPromiseDeferred(vm, vm.promiseDeferredStructure.get())
{
}

JSPromiseDeferred::JSPromiseDeferred(VM& vm, Structure* structure)
    : Base(vm, structure)
{
}

static inline void callFunction(ExecState* exec, JSValue function, JSValue value)
{
    CallData callData;
    CallType callType = getCallData(function, callData);
    ASSERT(callType != CallType::None);

    MarkedArgumentBuffer arguments;
    arguments.append(value);

    call(exec, function, callType, callData, jsUndefined(), arguments);
}

void JSPromiseDeferred::resolve(ExecState* exec, JSValue value)
{
    callFunction(exec, m_resolve.get(), value);
}

void JSPromiseDeferred::reject(ExecState* exec, JSValue reason)
{
    callFunction(exec, m_reject.get(), reason);
}

void JSPromiseDeferred::finishCreation(VM& vm, JSObject* promise, JSValue resolve, JSValue reject)
{
    Base::finishCreation(vm);
    m_promise.set(vm, this, promise);
    m_resolve.set(vm, this, resolve);
    m_reject.set(vm, this, reject);
}

void JSPromiseDeferred::visitChildren(JSCell* cell, SlotVisitor& visitor)
{
    JSPromiseDeferred* thisObject = jsCast<JSPromiseDeferred*>(cell);
    ASSERT_GC_OBJECT_INHERITS(thisObject, info());

    Base::visitChildren(thisObject, visitor);

    visitor.append(thisObject->m_promise);
    visitor.append(thisObject->m_resolve);
    visitor.append(thisObject->m_reject);
}

} // namespace JSC
