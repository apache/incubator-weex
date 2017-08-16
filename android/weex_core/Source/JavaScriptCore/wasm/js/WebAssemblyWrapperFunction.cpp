/*
 * Copyright (C) 2017 Apple Inc. All rights reserved.
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
#include "WebAssemblyWrapperFunction.h"

#if ENABLE(WEBASSEMBLY)

#include "Error.h"
#include "FunctionPrototype.h"
#include "JSCInlines.h"

namespace JSC {

const ClassInfo WebAssemblyWrapperFunction::s_info = { "WebAssemblyWrapperFunction", &Base::s_info, nullptr, CREATE_METHOD_TABLE(WebAssemblyWrapperFunction) };

static EncodedJSValue JSC_HOST_CALL callWebAssemblyWrapperFunction(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);
    WebAssemblyWrapperFunction* wasmFunction = jsDynamicCast<WebAssemblyWrapperFunction*>(vm, exec->jsCallee());
    if (!wasmFunction)
        return JSValue::encode(throwException(exec, scope, createTypeError(exec, "expected a WebAssembly function")));

    CallData callData;
    JSObject* function = wasmFunction->function();
    CallType callType = function->methodTable(vm)->getCallData(function, callData);
    RELEASE_ASSERT(callType != CallType::None);
    scope.release();
    return JSValue::encode(call(exec, function, callType, callData, jsUndefined(), ArgList(exec)));
}

WebAssemblyWrapperFunction::WebAssemblyWrapperFunction(VM& vm, JSGlobalObject* globalObject, Structure* structure, Wasm::SignatureIndex signatureIndex, void* wasmEntrypointCode)
    : Base(vm, globalObject, structure)
    , m_wasmEntrypointCode(wasmEntrypointCode)
    , m_signatureIndex(signatureIndex)
{ }

WebAssemblyWrapperFunction* WebAssemblyWrapperFunction::create(VM& vm, JSGlobalObject* globalObject, JSObject* function, unsigned importIndex, JSWebAssemblyCodeBlock* codeBlock, Wasm::SignatureIndex signatureIndex)
{
    ASSERT_WITH_MESSAGE(!function->inherits(vm, WebAssemblyWrapperFunction::info()), "We should never double wrap a wrapper function.");
    String name = "";
    NativeExecutable* executable = vm.getHostFunction(callWebAssemblyWrapperFunction, NoIntrinsic, callHostFunctionAsConstructor, nullptr, name);
    WebAssemblyWrapperFunction* result = new (NotNull, allocateCell<WebAssemblyWrapperFunction>(vm.heap)) WebAssemblyWrapperFunction(vm, globalObject, globalObject->webAssemblyWrapperFunctionStructure(), signatureIndex, codeBlock->wasmToJsCallStubForImport(importIndex));
    const Wasm::Signature* signature = Wasm::SignatureInformation::get(&vm, signatureIndex);
    result->finishCreation(vm, executable, signature->argumentCount(), name, function, codeBlock);
    return result;
}

void WebAssemblyWrapperFunction::finishCreation(VM& vm, NativeExecutable* executable, unsigned length, const String& name, JSObject* function, JSWebAssemblyCodeBlock* codeBlock)
{
    Base::finishCreation(vm, executable, length, name);
    RELEASE_ASSERT(JSValue(function).isFunction());
    m_function.set(vm, this, function);
    m_codeBlock.set(vm, this, codeBlock);
}

Structure* WebAssemblyWrapperFunction::createStructure(VM& vm, JSGlobalObject* globalObject, JSValue prototype)
{
    ASSERT(globalObject);
    return Structure::create(vm, globalObject, prototype, TypeInfo(ObjectType, StructureFlags), info());
}

void WebAssemblyWrapperFunction::visitChildren(JSCell* cell, SlotVisitor& visitor)
{
    WebAssemblyWrapperFunction* thisObject = jsCast<WebAssemblyWrapperFunction*>(cell);
    ASSERT_GC_OBJECT_INHERITS(thisObject, info());
    Base::visitChildren(thisObject, visitor);

    visitor.append(thisObject->m_codeBlock);
    visitor.append(thisObject->m_function);
}

} // namespace JSC

#endif // ENABLE(WEBASSEMBLY)
