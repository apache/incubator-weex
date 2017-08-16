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
#include "JSPromise.h"

#include "BuiltinNames.h"
#include "Error.h"
#include "JSCInlines.h"
#include "JSPromiseConstructor.h"
#include "Microtask.h"

namespace JSC {

const ClassInfo JSPromise::s_info = { "Promise", &Base::s_info, 0, CREATE_METHOD_TABLE(JSPromise) };

JSPromise* JSPromise::create(VM& vm, Structure* structure)
{
    JSPromise* promise = new (NotNull, allocateCell<JSPromise>(vm.heap)) JSPromise(vm, structure);
    promise->finishCreation(vm);
    return promise;
}

Structure* JSPromise::createStructure(VM& vm, JSGlobalObject* globalObject, JSValue prototype)
{
    return Structure::create(vm, globalObject, prototype, TypeInfo(ObjectType, StructureFlags), info());
}

JSPromise::JSPromise(VM& vm, Structure* structure)
    : Base(vm, structure)
{
}

void JSPromise::finishCreation(VM& vm)
{
    Base::finishCreation(vm);
    putDirect(vm, vm.propertyNames->builtinNames().promiseStatePrivateName(), jsNumber(static_cast<unsigned>(Status::Pending)));
    putDirect(vm, vm.propertyNames->builtinNames().promiseReactionsPrivateName(), jsUndefined());
    putDirect(vm, vm.propertyNames->builtinNames().promiseResultPrivateName(), jsUndefined());
}

void JSPromise::initialize(ExecState* exec, JSGlobalObject* globalObject, JSValue executor)
{
    JSFunction* initializePromise = globalObject->initializePromiseFunction();
    CallData callData;
    CallType callType = JSC::getCallData(initializePromise, callData);
    ASSERT(callType != CallType::None);

    MarkedArgumentBuffer arguments;
    arguments.append(executor);
    call(exec, initializePromise, callType, callData, this, arguments);
}

auto JSPromise::status(VM& vm) const -> Status
{
    JSValue value = getDirect(vm, vm.propertyNames->builtinNames().promiseStatePrivateName());
    ASSERT(value.isUInt32());
    return static_cast<Status>(value.asUInt32());
}

JSValue JSPromise::result(VM& vm) const
{
    return getDirect(vm, vm.propertyNames->builtinNames().promiseResultPrivateName());
}

} // namespace JSC
