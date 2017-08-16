/*
 * Copyright (C) 2016 Apple Inc. All rights reserved.
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
#include "WebAssemblyRuntimeErrorConstructor.h"

#if ENABLE(WEBASSEMBLY)

#include "FunctionPrototype.h"
#include "JSCInlines.h"
#include "JSWebAssemblyRuntimeError.h"
#include "WebAssemblyRuntimeErrorPrototype.h"

#include "WebAssemblyRuntimeErrorConstructor.lut.h"

namespace JSC {

const ClassInfo WebAssemblyRuntimeErrorConstructor::s_info = { "Function", &Base::s_info, &constructorTableWebAssemblyRuntimeError, CREATE_METHOD_TABLE(WebAssemblyRuntimeErrorConstructor) };

/* Source for WebAssemblyRuntimeErrorConstructor.lut.h
 @begin constructorTableWebAssemblyRuntimeError
 @end
 */

static EncodedJSValue JSC_HOST_CALL constructJSWebAssemblyRuntimeError(ExecState* state)
{
    auto& vm = state->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);
    JSValue message = state->argument(0);
    auto* structure = InternalFunction::createSubclassStructure(state, state->newTarget(), asInternalFunction(state->jsCallee())->globalObject()->WebAssemblyRuntimeErrorStructure());
    RETURN_IF_EXCEPTION(scope, encodedJSValue());
    return JSValue::encode(JSWebAssemblyRuntimeError::create(state, vm, structure, message));
}

static EncodedJSValue JSC_HOST_CALL callJSWebAssemblyRuntimeError(ExecState* state)
{
    VM& vm = state->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);
    return JSValue::encode(throwConstructorCannotBeCalledAsFunctionTypeError(state, scope, "WebAssembly.RuntimeError"));
}

WebAssemblyRuntimeErrorConstructor* WebAssemblyRuntimeErrorConstructor::create(VM& vm, Structure* structure, WebAssemblyRuntimeErrorPrototype* thisPrototype)
{
    auto* constructor = new (NotNull, allocateCell<WebAssemblyRuntimeErrorConstructor>(vm.heap)) WebAssemblyRuntimeErrorConstructor(vm, structure);
    constructor->finishCreation(vm, thisPrototype);
    return constructor;
}

Structure* WebAssemblyRuntimeErrorConstructor::createStructure(VM& vm, JSGlobalObject* globalObject, JSValue prototype)
{
    return Structure::create(vm, globalObject, prototype, TypeInfo(ObjectType, StructureFlags), info());
}

void WebAssemblyRuntimeErrorConstructor::finishCreation(VM& vm, WebAssemblyRuntimeErrorPrototype* prototype)
{
    Base::finishCreation(vm, ASCIILiteral("RuntimeError"));
    putDirectWithoutTransition(vm, vm.propertyNames->prototype, prototype, ReadOnly | DontEnum | DontDelete);
    putDirectWithoutTransition(vm, vm.propertyNames->length, jsNumber(1), ReadOnly | DontEnum | DontDelete);
}

WebAssemblyRuntimeErrorConstructor::WebAssemblyRuntimeErrorConstructor(VM& vm, Structure* structure)
    : Base(vm, structure)
{
}

ConstructType WebAssemblyRuntimeErrorConstructor::getConstructData(JSCell*, ConstructData& constructData)
{
    constructData.native.function = constructJSWebAssemblyRuntimeError;
    return ConstructType::Host;
}

CallType WebAssemblyRuntimeErrorConstructor::getCallData(JSCell*, CallData& callData)
{
    callData.native.function = callJSWebAssemblyRuntimeError;
    return CallType::Host;
}

} // namespace JSC

#endif // ENABLE(WEBASSEMBLY)
