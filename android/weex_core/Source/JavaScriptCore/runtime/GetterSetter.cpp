/*
 *  Copyright (C) 1999-2002 Harri Porten (porten@kde.org)
 *  Copyright (C) 2001 Peter Kelly (pmk@post.com)
 *  Copyright (C) 2004, 2007-2009, 2014, 2016 Apple Inc. All rights reserved.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 *
 */

#include "config.h"
#include "GetterSetter.h"

#include "Error.h"
#include "Exception.h"
#include "JSObject.h"
#include "JSCInlines.h"
#include <wtf/Assertions.h>

namespace JSC {

STATIC_ASSERT_IS_TRIVIALLY_DESTRUCTIBLE(GetterSetter);

const ClassInfo GetterSetter::s_info = { "GetterSetter", &Base::s_info, 0, CREATE_METHOD_TABLE(GetterSetter) };

void GetterSetter::visitChildren(JSCell* cell, SlotVisitor& visitor)
{
    GetterSetter* thisObject = jsCast<GetterSetter*>(cell);
    ASSERT_GC_OBJECT_INHERITS(thisObject, info());
    JSCell::visitChildren(thisObject, visitor);

    visitor.append(thisObject->m_getter);
    visitor.append(thisObject->m_setter);
}

GetterSetter* GetterSetter::withGetter(VM& vm, JSGlobalObject* globalObject, JSObject* newGetter)
{
    if (isGetterNull()) {
        setGetter(vm, globalObject, newGetter);
        return this;
    }
    
    GetterSetter* result = GetterSetter::create(vm, globalObject);
    result->setGetter(vm, globalObject, newGetter);
    result->setSetter(vm, globalObject, setter());
    return result;
}

GetterSetter* GetterSetter::withSetter(VM& vm, JSGlobalObject* globalObject, JSObject* newSetter)
{
    if (isSetterNull()) {
        setSetter(vm, globalObject, newSetter);
        return this;
    }
    
    GetterSetter* result = GetterSetter::create(vm, globalObject);
    result->setGetter(vm, globalObject, getter());
    result->setSetter(vm, globalObject, newSetter);
    return result;
}

JSValue callGetter(ExecState* exec, JSValue base, JSValue getterSetter)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);
    // FIXME: Some callers may invoke get() without checking for an exception first.
    // We work around that by checking here.
    RETURN_IF_EXCEPTION(scope, scope.exception()->value());

    JSObject* getter = jsCast<GetterSetter*>(getterSetter)->getter();

    CallData callData;
    CallType callType = getter->methodTable(vm)->getCallData(getter, callData);
    scope.release();
    return call(exec, getter, callType, callData, base, ArgList());
}

bool callSetter(ExecState* exec, JSValue base, JSValue getterSetter, JSValue value, ECMAMode ecmaMode)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    GetterSetter* getterSetterObj = jsCast<GetterSetter*>(getterSetter);

    if (getterSetterObj->isSetterNull())
        return typeError(exec, scope, ecmaMode == StrictMode, ASCIILiteral(ReadonlyPropertyWriteError));

    JSObject* setter = getterSetterObj->setter();

    MarkedArgumentBuffer args;
    args.append(value);

    CallData callData;
    CallType callType = setter->methodTable(vm)->getCallData(setter, callData);
    scope.release();
    call(exec, setter, callType, callData, base, args);
    return true;
}

} // namespace JSC
