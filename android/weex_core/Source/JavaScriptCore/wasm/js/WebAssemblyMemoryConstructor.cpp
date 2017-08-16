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
#include "WebAssemblyMemoryConstructor.h"

#if ENABLE(WEBASSEMBLY)

#include "FunctionPrototype.h"
#include "JSCInlines.h"
#include "JSWebAssemblyHelpers.h"
#include "JSWebAssemblyMemory.h"
#include "WasmMemory.h"
#include "WasmPageCount.h"
#include "WebAssemblyMemoryPrototype.h"
#include <wtf/Optional.h>

#include "WebAssemblyMemoryConstructor.lut.h"

namespace JSC {

const ClassInfo WebAssemblyMemoryConstructor::s_info = { "Function", &Base::s_info, &constructorTableWebAssemblyMemory, CREATE_METHOD_TABLE(WebAssemblyMemoryConstructor) };

/* Source for WebAssemblyMemoryConstructor.lut.h
 @begin constructorTableWebAssemblyMemory
 @end
 */

static EncodedJSValue JSC_HOST_CALL constructJSWebAssemblyMemory(ExecState* exec)
{
    VM& vm = exec->vm();
    auto throwScope = DECLARE_THROW_SCOPE(vm);
    if (exec->argumentCount() != 1)
        return JSValue::encode(throwException(exec, throwScope, createTypeError(exec, ASCIILiteral("WebAssembly.Memory expects exactly one argument"))));

    JSObject* memoryDescriptor;
    {
        JSValue argument = exec->argument(0);
        if (!argument.isObject())
            return JSValue::encode(throwException(exec, throwScope, createTypeError(exec, ASCIILiteral("WebAssembly.Memory expects its first argument to be an object"))));
        memoryDescriptor = jsCast<JSObject*>(argument);
    }

    Wasm::PageCount initialPageCount;
    {
        Identifier initial = Identifier::fromString(&vm, "initial");
        JSValue minSizeValue = memoryDescriptor->get(exec, initial);
        RETURN_IF_EXCEPTION(throwScope, encodedJSValue());
        uint32_t size = toNonWrappingUint32(exec, minSizeValue);
        RETURN_IF_EXCEPTION(throwScope, encodedJSValue());
        if (!Wasm::PageCount::isValid(size))
            return JSValue::encode(throwException(exec, throwScope, createRangeError(exec, ASCIILiteral("WebAssembly.Memory 'initial' page count is too large"))));
        initialPageCount = Wasm::PageCount(size);
    }

    Wasm::PageCount maximumPageCount;
    {
        Identifier maximum = Identifier::fromString(&vm, "maximum");
        bool hasProperty = memoryDescriptor->hasProperty(exec, maximum);
        RETURN_IF_EXCEPTION(throwScope, encodedJSValue());
        if (hasProperty) {
            JSValue maxSizeValue = memoryDescriptor->get(exec, maximum);
            RETURN_IF_EXCEPTION(throwScope, encodedJSValue());
            uint32_t size = toNonWrappingUint32(exec, maxSizeValue);
            RETURN_IF_EXCEPTION(throwScope, encodedJSValue());
            if (!Wasm::PageCount::isValid(size))
                return JSValue::encode(throwException(exec, throwScope, createRangeError(exec, ASCIILiteral("WebAssembly.Memory 'maximum' page count is too large"))));
            maximumPageCount = Wasm::PageCount(size);

            if (initialPageCount > maximumPageCount) {
                return JSValue::encode(throwException(exec, throwScope,
                    createRangeError(exec, ASCIILiteral("'maximum' page count must be than greater than or equal to the 'initial' page count"))));
            }
        }
    }

    RefPtr<Wasm::Memory> memory = Wasm::Memory::create(vm, initialPageCount, maximumPageCount);
    if (!memory)
        return JSValue::encode(throwException(exec, throwScope, createOutOfMemoryError(exec)));

    return JSValue::encode(JSWebAssemblyMemory::create(vm, exec->lexicalGlobalObject()->WebAssemblyMemoryStructure(), adoptRef(*memory.leakRef())));
}

static EncodedJSValue JSC_HOST_CALL callJSWebAssemblyMemory(ExecState* state)
{
    VM& vm = state->vm();
    auto throwScope = DECLARE_THROW_SCOPE(vm);
    return JSValue::encode(throwConstructorCannotBeCalledAsFunctionTypeError(state, throwScope, "WebAssembly.Memory"));
}

WebAssemblyMemoryConstructor* WebAssemblyMemoryConstructor::create(VM& vm, Structure* structure, WebAssemblyMemoryPrototype* thisPrototype)
{
    auto* constructor = new (NotNull, allocateCell<WebAssemblyMemoryConstructor>(vm.heap)) WebAssemblyMemoryConstructor(vm, structure);
    constructor->finishCreation(vm, thisPrototype);
    return constructor;
}

Structure* WebAssemblyMemoryConstructor::createStructure(VM& vm, JSGlobalObject* globalObject, JSValue prototype)
{
    return Structure::create(vm, globalObject, prototype, TypeInfo(ObjectType, StructureFlags), info());
}

void WebAssemblyMemoryConstructor::finishCreation(VM& vm, WebAssemblyMemoryPrototype* prototype)
{
    Base::finishCreation(vm, ASCIILiteral("Memory"));
    putDirectWithoutTransition(vm, vm.propertyNames->prototype, prototype, DontEnum | DontDelete | ReadOnly);
    putDirectWithoutTransition(vm, vm.propertyNames->length, jsNumber(1), ReadOnly | DontEnum | DontDelete);
}

WebAssemblyMemoryConstructor::WebAssemblyMemoryConstructor(VM& vm, Structure* structure)
    : Base(vm, structure)
{
}

ConstructType WebAssemblyMemoryConstructor::getConstructData(JSCell*, ConstructData& constructData)
{
    constructData.native.function = constructJSWebAssemblyMemory;
    return ConstructType::Host;
}

CallType WebAssemblyMemoryConstructor::getCallData(JSCell*, CallData& callData)
{
    callData.native.function = callJSWebAssemblyMemory;
    return CallType::Host;
}

void WebAssemblyMemoryConstructor::visitChildren(JSCell* cell, SlotVisitor& visitor)
{
    auto* thisObject = jsCast<WebAssemblyMemoryConstructor*>(cell);
    ASSERT_GC_OBJECT_INHERITS(thisObject, info());
    Base::visitChildren(thisObject, visitor);
}

} // namespace JSC

#endif // ENABLE(WEBASSEMBLY)

