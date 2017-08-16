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
#include "ErrorPrototype.h"

#include "Error.h"
#include "JSFunction.h"
#include "JSString.h"
#include "JSStringBuilder.h"
#include "ObjectPrototype.h"
#include "JSCInlines.h"
#include "StringRecursionChecker.h"

namespace JSC {

STATIC_ASSERT_IS_TRIVIALLY_DESTRUCTIBLE(ErrorPrototype);

static EncodedJSValue JSC_HOST_CALL errorProtoFuncToString(ExecState*);

}

#include "ErrorPrototype.lut.h"

namespace JSC {

const ClassInfo ErrorPrototype::s_info = { "Object", &Base::s_info, &errorPrototypeTable, CREATE_METHOD_TABLE(ErrorPrototype) };

/* Source for ErrorPrototype.lut.h
@begin errorPrototypeTable
  toString          errorProtoFuncToString         DontEnum|Function 0
@end
*/

ErrorPrototype::ErrorPrototype(VM& vm, Structure* structure)
    : JSNonFinalObject(vm, structure)
{
}

void ErrorPrototype::finishCreation(VM& vm)
{
    Base::finishCreation(vm);
    ASSERT(inherits(vm, info()));
    putDirect(vm, vm.propertyNames->name, jsNontrivialString(&vm, String(ASCIILiteral("Error"))), DontEnum);
    putDirect(vm, vm.propertyNames->message, jsEmptyString(&vm), DontEnum);
}

// ------------------------------ Functions ---------------------------

// ECMA-262 5.1, 15.11.4.4
EncodedJSValue JSC_HOST_CALL errorProtoFuncToString(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    // 1. Let O be the this value.
    JSValue thisValue = exec->thisValue();

    // 2. If Type(O) is not Object, throw a TypeError exception.
    if (!thisValue.isObject())
        return throwVMTypeError(exec, scope);
    JSObject* thisObj = asObject(thisValue);

    // Guard against recursion!
    StringRecursionChecker checker(exec, thisObj);
    ASSERT(!scope.exception() || checker.earlyReturnValue());
    if (JSValue earlyReturnValue = checker.earlyReturnValue())
        return JSValue::encode(earlyReturnValue);

    // 3. Let name be the result of calling the [[Get]] internal method of O with argument "name".
    JSValue name = thisObj->get(exec, exec->propertyNames().name);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());

    // 4. If name is undefined, then let name be "Error"; else let name be ToString(name).
    String nameString;
    if (name.isUndefined())
        nameString = ASCIILiteral("Error");
    else {
        nameString = name.toWTFString(exec);
        RETURN_IF_EXCEPTION(scope, encodedJSValue());
    }

    // 5. Let msg be the result of calling the [[Get]] internal method of O with argument "message".
    JSValue message = thisObj->get(exec, exec->propertyNames().message);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());

    // (sic)
    // 6. If msg is undefined, then let msg be the empty String; else let msg be ToString(msg).
    // 7. If msg is undefined, then let msg be the empty String; else let msg be ToString(msg).
    String messageString;
    if (message.isUndefined())
        messageString = String();
    else {
        messageString = message.toWTFString(exec);
        RETURN_IF_EXCEPTION(scope, encodedJSValue());
    }

    // 8. If name is the empty String, return msg.
    if (!nameString.length())
        return JSValue::encode(message.isString() ? message : jsString(exec, messageString));

    // 9. If msg is the empty String, return name.
    if (!messageString.length())
        return JSValue::encode(name.isString() ? name : jsString(exec, nameString));

    // 10. Return the result of concatenating name, ":", a single space character, and msg.
    scope.release();
    return JSValue::encode(jsMakeNontrivialString(exec, nameString, ": ", messageString));
}

} // namespace JSC
