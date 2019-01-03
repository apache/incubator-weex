/**
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
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
