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
#include "StringConstructor.h"

#include "Error.h"
#include "JITCode.h"
#include "JSFunction.h"
#include "JSGlobalObject.h"
#include "JSCInlines.h"
#include "StringPrototype.h"
#include <wtf/text/StringBuilder.h>

namespace JSC {

static EncodedJSValue JSC_HOST_CALL stringFromCharCode(ExecState*);
static EncodedJSValue JSC_HOST_CALL stringFromCodePoint(ExecState*);

}

#include "StringConstructor.lut.h"

namespace JSC {

const ClassInfo StringConstructor::s_info = { "Function", &InternalFunction::s_info, &stringConstructorTable, CREATE_METHOD_TABLE(StringConstructor) };

/* Source for StringConstructor.lut.h
@begin stringConstructorTable
  fromCharCode          stringFromCharCode         DontEnum|Function 1 FromCharCodeIntrinsic
  fromCodePoint         stringFromCodePoint        DontEnum|Function 1
  raw                   JSBuiltin                  DontEnum|Function 1
@end
*/

STATIC_ASSERT_IS_TRIVIALLY_DESTRUCTIBLE(StringConstructor);

StringConstructor::StringConstructor(VM& vm, Structure* structure)
    : InternalFunction(vm, structure)
{
}

void StringConstructor::finishCreation(VM& vm, StringPrototype* stringPrototype)
{
    Base::finishCreation(vm, stringPrototype->classInfo()->className);
    putDirectWithoutTransition(vm, vm.propertyNames->prototype, stringPrototype, ReadOnly | DontEnum | DontDelete);
    putDirectWithoutTransition(vm, vm.propertyNames->length, jsNumber(1), ReadOnly | DontEnum | DontDelete);
}

// ------------------------------ Functions --------------------------------

static NEVER_INLINE JSValue stringFromCharCodeSlowCase(ExecState* exec)
{
    unsigned length = exec->argumentCount();
    UChar* buf;
    auto impl = StringImpl::createUninitialized(length, buf);
    for (unsigned i = 0; i < length; ++i)
        buf[i] = static_cast<UChar>(exec->uncheckedArgument(i).toUInt32(exec));
    return jsString(exec, WTFMove(impl));
}

static EncodedJSValue JSC_HOST_CALL stringFromCharCode(ExecState* exec)
{
    if (LIKELY(exec->argumentCount() == 1))
        return JSValue::encode(jsSingleCharacterString(exec, exec->uncheckedArgument(0).toUInt32(exec)));
    return JSValue::encode(stringFromCharCodeSlowCase(exec));
}

JSCell* JSC_HOST_CALL stringFromCharCode(ExecState* exec, int32_t arg)
{
    return jsSingleCharacterString(exec, arg);
}

static EncodedJSValue JSC_HOST_CALL stringFromCodePoint(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    unsigned length = exec->argumentCount();
    StringBuilder builder;
    builder.reserveCapacity(length);

    for (unsigned i = 0; i < length; ++i) {
        double codePointAsDouble = exec->uncheckedArgument(i).toNumber(exec);
        RETURN_IF_EXCEPTION(scope, encodedJSValue());

        uint32_t codePoint = static_cast<uint32_t>(codePointAsDouble);

        if (codePoint != codePointAsDouble || codePoint > UCHAR_MAX_VALUE)
            return throwVMError(exec, scope, createRangeError(exec, ASCIILiteral("Arguments contain a value that is out of range of code points")));

        if (U_IS_BMP(codePoint))
            builder.append(static_cast<UChar>(codePoint));
        else {
            builder.append(U16_LEAD(codePoint));
            builder.append(U16_TRAIL(codePoint));
        }
    }

    scope.release();
    return JSValue::encode(jsString(exec, builder.toString()));
}

static EncodedJSValue JSC_HOST_CALL constructWithStringConstructor(ExecState* exec)
{
    JSGlobalObject* globalObject = asInternalFunction(exec->jsCallee())->globalObject();
    VM& vm = globalObject->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    Structure* structure = InternalFunction::createSubclassStructure(exec, exec->newTarget(), globalObject->stringObjectStructure());
    RETURN_IF_EXCEPTION(scope, encodedJSValue());

    if (!exec->argumentCount())
        return JSValue::encode(StringObject::create(vm, structure));
    JSString* str = exec->uncheckedArgument(0).toString(exec);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());
    return JSValue::encode(StringObject::create(vm, structure, str));
}

ConstructType StringConstructor::getConstructData(JSCell*, ConstructData& constructData)
{
    constructData.native.function = constructWithStringConstructor;
    return ConstructType::Host;
}

JSCell* stringConstructor(ExecState* exec, JSValue argument)
{
    if (argument.isSymbol())
        return jsNontrivialString(exec, asSymbol(argument)->descriptiveString());
    return argument.toString(exec);
}

static EncodedJSValue JSC_HOST_CALL callStringConstructor(ExecState* exec)
{
    if (!exec->argumentCount())
        return JSValue::encode(jsEmptyString(exec));
    return JSValue::encode(stringConstructor(exec, exec->uncheckedArgument(0)));
}

CallType StringConstructor::getCallData(JSCell*, CallData& callData)
{
    callData.native.function = callStringConstructor;
    return CallType::Host;
}

} // namespace JSC
