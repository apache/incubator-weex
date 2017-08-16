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
#include "JSWebAssembly.h"

#if ENABLE(WEBASSEMBLY)

#include "Exception.h"
#include "FunctionPrototype.h"
#include "JSCInlines.h"
#include "JSPromiseDeferred.h"
#include "JSWebAssemblyHelpers.h"
#include "ObjectConstructor.h"
#include "WasmPlan.h"
#include "WebAssemblyModuleConstructor.h"

namespace JSC {

STATIC_ASSERT_IS_TRIVIALLY_DESTRUCTIBLE(JSWebAssembly);

EncodedJSValue JSC_HOST_CALL webAssemblyValidateFunc(ExecState*);
EncodedJSValue JSC_HOST_CALL webAssemblyCompileFunc(ExecState*);
EncodedJSValue JSC_HOST_CALL webAssemblyInstantiateFunc(ExecState*);

EncodedJSValue JSC_HOST_CALL webAssemblyCompileFunc(ExecState* exec)
{
    VM& vm = exec->vm();
    auto catchScope = DECLARE_CATCH_SCOPE(vm);

    JSPromiseDeferred* promise = JSPromiseDeferred::create(exec, exec->lexicalGlobalObject());
    RETURN_IF_EXCEPTION(catchScope, encodedJSValue());

    // FIXME: Make this truly asynchronous:
    // https://bugs.webkit.org/show_bug.cgi?id=166016
    JSValue module = WebAssemblyModuleConstructor::createModule(exec, exec->argument(0), exec->lexicalGlobalObject()->WebAssemblyModuleStructure());
    if (Exception* exception = catchScope.exception()) {
        catchScope.clearException();
        promise->reject(exec, exception->value());
        return JSValue::encode(promise->promise());
    }

    promise->resolve(exec, module);
    return JSValue::encode(promise->promise());
}

EncodedJSValue JSC_HOST_CALL webAssemblyInstantiateFunc(ExecState* exec)
{
    VM& vm = exec->vm();
    auto catchScope = DECLARE_CATCH_SCOPE(vm);

    // FIXME: Make this API truly asynchronous: https://bugs.webkit.org/show_bug.cgi?id=169187

    JSPromiseDeferred* promise = JSPromiseDeferred::create(exec, exec->lexicalGlobalObject());
    RETURN_IF_EXCEPTION(catchScope, encodedJSValue());

    auto reject = [&] () {
        Exception* exception = catchScope.exception();
        ASSERT(exception);
        catchScope.clearException();
        promise->reject(exec, exception->value());
        return JSValue::encode(promise->promise());
    };

    JSValue importArgument = exec->argument(1);
    JSObject* importObject = importArgument.getObject();
    if (!importArgument.isUndefined() && !importObject) {
        promise->reject(exec, createTypeError(exec,
            ASCIILiteral("second argument to WebAssembly.instantiate must be undefined or an Object"), defaultSourceAppender, runtimeTypeForValue(importArgument)));
        return JSValue::encode(promise->promise());
    }

    JSValue firstArgument = exec->argument(0);
    JSValue module;
    bool firstArgumentIsModule = false;
    if (firstArgument.inherits(vm, JSWebAssemblyModule::info())) {
        firstArgumentIsModule = true;
        module = firstArgument;
    } else {
        module = WebAssemblyModuleConstructor::createModule(exec, firstArgument, exec->lexicalGlobalObject()->WebAssemblyModuleStructure());
        if (catchScope.exception())
            return reject();
    }

    JSWebAssemblyInstance* instance = WebAssemblyInstanceConstructor::createInstance(exec, jsCast<JSWebAssemblyModule*>(module), importObject, exec->lexicalGlobalObject()->WebAssemblyInstanceStructure());
    if (catchScope.exception())
        return reject();

    if (firstArgumentIsModule)
        promise->resolve(exec, instance);
    else {
        JSObject* result = constructEmptyObject(exec);
        result->putDirect(vm, Identifier::fromString(&vm, ASCIILiteral("module")), module);
        result->putDirect(vm, Identifier::fromString(&vm, ASCIILiteral("instance")), instance);
        promise->resolve(exec, result);
    }

    return JSValue::encode(promise->promise());
}

EncodedJSValue JSC_HOST_CALL webAssemblyValidateFunc(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    size_t byteOffset;
    size_t byteSize;
    uint8_t* base = getWasmBufferFromValue(exec, exec->argument(0), byteOffset, byteSize);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());
    Wasm::Plan plan(&vm, base + byteOffset, byteSize);
    // FIXME: We might want to throw an OOM exception here if we detect that something will OOM.
    // https://bugs.webkit.org/show_bug.cgi?id=166015
    return JSValue::encode(jsBoolean(plan.parseAndValidateModule()));
}

const ClassInfo JSWebAssembly::s_info = { "WebAssembly", &Base::s_info, 0, CREATE_METHOD_TABLE(JSWebAssembly) };

JSWebAssembly* JSWebAssembly::create(VM& vm, JSGlobalObject* globalObject, Structure* structure)
{
    auto* object = new (NotNull, allocateCell<JSWebAssembly>(vm.heap)) JSWebAssembly(vm, structure);
    object->finishCreation(vm, globalObject);
    return object;
}

Structure* JSWebAssembly::createStructure(VM& vm, JSGlobalObject* globalObject, JSValue prototype)
{
    return Structure::create(vm, globalObject, prototype, TypeInfo(ObjectType, StructureFlags), info());
}

void JSWebAssembly::finishCreation(VM& vm, JSGlobalObject* globalObject)
{
    Base::finishCreation(vm);
    ASSERT(inherits(vm, info()));
    JSC_NATIVE_FUNCTION_WITHOUT_TRANSITION("validate", webAssemblyValidateFunc, DontEnum, 1);
    JSC_NATIVE_FUNCTION_WITHOUT_TRANSITION("compile", webAssemblyCompileFunc, DontEnum, 1);
    JSC_NATIVE_FUNCTION_WITHOUT_TRANSITION("instantiate", webAssemblyInstantiateFunc, DontEnum, 1);
}

JSWebAssembly::JSWebAssembly(VM& vm, Structure* structure)
    : JSNonFinalObject(vm, structure)
{
}

} // namespace JSC

#endif // ENABLE(WEBASSEMBLY)
