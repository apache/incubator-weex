/*
 *  Copyright (C) 1999-2000 Harri Porten (porten@kde.org)
 *  Copyright (C) 2003, 2008, 2016 Apple Inc. All rights reserved.
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
#include "BooleanConstructor.h"

#include "BooleanPrototype.h"
#include "JSGlobalObject.h"
#include "JSCInlines.h"

namespace JSC {

STATIC_ASSERT_IS_TRIVIALLY_DESTRUCTIBLE(BooleanConstructor);

const ClassInfo BooleanConstructor::s_info = { "Function", &Base::s_info, 0, CREATE_METHOD_TABLE(BooleanConstructor) };

BooleanConstructor::BooleanConstructor(VM& vm, Structure* structure)
    : InternalFunction(vm, structure)
{
}

void BooleanConstructor::finishCreation(VM& vm, BooleanPrototype* booleanPrototype)
{
    Base::finishCreation(vm, booleanPrototype->classInfo()->className);
    putDirectWithoutTransition(vm, vm.propertyNames->prototype, booleanPrototype, DontEnum | DontDelete | ReadOnly);

    // no. of arguments for constructor
    putDirectWithoutTransition(vm, vm.propertyNames->length, jsNumber(1), ReadOnly | DontDelete | DontEnum);
}

// ECMA 15.6.2
static EncodedJSValue JSC_HOST_CALL constructWithBooleanConstructor(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);
    JSValue boolean = jsBoolean(exec->argument(0).toBoolean(exec));
    Structure* booleanStructure = InternalFunction::createSubclassStructure(exec, exec->newTarget(), asInternalFunction(exec->jsCallee())->globalObject()->booleanObjectStructure());
    RETURN_IF_EXCEPTION(scope, encodedJSValue());
    BooleanObject* obj = BooleanObject::create(vm, booleanStructure);
    obj->setInternalValue(vm, boolean);
    return JSValue::encode(obj);
}

ConstructType BooleanConstructor::getConstructData(JSCell*, ConstructData& constructData)
{
    constructData.native.function = constructWithBooleanConstructor;
    return ConstructType::Host;
}

// ECMA 15.6.1
static EncodedJSValue JSC_HOST_CALL callBooleanConstructor(ExecState* exec)
{
    return JSValue::encode(jsBoolean(exec->argument(0).toBoolean(exec)));
}

CallType BooleanConstructor::getCallData(JSCell*, CallData& callData)
{
    callData.native.function = callBooleanConstructor;
    return CallType::Host;
}

JSObject* constructBooleanFromImmediateBoolean(ExecState* exec, JSGlobalObject* globalObject, JSValue immediateBooleanValue)
{
    BooleanObject* obj = BooleanObject::create(exec->vm(), globalObject->booleanObjectStructure());
    obj->setInternalValue(exec->vm(), immediateBooleanValue);
    return obj;
}

} // namespace JSC
