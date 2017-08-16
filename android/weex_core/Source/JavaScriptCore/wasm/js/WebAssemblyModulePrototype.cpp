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
#include "WebAssemblyModulePrototype.h"

#if ENABLE(WEBASSEMBLY)

#include "ArrayBuffer.h"
#include "FunctionPrototype.h"
#include "JSArrayBuffer.h"
#include "JSCInlines.h"
#include "JSWebAssemblyModule.h"

namespace JSC {
static EncodedJSValue JSC_HOST_CALL webAssemblyModuleProtoCustomSections(ExecState*);
}

#include "WebAssemblyModulePrototype.lut.h"

namespace JSC {

const ClassInfo WebAssemblyModulePrototype::s_info = { "WebAssembly.Module.prototype", &Base::s_info, &prototypeTableWebAssemblyModule, CREATE_METHOD_TABLE(WebAssemblyModulePrototype) };

/* Source for WebAssemblyModulePrototype.lut.h
 @begin prototypeTableWebAssemblyModule
 customSections webAssemblyModuleProtoCustomSections DontEnum|Function 1
 @end
 */

EncodedJSValue JSC_HOST_CALL webAssemblyModuleProtoCustomSections(ExecState* exec)
{
    VM& vm = exec->vm();
    auto* globalObject = exec->lexicalGlobalObject();
    auto throwScope = DECLARE_THROW_SCOPE(vm);

    JSWebAssemblyModule* module = jsDynamicCast<JSWebAssemblyModule*>(vm, exec->thisValue());
    if (!module)
        throwException(exec, throwScope, createTypeError(exec, ASCIILiteral("WebAssembly.Module.prototype.customSections called with non WebAssembly.Module |this| value")));
    RETURN_IF_EXCEPTION(throwScope, { });

    const String sectionNameString = exec->argument(0).getString(exec);
    RETURN_IF_EXCEPTION(throwScope, { });

    JSArray* result = constructEmptyArray(exec, nullptr, globalObject);
    RETURN_IF_EXCEPTION(throwScope, { });

    const auto& customSections = module->moduleInformation().customSections;
    for (const Wasm::CustomSection& section : customSections) {
        if (section.name == sectionNameString) {
            auto buffer = ArrayBuffer::tryCreate(section.payload.data(), section.payload.size());
            if (!buffer)
                throwException(exec, throwScope, createOutOfMemoryError(exec));

            Structure* arrayBufferStructure = InternalFunction::createSubclassStructure(exec, JSValue(), globalObject->arrayBufferStructure(ArrayBufferSharingMode::Default));
            RETURN_IF_EXCEPTION(throwScope, { });

            result->push(exec, JSArrayBuffer::create(vm, arrayBufferStructure, WTFMove(buffer)));
            RETURN_IF_EXCEPTION(throwScope, { });
        }
    }

    return JSValue::encode(result);
}

WebAssemblyModulePrototype* WebAssemblyModulePrototype::create(VM& vm, JSGlobalObject*, Structure* structure)
{
    auto* object = new (NotNull, allocateCell<WebAssemblyModulePrototype>(vm.heap)) WebAssemblyModulePrototype(vm, structure);
    object->finishCreation(vm);
    return object;
}

Structure* WebAssemblyModulePrototype::createStructure(VM& vm, JSGlobalObject* globalObject, JSValue prototype)
{
    return Structure::create(vm, globalObject, prototype, TypeInfo(ObjectType, StructureFlags), info());
}

void WebAssemblyModulePrototype::finishCreation(VM& vm)
{
    Base::finishCreation(vm);
}

WebAssemblyModulePrototype::WebAssemblyModulePrototype(VM& vm, Structure* structure)
    : Base(vm, structure)
{
}

} // namespace JSC

#endif // ENABLE(WEBASSEMBLY)
