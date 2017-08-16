/*
 * Copyright (C) 2015 Apple Inc. All rights reserved.
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
#include "JSInternalPromiseDeferred.h"

#include "BuiltinNames.h"
#include "Error.h"
#include "Exception.h"
#include "JSCInlines.h"
#include "JSInternalPromise.h"
#include "JSInternalPromiseConstructor.h"

namespace JSC {

const ClassInfo JSInternalPromiseDeferred::s_info = { "JSInternalPromiseDeferred", &Base::s_info, nullptr, CREATE_METHOD_TABLE(JSInternalPromiseDeferred) };

JSInternalPromiseDeferred* JSInternalPromiseDeferred::create(ExecState* exec, JSGlobalObject* globalObject)
{
    VM& vm = exec->vm();

    JSValue deferred = newPromiseCapability(exec, globalObject, globalObject->internalPromiseConstructor());

    JSValue promise = deferred.get(exec, vm.propertyNames->builtinNames().promisePrivateName());
    ASSERT(promise.inherits(vm, JSInternalPromise::info()));
    JSValue resolve = deferred.get(exec, vm.propertyNames->builtinNames().resolvePrivateName());
    JSValue reject = deferred.get(exec, vm.propertyNames->builtinNames().rejectPrivateName());

    JSInternalPromiseDeferred* result = new (NotNull, allocateCell<JSInternalPromiseDeferred>(vm.heap)) JSInternalPromiseDeferred(vm);
    result->finishCreation(vm, jsCast<JSObject*>(promise), resolve, reject);
    return result;
}

JSInternalPromiseDeferred::JSInternalPromiseDeferred(VM& vm)
    : Base(vm, vm.internalPromiseDeferredStructure.get())
{
}

JSInternalPromise* JSInternalPromiseDeferred::promise() const
{
    return jsCast<JSInternalPromise*>(Base::promise());
}

JSInternalPromise* JSInternalPromiseDeferred::resolve(ExecState* exec, JSValue value)
{
    Base::resolve(exec, value);
    return promise();
}

JSInternalPromise* JSInternalPromiseDeferred::reject(ExecState* exec, JSValue reason)
{
    Base::reject(exec, reason);
    return promise();
}

} // namespace JSC
