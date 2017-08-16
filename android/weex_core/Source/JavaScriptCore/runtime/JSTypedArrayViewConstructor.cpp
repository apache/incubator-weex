/*
 * Copyright (C) 2015-2016 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "JSTypedArrayViewConstructor.h"

#include "CallFrame.h"
#include "Error.h"
#include "GetterSetter.h"
#include "JSCBuiltins.h"
#include "JSCInlines.h"
#include "JSGenericTypedArrayViewConstructorInlines.h"
#include "JSTypedArrayViewPrototype.h"
#include "JSTypedArrays.h"

namespace JSC {

JSTypedArrayViewConstructor::JSTypedArrayViewConstructor(VM& vm, Structure* structure)
    : Base(vm, structure)
{
}

const ClassInfo JSTypedArrayViewConstructor::s_info = { "Function", &Base::s_info, 0, CREATE_METHOD_TABLE(JSTypedArrayViewConstructor) };

void JSTypedArrayViewConstructor::finishCreation(VM& vm, JSGlobalObject* globalObject, JSTypedArrayViewPrototype* prototype, GetterSetter* speciesSymbol)
{
    Base::finishCreation(vm, "TypedArray");
    putDirectWithoutTransition(vm, vm.propertyNames->prototype, prototype, DontEnum | DontDelete | ReadOnly);
    putDirectWithoutTransition(vm, vm.propertyNames->length, jsNumber(0), DontEnum | ReadOnly);
    putDirectNonIndexAccessor(vm, vm.propertyNames->speciesSymbol, speciesSymbol, Accessor | ReadOnly | DontEnum);

    JSC_BUILTIN_FUNCTION_WITHOUT_TRANSITION(vm.propertyNames->of, typedArrayConstructorOfCodeGenerator, DontEnum);
    JSC_BUILTIN_FUNCTION_WITHOUT_TRANSITION(vm.propertyNames->from, typedArrayConstructorFromCodeGenerator, DontEnum);
}

Structure* JSTypedArrayViewConstructor::createStructure(
    VM& vm, JSGlobalObject* globalObject, JSValue prototype)
{
    return Structure::create(vm, globalObject, prototype, TypeInfo(ObjectType, StructureFlags), info());
}



static EncodedJSValue JSC_HOST_CALL constructTypedArrayView(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);
    return throwVMTypeError(exec, scope, ASCIILiteral("%TypedArray% should not be called directly"));
}

ConstructType JSTypedArrayViewConstructor::getConstructData(JSCell*, ConstructData& constructData)
{
    constructData.native.function = constructTypedArrayView;
    return ConstructType::Host;
}

CallType JSTypedArrayViewConstructor::getCallData(JSCell*, CallData& callData)
{
    callData.native.function = constructTypedArrayView;
    return CallType::Host;
}

} // namespace JSC
