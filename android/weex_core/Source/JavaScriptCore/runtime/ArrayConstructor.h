/*
 *  Copyright (C) 1999-2000 Harri Porten (porten@kde.org)
 *  Copyright (C) 2007-2008, 2011, 2016 Apple Inc. All rights reserved.
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
