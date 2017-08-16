/*
 * Copyright (C) 2015-2016 Apple, Inc. All rights reserved.
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
#include "WeakSetConstructor.h"

#include "Error.h"
#include "IteratorOperations.h"
#include "JSCInlines.h"
#include "JSGlobalObject.h"
#include "JSObjectInlines.h"
#include "JSWeakSet.h"
#include "WeakSetPrototype.h"

namespace JSC {

const ClassInfo WeakSetConstructor::s_info = { "Function", &Base::s_info, 0, CREATE_METHOD_TABLE(WeakSetConstructor) };

void WeakSetConstructor::finishCreation(VM& vm, WeakSetPrototype* prototype)
{
    Base::finishCreation(vm, prototype->classInfo(vm)->className);
    putDirectWithoutTransition(vm, vm.propertyNames->prototype, prototype, DontEnum | DontDelete | ReadOnly);
    putDirectWithoutTransition(vm, vm.propertyNames->length, jsNumber(0), DontEnum | ReadOnly);
}

static EncodedJSValue JSC_HOST_CALL callWeakSet(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);
    return JSValue::encode(throwConstructorCannotBeCalledAsFunctionTypeError(exec, scope, "WeakSet"));
}

static EncodedJSValue JSC_HOST_CALL constructWeakSet(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    JSGlobalObject* globalObject = asInternalFunction(exec->jsCallee())->globalObject();
    Structure* weakSetStructure = InternalFunction::createSubclassStructure(exec, exec->newTarget(), globalObject->weakSetStructure());
    RETURN_IF_EXCEPTION(scope, encodedJSValue());
    JSWeakSet* weakSet = JSWeakSet::create(exec, weakSetStructure);
    JSValue iterable = exec->argument(0);
    if (iterable.isUndefinedOrNull())
        return JSValue::encode(weakSet);

    JSValue adderFunction = weakSet->JSObject::get(exec, vm.propertyNames->add);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());

    CallData adderFunctionCallData;
    CallType adderFunctionCallType = getCallData(adderFunction, adderFunctionCallData);
    if (adderFunctionCallType == CallType::None)
        return JSValue::encode(throwTypeError(exec, scope));

    scope.release();
    forEachInIterable(exec, iterable, [&](VM&, ExecState* exec, JSValue nextValue) {
        MarkedArgumentBuffer arguments;
        arguments.append(nextValue);
        call(exec, adderFunction, adderFunctionCallType, adderFunctionCallData, weakSet, arguments);
    });

    return JSValue::encode(weakSet);
}

ConstructType WeakSetConstructor::getConstructData(JSCell*, ConstructData& constructData)
{
    constructData.native.function = constructWeakSet;
    return ConstructType::Host;
}

CallType WeakSetConstructor::getCallData(JSCell*, CallData& callData)
{
    callData.native.function = callWeakSet;
    return CallType::Host;
}

}
