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
#include "WebAssemblyModuleConstructor.h"

#if ENABLE(WEBASSEMBLY)

#include "ExceptionHelpers.h"
#include "FunctionPrototype.h"
#include "JSArrayBuffer.h"
#include "JSCInlines.h"
#include "JSTypedArrays.h"
#include "JSWebAssemblyCallee.h"
#include "JSWebAssemblyCompileError.h"
#include "JSWebAssemblyHelpers.h"
#include "JSWebAssemblyModule.h"
#include "SymbolTable.h"
#include "WasmPlan.h"
#include "WebAssemblyModulePrototype.h"
#include <wtf/StdLibExtras.h>

#include "WebAssemblyModuleConstructor.lut.h"

namespace JSC {

const ClassInfo WebAssemblyModuleConstructor::s_info = { "Function", &Base::s_info, &constructorTableWebAssemblyModule, CREATE_METHOD_TABLE(WebAssemblyModuleConstructor) };

/* Source for WebAssemblyModuleConstructor.lut.h
 @begin constructorTableWebAssemblyModule
 @end
 */

static EncodedJSValue JSC_HOST_CALL constructJSWebAssemblyModule(ExecState* exec)
{
    VM& vm = exec->vm();
    auto throwScope = DECLARE_THROW_SCOPE(vm);
    auto* structure = InternalFunction::createSubclassStructure(exec, exec->newTarget(), exec->lexicalGlobalObject()->WebAssemblyModuleStructure());
    RETURN_IF_EXCEPTION(throwScope, encodedJSValue());
    throwScope.release();
    return JSValue::encode(WebAssemblyModuleConstructor::createModule(exec, exec->argument(0), structure));
}

static EncodedJSValue JSC_HOST_CALL callJSWebAssemblyModule(ExecState* state)
{
    VM& vm = state->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);
    return JSValue::encode(throwConstructorCannotBeCalledAsFunctionTypeError(state, scope, "WebAssembly.Module"));
}

JSValue WebAssemblyModuleConstructor::createModule(ExecState* state, JSValue buffer, Structure* structure)
{
    VM& vm = state->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    size_t byteOffset;
    size_t byteSize;
    uint8_t* base = getWasmBufferFromValue(state, buffer, byteOffset, byteSize);
    RETURN_IF_EXCEPTION(scope, { });

    scope.release();
    return JSWebAssemblyModule::create(vm, state, structure, base + byteOffset, byteSize);
}

WebAssemblyModuleConstructor* WebAssemblyModuleConstructor::create(VM& vm, Structure* structure, WebAssemblyModulePrototype* thisPrototype)
{
    auto* constructor = new (NotNull, allocateCell<WebAssemblyModuleConstructor>(vm.heap)) WebAssemblyModuleConstructor(vm, structure);
    constructor->finishCreation(vm, thisPrototype);
    return constructor;
}

Structure* WebAssemblyModuleConstructor::createStructure(VM& vm, JSGlobalObject* globalObject, JSValue prototype)
{
    return Structure::create(vm, globalObject, prototype, TypeInfo(ObjectType, StructureFlags), info());
}

void WebAssemblyModuleConstructor::finishCreation(VM& vm, WebAssemblyModulePrototype* prototype)
{
    Base::finishCreation(vm, ASCIILiteral("Module"));
    putDirectWithoutTransition(vm, vm.propertyNames->prototype, prototype, DontEnum | DontDelete | ReadOnly);
    putDirectWithoutTransition(vm, vm.propertyNames->length, jsNumber(1), ReadOnly | DontEnum | DontDelete);
}

WebAssemblyModuleConstructor::WebAssemblyModuleConstructor(VM& vm, Structure* structure)
    : Base(vm, structure)
{
}

ConstructType WebAssemblyModuleConstructor::getConstructData(JSCell*, ConstructData& constructData)
{
    constructData.native.function = constructJSWebAssemblyModule;
    return ConstructType::Host;
}

CallType WebAssemblyModuleConstructor::getCallData(JSCell*, CallData& callData)
{
    callData.native.function = callJSWebAssemblyModule;
    return CallType::Host;
}

void WebAssemblyModuleConstructor::visitChildren(JSCell* cell, SlotVisitor& visitor)
{
    auto* thisObject = jsCast<WebAssemblyModuleConstructor*>(cell);
    ASSERT_GC_OBJECT_INHERITS(thisObject, info());
    Base::visitChildren(thisObject, visitor);
}

} // namespace JSC

#endif // ENABLE(WEBASSEMBLY)

