/*
 *  Copyright (C) 1999-2000 Harri Porten (porten@kde.org)
 *  Copyright (C) 2003, 2007-2008, 2011, 2016 Apple Inc. All rights reserved.
 *  Copyright (C) 2003 Peter Kelly (pmk@post.com)
 *  Copyright (C) 2006 Alexey Proskuryakov (ap@nypop.com)
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
 *  USA
 *
 */

#include "config.h"
#include "ArrayConstructor.h"

#include "ArrayPrototype.h"
#include "ButterflyInlines.h"
#include "Error.h"
#include "ExceptionHelpers.h"
#include "GetterSetter.h"
#include "JSArray.h"
#include "JSFunction.h"
#include "Lookup.h"
#include "ProxyObject.h"
#include "JSCInlines.h"

#include "ArrayConstructor.lut.h"

namespace JSC {

STATIC_ASSERT_IS_TRIVIALLY_DESTRUCTIBLE(ArrayConstructor);

const ClassInfo ArrayConstructor::s_info = { "Function", &InternalFunction::s_info, &arrayConstructorTable, CREATE_METHOD_TABLE(ArrayConstructor) };

/* Source for ArrayConstructor.lut.h
@begin arrayConstructorTable
  of        JSBuiltin                   DontEnum|Function 0
  from      JSBuiltin                   DontEnum|Function 0
@end
*/

ArrayConstructor::ArrayConstructor(VM& vm, Structure* structure)
    : InternalFunction(vm, structure)
{
}

void ArrayConstructor::finishCreation(VM& vm, JSGlobalObject* globalObject, ArrayPrototype* arrayPrototype, GetterSetter* speciesSymbol)
{
    Base::finishCreation(vm, arrayPrototype->classInfo(vm)->className);
    putDirectWithoutTransition(vm, vm.propertyNames->prototype, arrayPrototype, DontEnum | DontDelete | ReadOnly);
    putDirectWithoutTransition(vm, vm.propertyNames->length, jsNumber(1), ReadOnly | DontEnum | DontDelete);
    putDirectNonIndexAccessor(vm, vm.propertyNames->speciesSymbol, speciesSymbol, Accessor | ReadOnly | DontEnum);
    JSC_BUILTIN_FUNCTION_WITHOUT_TRANSITION(vm.propertyNames->isArray, arrayConstructorIsArrayCodeGenerator, DontEnum);
}

// ------------------------------ Functions ---------------------------

JSValue constructArrayWithSizeQuirk(ExecState* exec, ArrayAllocationProfile* profile, JSGlobalObject* globalObject, JSValue length, JSValue newTarget)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);
    if (!length.isNumber()) {
        scope.release();
        return constructArrayNegativeIndexed(exec, profile, globalObject, &length, 1, newTarget);
    }
    
    uint32_t n = length.toUInt32(exec);
    if (n != length.toNumber(exec))
        return throwException(exec, scope, createRangeError(exec, ASCIILiteral("Array size is not a small enough positive integer.")));
    scope.release();
    return constructEmptyArray(exec, profile, globalObject, n, newTarget);
}

static inline JSValue constructArrayWithSizeQuirk(ExecState* exec, const ArgList& args, JSValue newTarget)
{
    JSGlobalObject* globalObject = asInternalFunction(exec->jsCallee())->globalObject();

    // a single numeric argument denotes the array size (!)
    if (args.size() == 1)
        return constructArrayWithSizeQuirk(exec, nullptr, globalObject, args.at(0), newTarget);

    // otherwise the array is constructed with the arguments in it
    return constructArray(exec, nullptr, globalObject, args, newTarget);
}

static EncodedJSValue JSC_HOST_CALL constructWithArrayConstructor(ExecState* exec)
{
    ArgList args(exec);
    return JSValue::encode(constructArrayWithSizeQuirk(exec, args, exec->newTarget()));
}

ConstructType ArrayConstructor::getConstructData(JSCell*, ConstructData& constructData)
{
    constructData.native.function = constructWithArrayConstructor;
    return ConstructType::Host;
}

static EncodedJSValue JSC_HOST_CALL callArrayConstructor(ExecState* exec)
{
    ArgList args(exec);
    return JSValue::encode(constructArrayWithSizeQuirk(exec, args, JSValue()));
}

CallType ArrayConstructor::getCallData(JSCell*, CallData& callData)
{
    // equivalent to 'new Array(....)'
    callData.native.function = callArrayConstructor;
    return CallType::Host;
}

static ALWAYS_INLINE bool isArraySlowInline(ExecState* exec, ProxyObject* proxy)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    while (true) {
        if (proxy->isRevoked()) {
            throwTypeError(exec, scope, ASCIILiteral("Array.isArray cannot be called on a Proxy that has been revoked"));
            return false;
        }
        JSObject* argument = proxy->target();

        if (argument->type() == ArrayType ||  argument->type() == DerivedArrayType)
            return true;

        if (argument->type() != ProxyObjectType)
            return false;

        proxy = jsCast<ProxyObject*>(argument);
    }

    ASSERT_NOT_REACHED();
}

bool isArraySlow(ExecState* exec, ProxyObject* argument)
{
    return isArraySlowInline(exec, argument);
}

// ES6 7.2.2
// https://tc39.github.io/ecma262/#sec-isarray
EncodedJSValue JSC_HOST_CALL arrayConstructorPrivateFuncIsArraySlow(ExecState* exec)
{
    ASSERT(jsDynamicCast<ProxyObject*>(exec->vm(), exec->argument(0)));
    return JSValue::encode(jsBoolean(isArraySlowInline(exec, jsCast<ProxyObject*>(exec->uncheckedArgument(0)))));
}

EncodedJSValue JSC_HOST_CALL arrayConstructorPrivateFuncIsArrayConstructor(ExecState* exec)
{
    VM& vm = exec->vm();
    return JSValue::encode(jsBoolean(jsDynamicCast<ArrayConstructor*>(vm, exec->uncheckedArgument(0))));
}

} // namespace JSC
