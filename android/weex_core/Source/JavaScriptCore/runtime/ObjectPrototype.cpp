/*
 *  Copyright (C) 1999-2000 Harri Porten (porten@kde.org)
 *  Copyright (C) 2008, 2011, 2016 Apple Inc. All rights reserved.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include "config.h"
#include "ObjectPrototype.h"

#include "Error.h"
#include "GetterSetter.h"
#include "HasOwnPropertyCache.h"
#include "JSFunction.h"
#include "JSString.h"
#include "JSCInlines.h"
#include "PropertySlot.h"
#include "StructureInlines.h"
#include "StructureRareDataInlines.h"

namespace JSC {

static EncodedJSValue JSC_HOST_CALL objectProtoFuncValueOf(ExecState*);
static EncodedJSValue JSC_HOST_CALL objectProtoFuncHasOwnProperty(ExecState*);
static EncodedJSValue JSC_HOST_CALL objectProtoFuncIsPrototypeOf(ExecState*);
static EncodedJSValue JSC_HOST_CALL objectProtoFuncDefineGetter(ExecState*);
static EncodedJSValue JSC_HOST_CALL objectProtoFuncDefineSetter(ExecState*);
static EncodedJSValue JSC_HOST_CALL objectProtoFuncLookupGetter(ExecState*);
static EncodedJSValue JSC_HOST_CALL objectProtoFuncLookupSetter(ExecState*);
static EncodedJSValue JSC_HOST_CALL objectProtoFuncPropertyIsEnumerable(ExecState*);
static EncodedJSValue JSC_HOST_CALL objectProtoFuncToLocaleString(ExecState*);

STATIC_ASSERT_IS_TRIVIALLY_DESTRUCTIBLE(ObjectPrototype);

const ClassInfo ObjectPrototype::s_info = { "Object", &JSNonFinalObject::s_info, 0, CREATE_METHOD_TABLE(ObjectPrototype) };

ObjectPrototype::ObjectPrototype(VM& vm, Structure* stucture)
    : JSNonFinalObject(vm, stucture)
{
}

void ObjectPrototype::finishCreation(VM& vm, JSGlobalObject* globalObject)
{
    Base::finishCreation(vm);
    ASSERT(inherits(vm, info()));
    vm.prototypeMap.addPrototype(this);
    
    JSC_NATIVE_FUNCTION_WITHOUT_TRANSITION(vm.propertyNames->toString, objectProtoFuncToString, DontEnum, 0);
    JSC_NATIVE_FUNCTION_WITHOUT_TRANSITION(vm.propertyNames->toLocaleString, objectProtoFuncToLocaleString, DontEnum, 0);
    JSC_NATIVE_FUNCTION_WITHOUT_TRANSITION(vm.propertyNames->valueOf, objectProtoFuncValueOf, DontEnum, 0);
    JSC_NATIVE_INTRINSIC_FUNCTION_WITHOUT_TRANSITION(vm.propertyNames->hasOwnProperty, objectProtoFuncHasOwnProperty, DontEnum, 1, HasOwnPropertyIntrinsic);
    JSC_NATIVE_FUNCTION_WITHOUT_TRANSITION(vm.propertyNames->propertyIsEnumerable, objectProtoFuncPropertyIsEnumerable, DontEnum, 1);
    JSC_NATIVE_FUNCTION_WITHOUT_TRANSITION(vm.propertyNames->isPrototypeOf, objectProtoFuncIsPrototypeOf, DontEnum, 1);
    JSC_NATIVE_FUNCTION_WITHOUT_TRANSITION(vm.propertyNames->__defineGetter__, objectProtoFuncDefineGetter, DontEnum, 2);
    JSC_NATIVE_FUNCTION_WITHOUT_TRANSITION(vm.propertyNames->__defineSetter__, objectProtoFuncDefineSetter, DontEnum, 2);
    JSC_NATIVE_FUNCTION_WITHOUT_TRANSITION(vm.propertyNames->__lookupGetter__, objectProtoFuncLookupGetter, DontEnum, 1);
    JSC_NATIVE_FUNCTION_WITHOUT_TRANSITION(vm.propertyNames->__lookupSetter__, objectProtoFuncLookupSetter, DontEnum, 1);
}

ObjectPrototype* ObjectPrototype::create(VM& vm, JSGlobalObject* globalObject, Structure* structure)
{
    ObjectPrototype* prototype = new (NotNull, allocateCell<ObjectPrototype>(vm.heap)) ObjectPrototype(vm, structure);
    prototype->finishCreation(vm, globalObject);
    return prototype;
}

// ------------------------------ Functions --------------------------------

EncodedJSValue JSC_HOST_CALL objectProtoFuncValueOf(ExecState* exec)
{
    JSValue thisValue = exec->thisValue().toThis(exec, StrictMode);
    JSObject* valueObj = thisValue.toObject(exec);
    if (UNLIKELY(!valueObj))
        return encodedJSValue();
    return JSValue::encode(valueObj);
}

EncodedJSValue JSC_HOST_CALL objectProtoFuncHasOwnProperty(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    JSValue thisValue = exec->thisValue().toThis(exec, StrictMode);
    auto propertyName = exec->argument(0).toPropertyKey(exec);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());
    JSObject* thisObject = thisValue.toObject(exec);
    ASSERT(!!scope.exception() == !thisObject);
    if (UNLIKELY(!thisObject))
        return encodedJSValue();

    Structure* structure = thisObject->structure(vm);
    HasOwnPropertyCache* hasOwnPropertyCache = vm.ensureHasOwnPropertyCache();
    if (std::optional<bool> result = hasOwnPropertyCache->get(structure, propertyName)) {
        ASSERT(*result == thisObject->hasOwnProperty(exec, propertyName));
        ASSERT(!scope.exception());
        return JSValue::encode(jsBoolean(*result));
    }

    PropertySlot slot(thisObject, PropertySlot::InternalMethodType::GetOwnProperty);
    bool result = thisObject->hasOwnProperty(exec, propertyName, slot);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());

    hasOwnPropertyCache->tryAdd(vm, slot, thisObject, propertyName, result);
    return JSValue::encode(jsBoolean(result));
}

EncodedJSValue JSC_HOST_CALL objectProtoFuncIsPrototypeOf(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    JSValue thisValue = exec->thisValue().toThis(exec, StrictMode);
    JSObject* thisObj = thisValue.toObject(exec);
    ASSERT(!!scope.exception() == !thisObj);
    if (UNLIKELY(!thisObj))
        return encodedJSValue();

    if (!exec->argument(0).isObject())
        return JSValue::encode(jsBoolean(false));

    JSValue v = asObject(exec->argument(0))->getPrototype(vm, exec);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());

    while (true) {
        if (!v.isObject())
            return JSValue::encode(jsBoolean(false));
        if (v == thisObj)
            return JSValue::encode(jsBoolean(true));
        v = asObject(v)->getPrototype(vm, exec);
        RETURN_IF_EXCEPTION(scope, encodedJSValue());
    }
}

EncodedJSValue JSC_HOST_CALL objectProtoFuncDefineGetter(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    JSObject* thisObject = exec->thisValue().toThis(exec, StrictMode).toObject(exec);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());

    JSValue get = exec->argument(1);
    CallData callData;
    if (getCallData(get, callData) == CallType::None)
        return throwVMTypeError(exec, scope, ASCIILiteral("invalid getter usage"));

    auto propertyName = exec->argument(0).toPropertyKey(exec);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());

    PropertyDescriptor descriptor;
    descriptor.setGetter(get);
    descriptor.setEnumerable(true);
    descriptor.setConfigurable(true);

    bool shouldThrow = true;
    scope.release();
    thisObject->methodTable(vm)->defineOwnProperty(thisObject, exec, propertyName, descriptor, shouldThrow);

    return JSValue::encode(jsUndefined());
}

EncodedJSValue JSC_HOST_CALL objectProtoFuncDefineSetter(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    JSObject* thisObject = exec->thisValue().toThis(exec, StrictMode).toObject(exec);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());

    JSValue set = exec->argument(1);
    CallData callData;
    if (getCallData(set, callData) == CallType::None)
        return throwVMTypeError(exec, scope, ASCIILiteral("invalid setter usage"));

    auto propertyName = exec->argument(0).toPropertyKey(exec);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());

    PropertyDescriptor descriptor;
    descriptor.setSetter(set);
    descriptor.setEnumerable(true);
    descriptor.setConfigurable(true);

    bool shouldThrow = true;
    scope.release();
    thisObject->methodTable(vm)->defineOwnProperty(thisObject, exec, propertyName, descriptor, shouldThrow);

    return JSValue::encode(jsUndefined());
}

EncodedJSValue JSC_HOST_CALL objectProtoFuncLookupGetter(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    JSObject* thisObject = exec->thisValue().toThis(exec, StrictMode).toObject(exec);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());

    auto propertyName = exec->argument(0).toPropertyKey(exec);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());

    PropertySlot slot(thisObject, PropertySlot::InternalMethodType::GetOwnProperty);
    bool hasProperty = thisObject->getPropertySlot(exec, propertyName, slot);
    ASSERT(!scope.exception() || !hasProperty);
    if (hasProperty) {
        if (slot.isAccessor()) {
            GetterSetter* getterSetter = slot.getterSetter();
            return getterSetter->isGetterNull() ? JSValue::encode(jsUndefined()) : JSValue::encode(getterSetter->getter());
        }
        if (slot.attributes() & CustomAccessor) {
            PropertyDescriptor descriptor;
            ASSERT(slot.slotBase());
            if (slot.slotBase()->getOwnPropertyDescriptor(exec, propertyName, descriptor))
                return descriptor.getterPresent() ? JSValue::encode(descriptor.getter()) : JSValue::encode(jsUndefined());
        }
    }

    return JSValue::encode(jsUndefined());
}

EncodedJSValue JSC_HOST_CALL objectProtoFuncLookupSetter(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    JSObject* thisObject = exec->thisValue().toThis(exec, StrictMode).toObject(exec);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());

    auto propertyName = exec->argument(0).toPropertyKey(exec);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());

    PropertySlot slot(thisObject, PropertySlot::InternalMethodType::GetOwnProperty);
    bool hasProperty = thisObject->getPropertySlot(exec, propertyName, slot);
    ASSERT(!scope.exception() || !hasProperty);
    if (hasProperty) {
        if (slot.isAccessor()) {
            GetterSetter* getterSetter = slot.getterSetter();
            return getterSetter->isSetterNull() ? JSValue::encode(jsUndefined()) : JSValue::encode(getterSetter->setter());
        }
        if (slot.attributes() & CustomAccessor) {
            PropertyDescriptor descriptor;
            ASSERT(slot.slotBase());
            if (slot.slotBase()->getOwnPropertyDescriptor(exec, propertyName, descriptor))
                return descriptor.setterPresent() ? JSValue::encode(descriptor.setter()) : JSValue::encode(jsUndefined());
        }
    }

    return JSValue::encode(jsUndefined());
}

EncodedJSValue JSC_HOST_CALL objectProtoFuncPropertyIsEnumerable(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    auto propertyName = exec->argument(0).toPropertyKey(exec);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());

    JSObject* thisObject = exec->thisValue().toThis(exec, StrictMode).toObject(exec);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());
    PropertyDescriptor descriptor;
    bool enumerable = thisObject->getOwnPropertyDescriptor(exec, propertyName, descriptor) && descriptor.enumerable();
    return JSValue::encode(jsBoolean(enumerable));
}

// 15.2.4.3 Object.prototype.toLocaleString()
EncodedJSValue JSC_HOST_CALL objectProtoFuncToLocaleString(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    // 1. Let O be the result of calling ToObject passing the this value as the argument.
    JSObject* object = exec->thisValue().toThis(exec, StrictMode).toObject(exec);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());

    // 2. Let toString be the result of calling the [[Get]] internal method of O passing "toString" as the argument.
    JSValue toString = object->get(exec, vm.propertyNames->toString);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());

    // 3. If IsCallable(toString) is false, throw a TypeError exception.
    CallData callData;
    CallType callType = getCallData(toString, callData);
    if (callType == CallType::None)
        return JSValue::encode(jsUndefined());

    // 4. Return the result of calling the [[Call]] internal method of toString passing O as the this value and no arguments.
    scope.release();
    return JSValue::encode(call(exec, toString, callType, callData, object, exec->emptyList()));
}

EncodedJSValue JSC_HOST_CALL objectProtoFuncToString(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    JSValue thisValue = exec->thisValue().toThis(exec, StrictMode);
    if (thisValue.isUndefinedOrNull())
        return JSValue::encode(thisValue.isUndefined() ? vm.smallStrings.undefinedObjectString() : vm.smallStrings.nullObjectString());
    JSObject* thisObject = thisValue.toObject(exec);
    ASSERT(!!scope.exception() == !thisObject);
    if (!thisObject)
        return JSValue::encode(jsUndefined());

    auto result = thisObject->structure(vm)->objectToStringValue();
    if (result)
        return JSValue::encode(result);

    PropertyName toStringTagSymbol = vm.propertyNames->toStringTagSymbol;
    scope.release();
    return JSValue::encode(thisObject->getPropertySlot(exec, toStringTagSymbol, [&] (bool found, PropertySlot& toStringTagSlot) -> JSValue {
        if (found) {
            JSValue stringTag = toStringTagSlot.getValue(exec, toStringTagSymbol);
            RETURN_IF_EXCEPTION(scope, { });
            if (stringTag.isString()) {
                JSRopeString::RopeBuilder ropeBuilder(vm);
                ropeBuilder.append(vm.smallStrings.objectStringStart());
                ropeBuilder.append(asString(stringTag));
                ropeBuilder.append(vm.smallStrings.singleCharacterString(']'));
                JSString* result = ropeBuilder.release();

                thisObject->structure(vm)->setObjectToStringValue(exec, vm, result, toStringTagSlot);
                return result;
            }
        }

        String tag = thisObject->methodTable(vm)->toStringName(thisObject, exec);
        RETURN_IF_EXCEPTION(scope, { });
        String newString = tryMakeString("[object ", WTFMove(tag), "]");
        if (!newString)
            return throwOutOfMemoryError(exec, scope);

        auto result = jsNontrivialString(&vm, newString);
        thisObject->structure(vm)->setObjectToStringValue(exec, vm, result, toStringTagSlot);
        return result;
    }));
}

} // namespace JSC
