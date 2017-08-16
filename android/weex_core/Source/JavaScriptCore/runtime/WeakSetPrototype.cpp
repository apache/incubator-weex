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
#include "WeakSetPrototype.h"

#include "JSCInlines.h"
#include "JSWeakSet.h"
#include "WeakMapData.h"

namespace JSC {

const ClassInfo WeakSetPrototype::s_info = { "WeakSet", &Base::s_info, 0, CREATE_METHOD_TABLE(WeakSetPrototype) };

static EncodedJSValue JSC_HOST_CALL protoFuncWeakSetDelete(ExecState*);
static EncodedJSValue JSC_HOST_CALL protoFuncWeakSetHas(ExecState*);
static EncodedJSValue JSC_HOST_CALL protoFuncWeakSetAdd(ExecState*);

void WeakSetPrototype::finishCreation(VM& vm, JSGlobalObject* globalObject)
{
    Base::finishCreation(vm);
    ASSERT(inherits(vm, info()));
    vm.prototypeMap.addPrototype(this);

    JSC_NATIVE_FUNCTION_WITHOUT_TRANSITION(vm.propertyNames->deleteKeyword, protoFuncWeakSetDelete, DontEnum, 1);
    JSC_NATIVE_FUNCTION_WITHOUT_TRANSITION(vm.propertyNames->has, protoFuncWeakSetHas, DontEnum, 1);
    JSC_NATIVE_FUNCTION_WITHOUT_TRANSITION(vm.propertyNames->add, protoFuncWeakSetAdd, DontEnum, 1);

    putDirectWithoutTransition(vm, vm.propertyNames->toStringTagSymbol, jsString(&vm, "WeakSet"), DontEnum | ReadOnly);
}

static WeakMapData* getWeakMapData(CallFrame* callFrame, JSValue value)
{
    VM& vm = callFrame->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    if (!value.isObject()) {
        throwTypeError(callFrame, scope, WTF::ASCIILiteral("Called WeakSet function on non-object"));
        return nullptr;
    }

    if (JSWeakSet* weakSet = jsDynamicCast<JSWeakSet*>(vm, value))
        return weakSet->weakMapData();

    throwTypeError(callFrame, scope, WTF::ASCIILiteral("Called WeakSet function on a non-WeakSet object"));
    return nullptr;
}

EncodedJSValue JSC_HOST_CALL protoFuncWeakSetDelete(CallFrame* callFrame)
{
    WeakMapData* map = getWeakMapData(callFrame, callFrame->thisValue());
    if (!map)
        return JSValue::encode(jsUndefined());
    JSValue key = callFrame->argument(0);
    return JSValue::encode(jsBoolean(key.isObject() && map->remove(asObject(key))));
}

EncodedJSValue JSC_HOST_CALL protoFuncWeakSetHas(CallFrame* callFrame)
{
    WeakMapData* map = getWeakMapData(callFrame, callFrame->thisValue());
    if (!map)
        return JSValue::encode(jsUndefined());
    JSValue key = callFrame->argument(0);
    return JSValue::encode(jsBoolean(key.isObject() && map->contains(asObject(key))));
}

EncodedJSValue JSC_HOST_CALL protoFuncWeakSetAdd(CallFrame* callFrame)
{
    VM& vm = callFrame->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    WeakMapData* map = getWeakMapData(callFrame, callFrame->thisValue());
    ASSERT(!!scope.exception() == !map);
    if (!map)
        return JSValue::encode(jsUndefined());
    JSValue key = callFrame->argument(0);
    if (!key.isObject())
        return JSValue::encode(throwTypeError(callFrame, scope, WTF::ASCIILiteral("Attempted to add a non-object key to a WeakSet")));
    map->set(vm, asObject(key), jsUndefined());
    return JSValue::encode(callFrame->thisValue());
}

}
