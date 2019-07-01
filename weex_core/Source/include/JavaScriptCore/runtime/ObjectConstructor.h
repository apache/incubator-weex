/*
 *  Copyright (C) 1999-2000 Harri Porten (porten@kde.org)
 *  Copyright (C) 2008, 2016-2017 Apple Inc. All rights reserved.
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

#pragma once

#include "InternalFunction.h"
#include "JSGlobalObject.h"
#include "ObjectPrototype.h"

namespace JSC {

EncodedJSValue JSC_HOST_CALL objectConstructorGetOwnPropertyDescriptor(ExecState*);
EncodedJSValue JSC_HOST_CALL objectConstructorGetOwnPropertyDescriptors(ExecState*);
EncodedJSValue JSC_HOST_CALL objectConstructorGetOwnPropertySymbols(ExecState*);
EncodedJSValue JSC_HOST_CALL objectConstructorKeys(ExecState*);
EncodedJSValue JSC_HOST_CALL ownEnumerablePropertyKeys(ExecState*);

class ObjectPrototype;

class ObjectConstructor : public InternalFunction {
public:
    typedef InternalFunction Base;
    static const unsigned StructureFlags = Base::StructureFlags | HasStaticPropertyTable;

    static ObjectConstructor* create(VM& vm, JSGlobalObject* globalObject, Structure* structure, ObjectPrototype* objectPrototype)
    {
        ObjectConstructor* constructor = new (NotNull, allocateCell<ObjectConstructor>(vm.heap)) ObjectConstructor(vm, structure);
        constructor->finishCreation(vm, globalObject, objectPrototype);
        return constructor;
    }

    DECLARE_INFO;

    static Structure* createStructure(VM& vm, JSGlobalObject* globalObject, JSValue prototype)
    {
        return Structure::create(vm, globalObject, prototype, TypeInfo(ObjectType, StructureFlags), info());
    }

protected:
    void finishCreation(VM&, JSGlobalObject*, ObjectPrototype*);

private:
    ObjectConstructor(VM&, Structure*);
    static ConstructType getConstructData(JSCell*, ConstructData&);
    static CallType getCallData(JSCell*, CallData&);
};

inline JSObject* constructEmptyObject(ExecState* exec, Structure* structure)
{
    return JSFinalObject::create(exec, structure);
}

inline JSObject* constructEmptyObject(ExecState* exec, JSObject* prototype, unsigned inlineCapacity)
{
    JSGlobalObject* globalObject = exec->lexicalGlobalObject();
    PrototypeMap& prototypeMap = globalObject->vm().prototypeMap;
    Structure* structure = prototypeMap.emptyObjectStructureForPrototype(globalObject, prototype, inlineCapacity);
    return constructEmptyObject(exec, structure);
}

inline JSObject* constructEmptyObject(ExecState* exec, JSObject* prototype)
{
    return constructEmptyObject(exec, prototype, JSFinalObject::defaultInlineCapacity());
}

inline JSObject* constructEmptyObject(ExecState* exec)
{
    return constructEmptyObject(exec, exec->lexicalGlobalObject()->objectStructureForObjectConstructor());
}

inline JSObject* constructObject(ExecState* exec, JSGlobalObject* globalObject, JSValue arg)
{
    if (arg.isUndefinedOrNull())
        return constructEmptyObject(exec, globalObject->objectPrototype());
    return arg.toObject(exec, globalObject);
}

// Section 6.2.4.4 of the ES6 specification.
// https://tc39.github.io/ecma262/#sec-frompropertydescriptor
inline JSObject* constructObjectFromPropertyDescriptor(ExecState* exec, const PropertyDescriptor& descriptor)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);
    JSObject* description = constructEmptyObject(exec);
    RETURN_IF_EXCEPTION(scope, nullptr);

    if (!descriptor.isAccessorDescriptor()) {
        description->putDirect(vm, vm.propertyNames->value, descriptor.value() ? descriptor.value() : jsUndefined(), 0);
        description->putDirect(vm, vm.propertyNames->writable, jsBoolean(descriptor.writable()), 0);
    } else {
        ASSERT(descriptor.getter() || descriptor.setter());
        if (descriptor.getter())
            description->putDirect(vm, vm.propertyNames->get, descriptor.getter(), 0);
        if (descriptor.setter())
            description->putDirect(vm, vm.propertyNames->set, descriptor.setter(), 0);
    }
    
    description->putDirect(vm, vm.propertyNames->enumerable, jsBoolean(descriptor.enumerable()), 0);
    description->putDirect(vm, vm.propertyNames->configurable, jsBoolean(descriptor.configurable()), 0);

    return description;
}


JS_EXPORT_PRIVATE JSObject* objectConstructorFreeze(ExecState*, JSObject*);
JSValue objectConstructorGetOwnPropertyDescriptor(ExecState*, JSObject*, const Identifier&);
JSValue objectConstructorGetOwnPropertyDescriptors(ExecState*, JSObject*);
JSArray* ownPropertyKeys(ExecState*, JSObject*, PropertyNameMode, DontEnumPropertiesMode);
bool toPropertyDescriptor(ExecState*, JSValue, PropertyDescriptor&);

} // namespace JSC
