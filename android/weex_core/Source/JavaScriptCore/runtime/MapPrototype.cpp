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
#include "MapPrototype.h"

#include "BuiltinNames.h"
#include "Error.h"
#include "ExceptionHelpers.h"
#include "IteratorOperations.h"
#include "JSCInlines.h"
#include "JSMap.h"
#include "JSMapIterator.h"
#include "Lookup.h"

#include "MapPrototype.lut.h"

namespace JSC {

const ClassInfo MapPrototype::s_info = { "Map", &Base::s_info, &mapPrototypeTable, CREATE_METHOD_TABLE(MapPrototype) };

/* Source for MapPrototype.lut.h
@begin mapPrototypeTable
  forEach   JSBuiltin  DontEnum|Function 0
@end
*/

static EncodedJSValue JSC_HOST_CALL mapProtoFuncClear(ExecState*);
static EncodedJSValue JSC_HOST_CALL mapProtoFuncDelete(ExecState*);
static EncodedJSValue JSC_HOST_CALL mapProtoFuncGet(ExecState*);
static EncodedJSValue JSC_HOST_CALL mapProtoFuncHas(ExecState*);
static EncodedJSValue JSC_HOST_CALL mapProtoFuncSet(ExecState*);
static EncodedJSValue JSC_HOST_CALL mapProtoFuncKeys(ExecState*);
static EncodedJSValue JSC_HOST_CALL mapProtoFuncValues(ExecState*);
static EncodedJSValue JSC_HOST_CALL mapProtoFuncEntries(ExecState*);

static EncodedJSValue JSC_HOST_CALL mapProtoFuncSize(ExecState*);
    
void MapPrototype::finishCreation(VM& vm, JSGlobalObject* globalObject)
{
    Base::finishCreation(vm);
    ASSERT(inherits(vm, info()));
    vm.prototypeMap.addPrototype(this);

    JSC_NATIVE_FUNCTION_WITHOUT_TRANSITION(vm.propertyNames->clear, mapProtoFuncClear, DontEnum, 0);
    JSC_NATIVE_FUNCTION_WITHOUT_TRANSITION(vm.propertyNames->deleteKeyword, mapProtoFuncDelete, DontEnum, 1);
    JSC_NATIVE_INTRINSIC_FUNCTION_WITHOUT_TRANSITION(vm.propertyNames->get, mapProtoFuncGet, DontEnum, 1, JSMapGetIntrinsic);
    JSC_NATIVE_INTRINSIC_FUNCTION_WITHOUT_TRANSITION(vm.propertyNames->has, mapProtoFuncHas, DontEnum, 1, JSMapHasIntrinsic);
    JSC_NATIVE_FUNCTION_WITHOUT_TRANSITION(vm.propertyNames->set, mapProtoFuncSet, DontEnum, 2);
    JSC_NATIVE_FUNCTION_WITHOUT_TRANSITION(vm.propertyNames->builtinNames().keysPublicName(), mapProtoFuncKeys, DontEnum, 0);
    JSC_NATIVE_FUNCTION_WITHOUT_TRANSITION(vm.propertyNames->builtinNames().valuesPublicName(), mapProtoFuncValues, DontEnum, 0);

    JSC_NATIVE_INTRINSIC_FUNCTION_WITHOUT_TRANSITION(vm.propertyNames->builtinNames().getPrivateName(), mapProtoFuncGet, DontEnum, 1, JSMapGetIntrinsic);
    JSC_NATIVE_FUNCTION_WITHOUT_TRANSITION(vm.propertyNames->builtinNames().setPrivateName(), mapProtoFuncSet, DontEnum, 2);

    JSFunction* entries = JSFunction::create(vm, globalObject, 0, vm.propertyNames->builtinNames().entriesPublicName().string(), mapProtoFuncEntries);
    putDirectWithoutTransition(vm, vm.propertyNames->builtinNames().entriesPublicName(), entries, DontEnum);
    putDirectWithoutTransition(vm, vm.propertyNames->iteratorSymbol, entries, DontEnum);
    putDirectWithoutTransition(vm, vm.propertyNames->toStringTagSymbol, jsString(&vm, "Map"), DontEnum | ReadOnly);

    JSC_NATIVE_GETTER(vm.propertyNames->size, mapProtoFuncSize, DontEnum | Accessor);
}

ALWAYS_INLINE static JSMap* getMap(CallFrame* callFrame, JSValue thisValue)
{
    VM& vm = callFrame->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    if (UNLIKELY(!thisValue.isCell())) {
        throwVMError(callFrame, scope, createNotAnObjectError(callFrame, thisValue));
        return nullptr;
    }

    if (LIKELY(thisValue.asCell()->type() == JSMapType))
        return jsCast<JSMap*>(thisValue);
    throwTypeError(callFrame, scope, ASCIILiteral("Map operation called on non-Map object"));
    return nullptr;
}

EncodedJSValue JSC_HOST_CALL mapProtoFuncClear(CallFrame* callFrame)
{
    JSMap* map = getMap(callFrame, callFrame->thisValue());
    if (!map)
        return JSValue::encode(jsUndefined());
    map->clear(callFrame);
    return JSValue::encode(jsUndefined());
}

EncodedJSValue JSC_HOST_CALL mapProtoFuncDelete(CallFrame* callFrame)
{
    JSMap* map = getMap(callFrame, callFrame->thisValue());
    if (!map)
        return JSValue::encode(jsUndefined());
    return JSValue::encode(jsBoolean(map->remove(callFrame, callFrame->argument(0))));
}

EncodedJSValue JSC_HOST_CALL mapProtoFuncGet(CallFrame* callFrame)
{
    JSMap* map = getMap(callFrame, callFrame->thisValue());
    if (!map)
        return JSValue::encode(jsUndefined());
    return JSValue::encode(map->get(callFrame, callFrame->argument(0)));
}

EncodedJSValue JSC_HOST_CALL mapProtoFuncHas(CallFrame* callFrame)
{
    JSMap* map = getMap(callFrame, callFrame->thisValue());
    if (!map)
        return JSValue::encode(jsUndefined());
    return JSValue::encode(jsBoolean(map->has(callFrame, callFrame->argument(0))));
}

EncodedJSValue JSC_HOST_CALL mapProtoFuncSet(CallFrame* callFrame)
{
    JSValue thisValue = callFrame->thisValue();
    JSMap* map = getMap(callFrame, thisValue);
    if (!map)
        return JSValue::encode(jsUndefined());
    map->set(callFrame, callFrame->argument(0), callFrame->argument(1));
    return JSValue::encode(thisValue);
}

EncodedJSValue JSC_HOST_CALL mapProtoFuncSize(CallFrame* callFrame)
{
    JSMap* map = getMap(callFrame, callFrame->thisValue());
    if (!map)
        return JSValue::encode(jsUndefined());
    return JSValue::encode(jsNumber(map->size(callFrame)));
}

EncodedJSValue JSC_HOST_CALL mapProtoFuncValues(CallFrame* callFrame)
{
    VM& vm = callFrame->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    JSMap* thisObj = jsDynamicCast<JSMap*>(vm, callFrame->thisValue());
    if (!thisObj)
        return JSValue::encode(throwTypeError(callFrame, scope, ASCIILiteral("Cannot create a Map value iterator for a non-Map object.")));
    return JSValue::encode(JSMapIterator::create(vm, callFrame->jsCallee()->globalObject()->mapIteratorStructure(), thisObj, IterateValue));
}

EncodedJSValue JSC_HOST_CALL mapProtoFuncEntries(CallFrame* callFrame)
{
    VM& vm = callFrame->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    JSMap* thisObj = jsDynamicCast<JSMap*>(vm, callFrame->thisValue());
    if (!thisObj)
        return JSValue::encode(throwTypeError(callFrame, scope, ASCIILiteral("Cannot create a Map entry iterator for a non-Map object.")));
    return JSValue::encode(JSMapIterator::create(vm, callFrame->jsCallee()->globalObject()->mapIteratorStructure(), thisObj, IterateKeyValue));
}

EncodedJSValue JSC_HOST_CALL mapProtoFuncKeys(CallFrame* callFrame)
{
    VM& vm = callFrame->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    JSMap* thisObj = jsDynamicCast<JSMap*>(vm, callFrame->thisValue());
    if (!thisObj)
        return JSValue::encode(throwTypeError(callFrame, scope, ASCIILiteral("Cannot create a Map key iterator for a non-Map object.")));
    return JSValue::encode(JSMapIterator::create(vm, callFrame->jsCallee()->globalObject()->mapIteratorStructure(), thisObj, IterateKey));
}

EncodedJSValue JSC_HOST_CALL privateFuncMapIterator(ExecState* exec)
{
    ASSERT(jsDynamicCast<JSMap*>(exec->vm(), exec->uncheckedArgument(0)));
    JSMap* map = jsCast<JSMap*>(exec->uncheckedArgument(0));
    return JSValue::encode(JSMapIterator::create(exec->vm(), exec->jsCallee()->globalObject()->mapIteratorStructure(), map, IterateKeyValue));
}

EncodedJSValue JSC_HOST_CALL privateFuncMapIteratorNext(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);
    ASSERT(jsDynamicCast<JSMapIterator*>(vm, exec->thisValue()));
    JSMapIterator* iterator = jsCast<JSMapIterator*>(exec->thisValue());
    JSValue key, value;
    if (iterator->nextKeyValue(exec, key, value)) {
        JSArray* resultArray = jsCast<JSArray*>(exec->uncheckedArgument(0));
        resultArray->putDirectIndex(exec, 0, key);
        RETURN_IF_EXCEPTION(scope, encodedJSValue());
        scope.release();
        resultArray->putDirectIndex(exec, 1, value);
        return JSValue::encode(jsBoolean(false));
    }
    return JSValue::encode(jsBoolean(true));
}

}
