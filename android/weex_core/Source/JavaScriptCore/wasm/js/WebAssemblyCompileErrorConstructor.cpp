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
#include "WebAssemblyCompileErrorConstructor.h"

#if ENABLE(WEBASSEMBLY)

#include "FunctionPrototype.h"
#include "JSCInlines.h"
#include "JSWebAssemblyCompileError.h"
#include "WebAssemblyCompileErrorPrototype.h"

#include "WebAssemblyCompileErrorConstructor.lut.h"

namespace JSC {

const ClassInfo WebAssemblyCompileErrorConstructor::s_info = { "Function", &Base::s_info, &constructorTableWebAssemblyCompileError, CREATE_METHOD_TABLE(WebAssemblyCompileErrorConstructor) };

/* Source for WebAssemblyCompileErrorConstructor.lut.h
 @begin constructorTableWebAssemblyCompileError
 @end
 */

static EncodedJSValue JSC_HOST_CALL constructJSWebAssemblyCompileError(ExecState* state)
{
    auto& vm = state->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);
    JSValue message = state->argument(0);
    auto* structure = InternalFunction::createSubclassStructure(state, state->newTarget(), asInternalFunction(state->jsCallee())->globalObject()->WebAssemblyCompileErrorStructure());
    RETURN_IF_EXCEPTION(scope, encodedJSValue());
    return JSValue::encode(JSWebAssemblyCompileError::create(state, vm, structure, message));
}

static EncodedJSValue JSC_HOST_CALL callJSWebAssemblyCompileError(ExecState* state)
{
    VM& vm = state->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);
    return JSValue::encode(throwConstructorCannotBeCalledAsFunctionTypeError(state, scope, "WebAssembly.CompileError"));
}

WebAssemblyCompileErrorConstructor* WebAssemblyCompileErrorConstructor::create(VM& vm, Structure* structure, WebAssemblyCompileErrorPrototype* thisPrototype)
{
    auto* constructor = new (NotNull, allocateCell<WebAssemblyCompileErrorConstructor>(vm.heap)) WebAssemblyCompileErrorConstructor(vm, structure);
    constructor->finishCreation(vm, thisPrototype);
    return constructor;
}

Structure* WebAssemblyCompileErrorConstructor::createStructure(VM& vm, JSGlobalObject* globalObject, JSValue prototype)
{
    return Structure::create(vm, globalObject, prototype, TypeInfo(ObjectType, StructureFlags), info());
}

void WebAssemblyCompileErrorConstructor::finishCreation(VM& vm, WebAssemblyCompileErrorPrototype* prototype)
{
    Base::finishCreation(vm, ASCIILiteral("CompileError"));
    putDirectWithoutTransition(vm, vm.propertyNames->prototype, prototype, ReadOnly | DontEnum | DontDelete);
    putDirectWithoutTransition(vm, vm.propertyNames->length, jsNumber(1), ReadOnly | DontEnum | DontDelete);
}

WebAssemblyCompileErrorConstructor::WebAssemblyCompileErrorConstructor(VM& vm, Structure* structure)
    : Base(vm, structure)
{
}

ConstructType WebAssemblyCompileErrorConstructor::getConstructData(JSCell*, ConstructData& constructData)
{
    constructData.native.function = constructJSWebAssemblyCompileError;
    return ConstructType::Host;
}

CallType WebAssemblyCompileErrorConstructor::getCallData(JSCell*, CallData& callData)
{
    callData.native.function = callJSWebAssemblyCompileError;
    return CallType::Host;
}

} // namespace JSC

#endif // ENABLE(WEBASSEMBLY)
