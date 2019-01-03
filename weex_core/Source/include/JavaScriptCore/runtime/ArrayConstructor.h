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
#include "ProxyObject.h"
#include "ThrowScope.h"

namespace JSC {

class ArrayAllocationProfile;
class ArrayPrototype;
class JSArray;
class GetterSetter;

class ArrayConstructor : public InternalFunction {
public:
    typedef InternalFunction Base;
    static const unsigned StructureFlags = HasStaticPropertyTable | InternalFunction::StructureFlags;

    static ArrayConstructor* create(VM& vm, JSGlobalObject* globalObject, Structure* structure, ArrayPrototype* arrayPrototype, GetterSetter* speciesSymbol)
    {
        ArrayConstructor* constructor = new (NotNull, allocateCell<ArrayConstructor>(vm.heap)) ArrayConstructor(vm, structure);
        constructor->finishCreation(vm, globalObject, arrayPrototype, speciesSymbol);
        return constructor;
    }

    DECLARE_INFO;

    static Structure* createStructure(VM& vm, JSGlobalObject* globalObject, JSValue prototype)
    {
        return Structure::create(vm, globalObject, prototype, TypeInfo(ObjectType, StructureFlags), info());
    }

protected:
    void finishCreation(VM&, JSGlobalObject*, ArrayPrototype*, GetterSetter* speciesSymbol);

private:
    ArrayConstructor(VM&, Structure*);

    static ConstructType getConstructData(JSCell*, ConstructData&);
    static CallType getCallData(JSCell*, CallData&);
};

JSValue constructArrayWithSizeQuirk(ExecState*, ArrayAllocationProfile*, JSGlobalObject*, JSValue length, JSValue prototype = JSValue());

EncodedJSValue JSC_HOST_CALL arrayConstructorPrivateFuncIsArrayConstructor(ExecState*);
EncodedJSValue JSC_HOST_CALL arrayConstructorPrivateFuncIsArraySlow(ExecState*);
bool isArraySlow(ExecState*, ProxyObject* argument);

// ES6 7.2.2
// https://tc39.github.io/ecma262/#sec-isarray
inline bool isArray(ExecState* exec, JSValue argumentValue)
{
    if (!argumentValue.isObject())
        return false;

    JSObject* argument = jsCast<JSObject*>(argumentValue);
    if (argument->type() == ArrayType || argument->type() == DerivedArrayType)
        return true;

    if (argument->type() != ProxyObjectType)
        return false;
    return isArraySlow(exec, jsCast<ProxyObject*>(argument));
}

} // namespace JSC
