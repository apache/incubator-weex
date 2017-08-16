/*
 *  Copyright (C) 1999-2001 Harri Porten (porten@kde.org)
 *  Copyright (C) 2004-2008, 2016 Apple Inc. All rights reserved.
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
#include "StringObject.h"

#include "Error.h"
#include "JSGlobalObject.h"
#include "JSCInlines.h"
#include "PropertyNameArray.h"

namespace JSC {

STATIC_ASSERT_IS_TRIVIALLY_DESTRUCTIBLE(StringObject);

const ClassInfo StringObject::s_info = { "String", &JSWrapperObject::s_info, 0, CREATE_METHOD_TABLE(StringObject) };

StringObject::StringObject(VM& vm, Structure* structure)
    : JSWrapperObject(vm, structure)
{
}

void StringObject::finishCreation(VM& vm, JSString* string)
{
    Base::finishCreation(vm);
    ASSERT(inherits(vm, info()));
    setInternalValue(vm, string);
}

bool StringObject::getOwnPropertySlot(JSObject* cell, ExecState* exec, PropertyName propertyName, PropertySlot& slot)
{
    StringObject* thisObject = jsCast<StringObject*>(cell);
    if (thisObject->internalValue()->getStringPropertySlot(exec, propertyName, slot))
        return true;
    return JSObject::getOwnPropertySlot(thisObject, exec, propertyName, slot);
}
    
bool StringObject::getOwnPropertySlotByIndex(JSObject* object, ExecState* exec, unsigned propertyName, PropertySlot& slot)
{
    StringObject* thisObject = jsCast<StringObject*>(object);
    if (thisObject->internalValue()->getStringPropertySlot(exec, propertyName, slot))
        return true;    
    return JSObject::getOwnPropertySlot(thisObject, exec, Identifier::from(exec, propertyName), slot);
}

bool StringObject::put(JSCell* cell, ExecState* exec, PropertyName propertyName, JSValue value, PutPropertySlot& slot)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    StringObject* thisObject = jsCast<StringObject*>(cell);

    if (UNLIKELY(isThisValueAltered(slot, thisObject))) {
        scope.release();
        return ordinarySetSlow(exec, thisObject, propertyName, value, slot.thisValue(), slot.isStrictMode());
    }

    if (propertyName == vm.propertyNames->length)
        return typeError(exec, scope, slot.isStrictMode(), ASCIILiteral(ReadonlyPropertyWriteError));
    if (std::optional<uint32_t> index = parseIndex(propertyName)) {
        scope.release();
        return putByIndex(cell, exec, index.value(), value, slot.isStrictMode());
    }
    scope.release();
    return JSObject::put(cell, exec, propertyName, value, slot);
}

bool StringObject::putByIndex(JSCell* cell, ExecState* exec, unsigned propertyName, JSValue value, bool shouldThrow)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    StringObject* thisObject = jsCast<StringObject*>(cell);
    if (thisObject->internalValue()->canGetIndex(propertyName))
        return typeError(exec, scope, shouldThrow, ASCIILiteral(ReadonlyPropertyWriteError));
    scope.release();
    return JSObject::putByIndex(cell, exec, propertyName, value, shouldThrow);
}

static bool isStringOwnProperty(ExecState* exec, StringObject* object, PropertyName propertyName)
{
    if (propertyName == exec->propertyNames().length)
        return true;
    if (std::optional<uint32_t> index = parseIndex(propertyName)) {
        if (object->internalValue()->canGetIndex(index.value()))
            return true;
    }
    return false;
}

bool StringObject::defineOwnProperty(JSObject* object, ExecState* exec, PropertyName propertyName, const PropertyDescriptor& descriptor, bool throwException)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);
    StringObject* thisObject = jsCast<StringObject*>(object);

    if (isStringOwnProperty(exec, thisObject, propertyName)) {
        // The current PropertyDescriptor is always
        // PropertyDescriptor{[[Value]]: value, [[Writable]]: false, [[Enumerable]]: true, [[Configurable]]: false}.
        // This ensures that any property descriptor cannot change the existing one.
        // Here, simply return the result of validateAndApplyPropertyDescriptor.
        // https://tc39.github.io/ecma262/#sec-string-exotic-objects-getownproperty-p
        PropertyDescriptor current;
        bool isCurrentDefined = thisObject->getOwnPropertyDescriptor(exec, propertyName, current);
        ASSERT(isCurrentDefined);
        bool isExtensible = thisObject->isExtensible(exec);
        RETURN_IF_EXCEPTION(scope, false);
        scope.release();
        return validateAndApplyPropertyDescriptor(exec, nullptr, propertyName, isExtensible, descriptor, isCurrentDefined, current, throwException);
    }

    scope.release();
    return Base::defineOwnProperty(object, exec, propertyName, descriptor, throwException);
}

bool StringObject::deleteProperty(JSCell* cell, ExecState* exec, PropertyName propertyName)
{
    StringObject* thisObject = jsCast<StringObject*>(cell);
    if (propertyName == exec->propertyNames().length)
        return false;
    std::optional<uint32_t> index = parseIndex(propertyName);
    if (index && thisObject->internalValue()->canGetIndex(index.value()))
        return false;
    return JSObject::deleteProperty(thisObject, exec, propertyName);
}

bool StringObject::deletePropertyByIndex(JSCell* cell, ExecState* exec, unsigned i)
{
    StringObject* thisObject = jsCast<StringObject*>(cell);
    if (thisObject->internalValue()->canGetIndex(i))
        return false;
    return JSObject::deletePropertyByIndex(thisObject, exec, i);
}

void StringObject::getOwnPropertyNames(JSObject* object, ExecState* exec, PropertyNameArray& propertyNames, EnumerationMode mode)
{
    StringObject* thisObject = jsCast<StringObject*>(object);
    if (propertyNames.includeStringProperties()) {
        int size = thisObject->internalValue()->length();
        for (int i = 0; i < size; ++i)
            propertyNames.add(Identifier::from(exec, i));
    }
    if (mode.includeDontEnumProperties())
        propertyNames.add(exec->propertyNames().length);
    return JSObject::getOwnPropertyNames(thisObject, exec, propertyNames, mode);
}

StringObject* constructString(VM& vm, JSGlobalObject* globalObject, JSValue string)
{
    StringObject* object = StringObject::create(vm, globalObject->stringObjectStructure());
    object->setInternalValue(vm, string);
    return object;
}

} // namespace JSC
