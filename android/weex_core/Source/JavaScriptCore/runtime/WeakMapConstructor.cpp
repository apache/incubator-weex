/*
 * Copyright (C) 2013, 2016 Apple, Inc. All rights reserved.
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
#include "WeakMapConstructor.h"

#include "Error.h"
#include "IteratorOperations.h"
#include "JSCInlines.h"
#include "JSGlobalObject.h"
#include "JSObjectInlines.h"
#include "JSWeakMap.h"
#include "WeakMapPrototype.h"

namespace JSC {

const ClassInfo WeakMapConstructor::s_info = { "Function", &Base::s_info, 0, CREATE_METHOD_TABLE(WeakMapConstructor) };

void WeakMapConstructor::finishCreation(VM& vm, WeakMapPrototype* prototype)
{
    Base::finishCreation(vm, prototype->classInfo(vm)->className);
    putDirectWithoutTransition(vm, vm.propertyNames->prototype, prototype, DontEnum | DontDelete | ReadOnly);
    putDirectWithoutTransition(vm, vm.propertyNames->length, jsNumber(0), DontEnum | ReadOnly);
}

static EncodedJSValue JSC_HOST_CALL callWeakMap(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);
    return JSValue::encode(throwConstructorCannotBeCalledAsFunctionTypeError(exec, scope, "WeakMap"));
}

static EncodedJSValue JSC_HOST_CALL constructWeakMap(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    JSGlobalObject* globalObject = asInternalFunction(exec->jsCallee())->globalObject();
    Structure* weakMapStructure = InternalFunction::createSubclassStructure(exec, exec->newTarget(), globalObject->weakMapStructure());
    RETURN_IF_EXCEPTION(scope, encodedJSValue());
    JSWeakMap* weakMap = JSWeakMap::create(exec, weakMapStructure);
    JSValue iterable = exec->argument(0);
    if (iterable.isUndefinedOrNull())
        return JSValue::encode(weakMap);

    JSValue adderFunction = weakMap->JSObject::get(exec, vm.propertyNames->set);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());

    CallData adderFunctionCallData;
    CallType adderFunctionCallType = getCallData(adderFunction, adderFunctionCallData);
    if (adderFunctionCallType == CallType::None)
        return JSValue::encode(throwTypeError(exec, scope));

    scope.release();
    forEachInIterable(exec, iterable, [&](VM& vm, ExecState* exec, JSValue nextItem) {
        auto scope = DECLARE_THROW_SCOPE(vm);
        if (!nextItem.isObject()) {
            throwTypeError(exec, scope);
            return;
        }

        JSValue key = nextItem.get(exec, static_cast<unsigned>(0));
        RETURN_IF_EXCEPTION(scope, void());

        JSValue value = nextItem.get(exec, static_cast<unsigned>(1));
        RETURN_IF_EXCEPTION(scope, void());

        MarkedArgumentBuffer arguments;
        arguments.append(key);
        arguments.append(value);
        scope.release();
        call(exec, adderFunction, adderFunctionCallType, adderFunctionCallData, weakMap, arguments);
    });

    return JSValue::encode(weakMap);
}

ConstructType WeakMapConstructor::getConstructData(JSCell*, ConstructData& constructData)
{
    constructData.native.function = constructWeakMap;
    return ConstructType::Host;
}

CallType WeakMapConstructor::getCallData(JSCell*, CallData& callData)
{
    callData.native.function = callWeakMap;
    return CallType::Host;
}

}
