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
#include "WeakMapPrototype.h"

#include "JSCInlines.h"
#include "JSWeakMap.h"
#include "WeakMapData.h"

namespace JSC {

const ClassInfo WeakMapPrototype::s_info = { "WeakMap", &Base::s_info, 0, CREATE_METHOD_TABLE(WeakMapPrototype) };

static EncodedJSValue JSC_HOST_CALL protoFuncWeakMapDelete(ExecState*);
static EncodedJSValue JSC_HOST_CALL protoFuncWeakMapGet(ExecState*);
static EncodedJSValue JSC_HOST_CALL protoFuncWeakMapHas(ExecState*);
static EncodedJSValue JSC_HOST_CALL protoFuncWeakMapSet(ExecState*);

void WeakMapPrototype::finishCreation(VM& vm, JSGlobalObject* globalObject)
{
    Base::finishCreation(vm);
    ASSERT(inherits(vm, info()));
    vm.prototypeMap.addPrototype(this);

    JSC_NATIVE_FUNCTION_WITHOUT_TRANSITION(vm.propertyNames->deleteKeyword, protoFuncWeakMapDelete, DontEnum, 1);
    JSC_NATIVE_FUNCTION_WITHOUT_TRANSITION(vm.propertyNames->get, protoFuncWeakMapGet, DontEnum, 1);
    JSC_NATIVE_FUNCTION_WITHOUT_TRANSITION(vm.propertyNames->has, protoFuncWeakMapHas, DontEnum, 1);
    JSC_NATIVE_FUNCTION_WITHOUT_TRANSITION(vm.propertyNames->set, protoFuncWeakMapSet, DontEnum, 2);

    putDirectWithoutTransition(vm, vm.propertyNames->toStringTagSymbol, jsString(&vm, "WeakMap"), DontEnum | ReadOnly);
}

static WeakMapData* getWeakMapData(CallFrame* callFrame, JSValue value)
{
    VM& vm = callFrame->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    if (!value.isObject()) {
        throwTypeError(callFrame, scope, WTF::ASCIILiteral("Called WeakMap function on non-object"));
        return nullptr;
    }

    if (JSWeakMap* weakMap = jsDynamicCast<JSWeakMap*>(vm, value))
        return weakMap->weakMapData();

    throwTypeError(callFrame, scope, WTF::ASCIILiteral("Called WeakMap function on a non-WeakMap object"));
    return nullptr;
}

EncodedJSValue JSC_HOST_CALL protoFuncWeakMapDelete(CallFrame* callFrame)
{
    WeakMapData* map = getWeakMapData(callFrame, callFrame->thisValue());
    if (!map)
        return JSValue::encode(jsUndefined());
    JSValue key = callFrame->argument(0);
    return JSValue::encode(jsBoolean(key.isObject() && map->remove(asObject(key))));
}

EncodedJSValue JSC_HOST_CALL protoFuncWeakMapGet(CallFrame* callFrame)
{
    WeakMapData* map = getWeakMapData(callFrame, callFrame->thisValue());
    if (!map)
        return JSValue::encode(jsUndefined());
    JSValue key = callFrame->argument(0);
    if (!key.isObject())
        return JSValue::encode(jsUndefined());
    return JSValue::encode(map->get(asObject(key)));
}

EncodedJSValue JSC_HOST_CALL protoFuncWeakMapHas(CallFrame* callFrame)
{
    WeakMapData* map = getWeakMapData(callFrame, callFrame->thisValue());
    if (!map)
        return JSValue::encode(jsUndefined());
    JSValue key = callFrame->argument(0);
    return JSValue::encode(jsBoolean(key.isObject() && map->contains(asObject(key))));
}

EncodedJSValue JSC_HOST_CALL protoFuncWeakMapSet(CallFrame* callFrame)
{
    VM& vm = callFrame->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    WeakMapData* map = getWeakMapData(callFrame, callFrame->thisValue());
    ASSERT(!!scope.exception() == !map);
    if (!map)
        return JSValue::encode(jsUndefined());
    JSValue key = callFrame->argument(0);
    if (!key.isObject())
        return JSValue::encode(throwTypeError(callFrame, scope, WTF::ASCIILiteral("Attempted to set a non-object key in a WeakMap")));
    map->set(vm, asObject(key), callFrame->argument(1));
    return JSValue::encode(callFrame->thisValue());
}

}
