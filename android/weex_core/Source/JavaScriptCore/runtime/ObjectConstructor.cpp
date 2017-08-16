/*
 *  Copyright (C) 1999-2000 Harri Porten (porten@kde.org)
 *  Copyright (C) 2008, 2016 Apple Inc. All rights reserved.
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
#include "ObjectConstructor.h"

#include "BuiltinNames.h"
#include "ButterflyInlines.h"
#include "Error.h"
#include "ExceptionHelpers.h"
#include "JSArray.h"
#include "JSCInlines.h"
#include "JSFunction.h"
#include "JSGlobalObject.h"
#include "JSGlobalObjectFunctions.h"
#include "Lookup.h"
#include "ObjectPrototype.h"
#include "PropertyDescriptor.h"
#include "PropertyNameArray.h"
#include "StackVisitor.h"
#include "Symbol.h"

namespace JSC {

EncodedJSValue JSC_HOST_CALL objectConstructorGetPrototypeOf(ExecState*);
EncodedJSValue JSC_HOST_CALL objectConstructorSetPrototypeOf(ExecState*);
EncodedJSValue JSC_HOST_CALL objectConstructorGetOwnPropertyNames(ExecState*);
EncodedJSValue JSC_HOST_CALL objectConstructorDefineProperty(ExecState*);
EncodedJSValue JSC_HOST_CALL objectConstructorDefineProperties(ExecState*);
EncodedJSValue JSC_HOST_CALL objectConstructorCreate(ExecState*);
EncodedJSValue JSC_HOST_CALL objectConstructorSeal(ExecState*);
EncodedJSValue JSC_HOST_CALL objectConstructorFreeze(ExecState*);
EncodedJSValue JSC_HOST_CALL objectConstructorPreventExtensions(ExecState*);
EncodedJSValue JSC_HOST_CALL objectConstructorIsSealed(ExecState*);
EncodedJSValue JSC_HOST_CALL objectConstructorIsFrozen(ExecState*);
EncodedJSValue JSC_HOST_CALL objectConstructorIsExtensible(ExecState*);
EncodedJSValue JSC_HOST_CALL objectConstructorIs(ExecState*);

}

#include "ObjectConstructor.lut.h"

namespace JSC {

STATIC_ASSERT_IS_TRIVIALLY_DESTRUCTIBLE(ObjectConstructor);

const ClassInfo ObjectConstructor::s_info = { "Function", &InternalFunction::s_info, &objectConstructorTable, CREATE_METHOD_TABLE(ObjectConstructor) };

/* Source for ObjectConstructor.lut.h
@begin objectConstructorTable
  getPrototypeOf            objectConstructorGetPrototypeOf             DontEnum|Function 1
  setPrototypeOf            objectConstructorSetPrototypeOf             DontEnum|Function 2
  getOwnPropertyDescriptor  objectConstructorGetOwnPropertyDescriptor   DontEnum|Function 2
  getOwnPropertyDescriptors objectConstructorGetOwnPropertyDescriptors  DontEnum|Function 1
  getOwnPropertyNames       objectConstructorGetOwnPropertyNames        DontEnum|Function 1
  getOwnPropertySymbols     objectConstructorGetOwnPropertySymbols      DontEnum|Function 1
  keys                      objectConstructorKeys                       DontEnum|Function 1
  defineProperty            objectConstructorDefineProperty             DontEnum|Function 3
  defineProperties          objectConstructorDefineProperties           DontEnum|Function 2
  create                    objectConstructorCreate                     DontEnum|Function 2
  seal                      objectConstructorSeal                       DontEnum|Function 1
  freeze                    objectConstructorFreeze                     DontEnum|Function 1
  preventExtensions         objectConstructorPreventExtensions          DontEnum|Function 1
  isSealed                  objectConstructorIsSealed                   DontEnum|Function 1
  isFrozen                  objectConstructorIsFrozen                   DontEnum|Function 1
  isExtensible              objectConstructorIsExtensible               DontEnum|Function 1
  is                        objectConstructorIs                         DontEnum|Function 2
  assign                    JSBuiltin                                   DontEnum|Function 2
  values                    JSBuiltin                                   DontEnum|Function 1
  entries                   JSBuiltin                                   DontEnum|Function 1
@end
*/

ObjectConstructor::ObjectConstructor(VM& vm, Structure* structure)
    : InternalFunction(vm, structure)
{
}

void ObjectConstructor::finishCreation(VM& vm, JSGlobalObject* globalObject, ObjectPrototype* objectPrototype)
{
    Base::finishCreation(vm, objectPrototype->classInfo(vm)->className);
    // ECMA 15.2.3.1
    putDirectWithoutTransition(vm, vm.propertyNames->prototype, objectPrototype, DontEnum | DontDelete | ReadOnly);
    // no. of arguments for constructor
    putDirectWithoutTransition(vm, vm.propertyNames->length, jsNumber(1), ReadOnly | DontEnum | DontDelete);

    JSC_NATIVE_FUNCTION_WITHOUT_TRANSITION(vm.propertyNames->builtinNames().createPrivateName(), objectConstructorCreate, DontEnum, 2);
    JSC_NATIVE_FUNCTION_WITHOUT_TRANSITION(vm.propertyNames->builtinNames().definePropertyPrivateName(), objectConstructorDefineProperty, DontEnum, 3);
    JSC_NATIVE_FUNCTION_WITHOUT_TRANSITION(vm.propertyNames->builtinNames().getPrototypeOfPrivateName(), objectConstructorGetPrototypeOf, DontEnum, 1);
    JSC_NATIVE_FUNCTION_WITHOUT_TRANSITION(vm.propertyNames->builtinNames().getOwnPropertyNamesPrivateName(), objectConstructorGetOwnPropertyNames, DontEnum, 1);
    JSC_NATIVE_FUNCTION_WITHOUT_TRANSITION(vm.propertyNames->builtinNames().getOwnPropertyDescriptorPrivateName(), objectConstructorGetOwnPropertyDescriptor, DontEnum, 1);
}

// ES 19.1.1.1 Object([value])
static ALWAYS_INLINE JSObject* constructObject(ExecState* exec, JSValue newTarget)
{
    ObjectConstructor* objectConstructor = jsCast<ObjectConstructor*>(exec->jsCallee());
    JSGlobalObject* globalObject = objectConstructor->globalObject();
    VM& vm = globalObject->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    // We need to check newTarget condition in this caller side instead of InternalFunction::createSubclassStructure side.
    // Since if we found this condition is met, we should not fall into the type conversion in the step 3.

    // 1. If NewTarget is neither undefined nor the active function, then
    if (newTarget && newTarget != objectConstructor) {
        // a. Return ? OrdinaryCreateFromConstructor(NewTarget, "%ObjectPrototype%").
        Structure* objectStructure = InternalFunction::createSubclassStructure(exec, newTarget, globalObject->objectStructureForObjectConstructor());
        RETURN_IF_EXCEPTION(scope, nullptr);
        return constructEmptyObject(exec, objectStructure);
    }

    // 2. If value is null, undefined or not supplied, return ObjectCreate(%ObjectPrototype%).
    ArgList args(exec);
    JSValue arg = args.at(0);
    if (arg.isUndefinedOrNull())
        return constructEmptyObject(exec, globalObject->objectStructureForObjectConstructor());

    // 3. Return ToObject(value).
    scope.release();
    return arg.toObject(exec, globalObject);
}

static EncodedJSValue JSC_HOST_CALL constructWithObjectConstructor(ExecState* exec)
{
    return JSValue::encode(constructObject(exec, exec->newTarget()));
}

ConstructType ObjectConstructor::getConstructData(JSCell*, ConstructData& constructData)
{
    constructData.native.function = constructWithObjectConstructor;
    return ConstructType::Host;
}

static EncodedJSValue JSC_HOST_CALL callObjectConstructor(ExecState* exec)
{
    return JSValue::encode(constructObject(exec, JSValue()));
}

CallType ObjectConstructor::getCallData(JSCell*, CallData& callData)
{
    callData.native.function = callObjectConstructor;
    return CallType::Host;
}

EncodedJSValue JSC_HOST_CALL objectConstructorGetPrototypeOf(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);
    JSObject* object = exec->argument(0).toObject(exec);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());
    scope.release();
    return JSValue::encode(object->getPrototype(vm, exec));
}

EncodedJSValue JSC_HOST_CALL objectConstructorSetPrototypeOf(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    JSValue objectValue = exec->argument(0);
    if (objectValue.isUndefinedOrNull())
        return throwVMTypeError(exec, scope, ASCIILiteral("Cannot set prototype of undefined or null"));

    JSValue protoValue = exec->argument(1);
    if (!protoValue.isObject() && !protoValue.isNull())
        return throwVMTypeError(exec, scope, ASCIILiteral("Prototype value can only be an object or null"));

    JSObject* object = objectValue.toObject(exec);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());

    bool shouldThrowIfCantSet = true;
    bool didSetPrototype = object->setPrototype(vm, exec, protoValue, shouldThrowIfCantSet);
    ASSERT_UNUSED(didSetPrototype, scope.exception() || didSetPrototype);
    return JSValue::encode(objectValue);
}

JSValue objectConstructorGetOwnPropertyDescriptor(ExecState* exec, JSObject* object, const Identifier& propertyName)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);
    PropertyDescriptor descriptor;
    if (!object->getOwnPropertyDescriptor(exec, propertyName, descriptor)) {
        scope.release();
        return jsUndefined();
    }
    RETURN_IF_EXCEPTION(scope, { });

    JSObject* result = constructObjectFromPropertyDescriptor(exec, descriptor);
    ASSERT(!!scope.exception() == !result);
    if (!result)
        return jsUndefined();
    return result;
}

JSValue objectConstructorGetOwnPropertyDescriptors(ExecState* exec, JSObject* object)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);
    PropertyNameArray properties(exec, PropertyNameMode::StringsAndSymbols);
    object->methodTable(vm)->getOwnPropertyNames(object, exec, properties, EnumerationMode(DontEnumPropertiesMode::Include));
    RETURN_IF_EXCEPTION(scope, { });

    JSObject* descriptors = constructEmptyObject(exec);
    RETURN_IF_EXCEPTION(scope, { });

    for (auto& propertyName : properties) {
        PropertyDescriptor descriptor;
        bool didGetDescriptor = object->getOwnPropertyDescriptor(exec, propertyName, descriptor);
        RETURN_IF_EXCEPTION(scope, { });

        if (!didGetDescriptor)
            continue;

        JSObject* fromDescriptor = constructObjectFromPropertyDescriptor(exec, descriptor);
        ASSERT(!!scope.exception() == !fromDescriptor);
        if (!fromDescriptor)
            return jsUndefined();

        PutPropertySlot slot(descriptors);
        descriptors->putOwnDataPropertyMayBeIndex(exec, propertyName, fromDescriptor, slot);
        ASSERT(!scope.exception());
    }

    return descriptors;
}

EncodedJSValue JSC_HOST_CALL objectConstructorGetOwnPropertyDescriptor(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);
    JSObject* object = exec->argument(0).toObject(exec);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());
    auto propertyName = exec->argument(1).toPropertyKey(exec);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());
    scope.release();
    return JSValue::encode(objectConstructorGetOwnPropertyDescriptor(exec, object, propertyName));
}

EncodedJSValue JSC_HOST_CALL objectConstructorGetOwnPropertyDescriptors(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);
    JSObject* object = exec->argument(0).toObject(exec);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());
    scope.release();
    return JSValue::encode(objectConstructorGetOwnPropertyDescriptors(exec, object));
}

// FIXME: Use the enumeration cache.
EncodedJSValue JSC_HOST_CALL objectConstructorGetOwnPropertyNames(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);
    JSObject* object = exec->argument(0).toObject(exec);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());
    scope.release();
    return JSValue::encode(ownPropertyKeys(exec, object, PropertyNameMode::Strings, DontEnumPropertiesMode::Include));
}

// FIXME: Use the enumeration cache.
EncodedJSValue JSC_HOST_CALL objectConstructorGetOwnPropertySymbols(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);
    JSObject* object = exec->argument(0).toObject(exec);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());
    scope.release();
    return JSValue::encode(ownPropertyKeys(exec, object, PropertyNameMode::Symbols, DontEnumPropertiesMode::Include));
}

// FIXME: Use the enumeration cache.
EncodedJSValue JSC_HOST_CALL objectConstructorKeys(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);
    JSObject* object = exec->argument(0).toObject(exec);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());
    scope.release();
    return JSValue::encode(ownPropertyKeys(exec, object, PropertyNameMode::Strings, DontEnumPropertiesMode::Exclude));
}

EncodedJSValue JSC_HOST_CALL ownEnumerablePropertyKeys(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);
    JSObject* object = exec->argument(0).toObject(exec);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());
    scope.release();
    return JSValue::encode(ownPropertyKeys(exec, object, PropertyNameMode::StringsAndSymbols, DontEnumPropertiesMode::Exclude));
}

// ES6 6.2.4.5 ToPropertyDescriptor
// https://tc39.github.io/ecma262/#sec-topropertydescriptor
bool toPropertyDescriptor(ExecState* exec, JSValue in, PropertyDescriptor& desc)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    if (!in.isObject()) {
        throwTypeError(exec, scope, ASCIILiteral("Property description must be an object."));
        return false;
    }
    JSObject* description = asObject(in);

    bool hasProperty = description->hasProperty(exec, vm.propertyNames->enumerable);
    ASSERT(!scope.exception() || !hasProperty);
    if (hasProperty) {
        JSValue value = description->get(exec, vm.propertyNames->enumerable);
        RETURN_IF_EXCEPTION(scope, false);
        desc.setEnumerable(value.toBoolean(exec));
    } else
        RETURN_IF_EXCEPTION(scope, false);

    hasProperty = description->hasProperty(exec, vm.propertyNames->configurable);
    ASSERT(!scope.exception() || !hasProperty);
    if (hasProperty) {
        JSValue value = description->get(exec, vm.propertyNames->configurable);
        RETURN_IF_EXCEPTION(scope, false);
        desc.setConfigurable(value.toBoolean(exec));
    } else
        RETURN_IF_EXCEPTION(scope, false);

    JSValue value;
    hasProperty = description->hasProperty(exec, vm.propertyNames->value);
    ASSERT(!scope.exception() || !hasProperty);
    if (hasProperty) {
        JSValue value = description->get(exec, vm.propertyNames->value);
        RETURN_IF_EXCEPTION(scope, false);
        desc.setValue(value);
    } else
        RETURN_IF_EXCEPTION(scope, false);

    hasProperty = description->hasProperty(exec, vm.propertyNames->writable);
    ASSERT(!scope.exception() || !hasProperty);
    if (hasProperty) {
        JSValue value = description->get(exec, vm.propertyNames->writable);
        RETURN_IF_EXCEPTION(scope, false);
        desc.setWritable(value.toBoolean(exec));
    } else
        RETURN_IF_EXCEPTION(scope, false);

    hasProperty = description->hasProperty(exec, vm.propertyNames->get);
    ASSERT(!scope.exception() || !hasProperty);
    if (hasProperty) {
        JSValue get = description->get(exec, vm.propertyNames->get);
        RETURN_IF_EXCEPTION(scope, false);
        if (!get.isUndefined()) {
            CallData callData;
            if (getCallData(get, callData) == CallType::None) {
                throwTypeError(exec, scope, ASCIILiteral("Getter must be a function."));
                return false;
            }
        }
        desc.setGetter(get);
    } else
        RETURN_IF_EXCEPTION(scope, false);

    hasProperty = description->hasProperty(exec, vm.propertyNames->set);
    ASSERT(!scope.exception() || !hasProperty);
    if (hasProperty) {
        JSValue set = description->get(exec, vm.propertyNames->set);
        RETURN_IF_EXCEPTION(scope, false);
        if (!set.isUndefined()) {
            CallData callData;
            if (getCallData(set, callData) == CallType::None) {
                throwTypeError(exec, scope, ASCIILiteral("Setter must be a function."));
                return false;
            }
        }
        desc.setSetter(set);
    } else
        RETURN_IF_EXCEPTION(scope, false);

    if (!desc.isAccessorDescriptor())
        return true;

    if (desc.value()) {
        throwTypeError(exec, scope, ASCIILiteral("Invalid property.  'value' present on property with getter or setter."));
        return false;
    }

    if (desc.writablePresent()) {
        throwTypeError(exec, scope, ASCIILiteral("Invalid property.  'writable' present on property with getter or setter."));
        return false;
    }
    return true;
}

EncodedJSValue JSC_HOST_CALL objectConstructorDefineProperty(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    if (!exec->argument(0).isObject())
        return throwVMTypeError(exec, scope, ASCIILiteral("Properties can only be defined on Objects."));
    JSObject* obj = asObject(exec->argument(0));
    auto propertyName = exec->argument(1).toPropertyKey(exec);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());
    PropertyDescriptor descriptor;
    auto success = toPropertyDescriptor(exec, exec->argument(2), descriptor);
    ASSERT(!scope.exception() == success);
    if (!success)
        return JSValue::encode(jsNull());
    ASSERT((descriptor.attributes() & Accessor) || (!descriptor.isAccessorDescriptor()));
    ASSERT(!scope.exception());
    obj->methodTable(vm)->defineOwnProperty(obj, exec, propertyName, descriptor, true);
    scope.release();
    return JSValue::encode(obj);
}

static JSValue defineProperties(ExecState* exec, JSObject* object, JSObject* properties)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    PropertyNameArray propertyNames(exec, PropertyNameMode::StringsAndSymbols);
    asObject(properties)->methodTable(vm)->getOwnPropertyNames(asObject(properties), exec, propertyNames, EnumerationMode(DontEnumPropertiesMode::Exclude));
    RETURN_IF_EXCEPTION(scope, { });
    size_t numProperties = propertyNames.size();
    Vector<PropertyDescriptor> descriptors;
    MarkedArgumentBuffer markBuffer;
    for (size_t i = 0; i < numProperties; i++) {
        JSValue prop = properties->get(exec, propertyNames[i]);
        RETURN_IF_EXCEPTION(scope, { });
        PropertyDescriptor descriptor;
        bool success = toPropertyDescriptor(exec, prop, descriptor);
        ASSERT(!scope.exception() || !success);
        if (UNLIKELY(!success))
            return jsNull();
        descriptors.append(descriptor);
        // Ensure we mark all the values that we're accumulating
        if (descriptor.isDataDescriptor() && descriptor.value())
            markBuffer.append(descriptor.value());
        if (descriptor.isAccessorDescriptor()) {
            if (descriptor.getter())
                markBuffer.append(descriptor.getter());
            if (descriptor.setter())
                markBuffer.append(descriptor.setter());
        }
    }
    for (size_t i = 0; i < numProperties; i++) {
        Identifier propertyName = propertyNames[i];
        if (vm.propertyNames->isPrivateName(propertyName))
            continue;
        object->methodTable(vm)->defineOwnProperty(object, exec, propertyName, descriptors[i], true);
        RETURN_IF_EXCEPTION(scope, { });
    }
    return object;
}

EncodedJSValue JSC_HOST_CALL objectConstructorDefineProperties(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    if (!exec->argument(0).isObject())
        return throwVMTypeError(exec, scope, ASCIILiteral("Properties can only be defined on Objects."));
    JSObject* targetObj = asObject(exec->argument(0));
    JSObject* props = exec->argument(1).toObject(exec);
    ASSERT(!!scope.exception() == !props);
    if (UNLIKELY(!props))
        return encodedJSValue();
    scope.release();
    return JSValue::encode(defineProperties(exec, targetObj, props));
}

EncodedJSValue JSC_HOST_CALL objectConstructorCreate(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    JSValue proto = exec->argument(0);
    if (!proto.isObject() && !proto.isNull())
        return throwVMTypeError(exec, scope, ASCIILiteral("Object prototype may only be an Object or null."));
    JSObject* newObject = proto.isObject()
        ? constructEmptyObject(exec, asObject(proto))
        : constructEmptyObject(exec, exec->lexicalGlobalObject()->nullPrototypeObjectStructure());
    if (exec->argument(1).isUndefined())
        return JSValue::encode(newObject);
    if (!exec->argument(1).isObject())
        return throwVMTypeError(exec, scope, ASCIILiteral("Property descriptor list must be an Object."));
    scope.release();
    return JSValue::encode(defineProperties(exec, newObject, asObject(exec->argument(1))));
}

enum class IntegrityLevel {
    Sealed,
    Frozen
};

template<IntegrityLevel level>
bool setIntegrityLevel(ExecState* exec, VM& vm, JSObject* object)
{
    // See https://tc39.github.io/ecma262/#sec-setintegritylevel.
    auto scope = DECLARE_THROW_SCOPE(vm);

    bool success = object->methodTable(vm)->preventExtensions(object, exec);
    RETURN_IF_EXCEPTION(scope, false);
    if (UNLIKELY(!success))
        return false;

    PropertyNameArray properties(exec, PropertyNameMode::StringsAndSymbols);
    object->methodTable(vm)->getOwnPropertyNames(object, exec, properties, EnumerationMode(DontEnumPropertiesMode::Include));
    RETURN_IF_EXCEPTION(scope, false);

    PropertyNameArray::const_iterator end = properties.end();
    for (PropertyNameArray::const_iterator iter = properties.begin(); iter != end; ++iter) {
        Identifier propertyName = *iter;
        if (vm.propertyNames->isPrivateName(propertyName))
            continue;

        PropertyDescriptor desc;
        if (level == IntegrityLevel::Sealed)
            desc.setConfigurable(false);
        else {
            bool hasPropertyDescriptor = object->getOwnPropertyDescriptor(exec, propertyName, desc);
            RETURN_IF_EXCEPTION(scope, false);
            if (!hasPropertyDescriptor)
                continue;

            if (desc.isDataDescriptor())
                desc.setWritable(false);

            desc.setConfigurable(false);
        }

        object->methodTable(vm)->defineOwnProperty(object, exec, propertyName, desc, true);
        RETURN_IF_EXCEPTION(scope, false);
    }
    return true;
}
    
EncodedJSValue JSC_HOST_CALL objectConstructorSeal(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    // 1. If Type(O) is not Object, return O.
    JSValue obj = exec->argument(0);
    if (!obj.isObject())
        return JSValue::encode(obj);
    JSObject* object = asObject(obj);

    if (isJSFinalObject(object)) {
        object->seal(vm);
        return JSValue::encode(obj);
    }

    bool success = setIntegrityLevel<IntegrityLevel::Sealed>(exec, vm, object);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());
    if (UNLIKELY(!success)) {
        throwTypeError(exec, scope, ASCIILiteral("Unable to prevent extension in Object.seal"));
        return encodedJSValue();
    }

    return JSValue::encode(obj);
}

JSObject* objectConstructorFreeze(ExecState* exec, JSObject* object)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    if (isJSFinalObject(object) && !hasIndexedProperties(object->indexingType())) {
        object->freeze(vm);
        return object;
    }

    bool success = setIntegrityLevel<IntegrityLevel::Frozen>(exec, vm, object);
    RETURN_IF_EXCEPTION(scope, nullptr);
    if (!success)
        return throwTypeError(exec, scope, ASCIILiteral("Unable to prevent extension in Object.freeze"));
    return object;
}

EncodedJSValue JSC_HOST_CALL objectConstructorFreeze(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);
    // 1. If Type(O) is not Object, return O.
    JSValue obj = exec->argument(0);
    if (!obj.isObject())
        return JSValue::encode(obj);
    JSObject* result = objectConstructorFreeze(exec, asObject(obj));
    RETURN_IF_EXCEPTION(scope, encodedJSValue());
    return JSValue::encode(result);
}

EncodedJSValue JSC_HOST_CALL objectConstructorPreventExtensions(ExecState* exec)
{
    VM& vm = exec->vm();
    JSValue argument = exec->argument(0);
    if (!argument.isObject())
        return JSValue::encode(argument);
    JSObject* object = asObject(argument);
    object->methodTable(vm)->preventExtensions(object, exec);
    return JSValue::encode(object);
}

EncodedJSValue JSC_HOST_CALL objectConstructorIsSealed(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    // 1. If Type(O) is not Object, return true.
    JSValue obj = exec->argument(0);
    if (!obj.isObject())
        return JSValue::encode(jsBoolean(true));
    JSObject* object = asObject(obj);

    if (isJSFinalObject(object))
        return JSValue::encode(jsBoolean(object->isSealed(vm)));

    // 2. For each named own property name P of O,
    PropertyNameArray properties(exec, PropertyNameMode::StringsAndSymbols);
    object->methodTable(vm)->getOwnPropertyNames(object, exec, properties, EnumerationMode(DontEnumPropertiesMode::Include));
    RETURN_IF_EXCEPTION(scope, { });
    PropertyNameArray::const_iterator end = properties.end();
    for (PropertyNameArray::const_iterator iter = properties.begin(); iter != end; ++iter) {
        Identifier propertyName = *iter;
        if (vm.propertyNames->isPrivateName(propertyName))
            continue;
        // a. Let desc be the result of calling the [[GetOwnProperty]] internal method of O with P.
        PropertyDescriptor desc;
        bool didGetDescriptor = object->getOwnPropertyDescriptor(exec, propertyName, desc);
        RETURN_IF_EXCEPTION(scope, { });
        if (!didGetDescriptor)
            continue;
        // b. If desc.[[Configurable]] is true, then return false.
        if (desc.configurable())
            return JSValue::encode(jsBoolean(false));
    }

    // 3. If the [[Extensible]] internal property of O is false, then return true.
    // 4. Otherwise, return false.
    bool isExtensible = object->isExtensible(exec);
    RETURN_IF_EXCEPTION(scope, { });
    return JSValue::encode(jsBoolean(!isExtensible));
}

EncodedJSValue JSC_HOST_CALL objectConstructorIsFrozen(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    // 1. If Type(O) is not Object, return true.
    JSValue obj = exec->argument(0);
    if (!obj.isObject())
        return JSValue::encode(jsBoolean(true));
    JSObject* object = asObject(obj);

    if (isJSFinalObject(object))
        return JSValue::encode(jsBoolean(object->isFrozen(vm)));

    // 2. For each named own property name P of O,
    PropertyNameArray properties(exec, PropertyNameMode::StringsAndSymbols);
    object->methodTable(vm)->getOwnPropertyNames(object, exec, properties, EnumerationMode(DontEnumPropertiesMode::Include));
    RETURN_IF_EXCEPTION(scope, { });
    PropertyNameArray::const_iterator end = properties.end();
    for (PropertyNameArray::const_iterator iter = properties.begin(); iter != end; ++iter) {
        Identifier propertyName = *iter;
        if (vm.propertyNames->isPrivateName(propertyName))
            continue;
        // a. Let desc be the result of calling the [[GetOwnProperty]] internal method of O with P.
        PropertyDescriptor desc;
        bool didGetDescriptor = object->getOwnPropertyDescriptor(exec, propertyName, desc);
        RETURN_IF_EXCEPTION(scope, { });
        if (!didGetDescriptor)
            continue;
        // b. If IsDataDescriptor(desc) is true then
        // i. If desc.[[Writable]] is true, return false. c. If desc.[[Configurable]] is true, then return false.
        if ((desc.isDataDescriptor() && desc.writable()) || desc.configurable())
            return JSValue::encode(jsBoolean(false));
    }

    // 3. If the [[Extensible]] internal property of O is false, then return true.
    // 4. Otherwise, return false.
    bool isExtensible = object->isExtensible(exec);
    RETURN_IF_EXCEPTION(scope, { });
    return JSValue::encode(jsBoolean(!isExtensible));
}

EncodedJSValue JSC_HOST_CALL objectConstructorIsExtensible(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);
    JSValue obj = exec->argument(0);
    if (!obj.isObject())
        return JSValue::encode(jsBoolean(false));
    JSObject* object = asObject(obj);
    bool isExtensible = object->isExtensible(exec);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());
    return JSValue::encode(jsBoolean(isExtensible));
}

EncodedJSValue JSC_HOST_CALL objectConstructorIs(ExecState* exec)
{
    return JSValue::encode(jsBoolean(sameValue(exec, exec->argument(0), exec->argument(1))));
}

// FIXME: Use the enumeration cache.
JSArray* ownPropertyKeys(ExecState* exec, JSObject* object, PropertyNameMode propertyNameMode, DontEnumPropertiesMode dontEnumPropertiesMode)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);
    PropertyNameArray properties(exec, propertyNameMode);
    object->methodTable(vm)->getOwnPropertyNames(object, exec, properties, EnumerationMode(dontEnumPropertiesMode));
    RETURN_IF_EXCEPTION(scope, nullptr);

    JSArray* keys = constructEmptyArray(exec, 0);
    RETURN_IF_EXCEPTION(scope, nullptr);

    switch (propertyNameMode) {
    case PropertyNameMode::Strings: {
        size_t numProperties = properties.size();
        for (size_t i = 0; i < numProperties; i++) {
            const auto& identifier = properties[i];
            ASSERT(!identifier.isSymbol());
            keys->push(exec, jsOwnedString(exec, identifier.string()));
            RETURN_IF_EXCEPTION(scope, nullptr);
        }
        break;
    }

    case PropertyNameMode::Symbols: {
        size_t numProperties = properties.size();
        for (size_t i = 0; i < numProperties; i++) {
            const auto& identifier = properties[i];
            ASSERT(identifier.isSymbol());
            if (!vm.propertyNames->isPrivateName(identifier)) {
                keys->push(exec, Symbol::create(vm, static_cast<SymbolImpl&>(*identifier.impl())));
                RETURN_IF_EXCEPTION(scope, nullptr);
            }
        }
        break;
    }

    case PropertyNameMode::StringsAndSymbols: {
        Vector<Identifier, 16> propertySymbols;
        size_t numProperties = properties.size();
        for (size_t i = 0; i < numProperties; i++) {
            const auto& identifier = properties[i];
            if (identifier.isSymbol()) {
                if (!vm.propertyNames->isPrivateName(identifier))
                    propertySymbols.append(identifier);
            } else {
                keys->push(exec, jsOwnedString(exec, identifier.string()));
                RETURN_IF_EXCEPTION(scope, nullptr);
            }
        }

        // To ensure the order defined in the spec (9.1.12), we append symbols at the last elements of keys.
        for (const auto& identifier : propertySymbols) {
            keys->push(exec, Symbol::create(vm, static_cast<SymbolImpl&>(*identifier.impl())));
            RETURN_IF_EXCEPTION(scope, nullptr);
        }

        break;
    }
    }

    return keys;
}

} // namespace JSC
