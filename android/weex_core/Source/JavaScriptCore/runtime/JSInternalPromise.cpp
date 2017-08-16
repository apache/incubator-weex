/*
 * Copyright (C) 2015-2016 Apple Inc. All rights reserved.
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
#include "JSInternalPromise.h"

#include "BuiltinNames.h"
#include "JSCInlines.h"

namespace JSC {

const ClassInfo JSInternalPromise::s_info = { "InternalPromise", &Base::s_info, nullptr, CREATE_METHOD_TABLE(JSInternalPromise) };

JSInternalPromise* JSInternalPromise::create(VM& vm, Structure* structure)
{
    JSInternalPromise* promise = new (NotNull, allocateCell<JSInternalPromise>(vm.heap)) JSInternalPromise(vm, structure);
    promise->finishCreation(vm);
    return promise;
}

Structure* JSInternalPromise::createStructure(VM& vm, JSGlobalObject* globalObject, JSValue prototype)
{
    return Structure::create(vm, globalObject, prototype, TypeInfo(ObjectType, StructureFlags), info());
}

JSInternalPromise::JSInternalPromise(VM& vm, Structure* structure)
    : Base(vm, structure)
{
}

JSInternalPromise* JSInternalPromise::then(ExecState* exec, JSFunction* onFulfilled, JSFunction* onRejected)
{
    JSObject* function = jsCast<JSObject*>(get(exec, exec->propertyNames().builtinNames().thenPublicName()));
    CallData callData;
    CallType callType = JSC::getCallData(function, callData);
    ASSERT(callType != CallType::None);

    MarkedArgumentBuffer arguments;
    arguments.append(onFulfilled ? onFulfilled : jsUndefined());
    arguments.append(onRejected ? onRejected : jsUndefined());

    return jsCast<JSInternalPromise*>(call(exec, function, callType, callData, this, arguments));
}

} // namespace JSC
