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

#pragma once

#if ENABLE(WEBASSEMBLY)

#include "JSArrayBuffer.h"
#include "JSCJSValue.h"
#include "WebAssemblyFunction.h"
#include "WebAssemblyWrapperFunction.h"

namespace JSC {

ALWAYS_INLINE uint32_t toNonWrappingUint32(ExecState* exec, JSValue value)
{
    VM& vm = exec->vm();
    auto throwScope = DECLARE_THROW_SCOPE(vm);
    double doubleValue = value.toInteger(exec);
    RETURN_IF_EXCEPTION(throwScope, { });
    if (doubleValue < 0 || doubleValue > UINT_MAX) {
        throwException(exec, throwScope,
            createRangeError(exec, ASCIILiteral("Expect an integer argument in the range: [0, 2^32 - 1]")));
        return { };
    }

    return static_cast<uint32_t>(doubleValue);
}

ALWAYS_INLINE uint8_t* getWasmBufferFromValue(ExecState* exec, JSValue value, size_t& byteOffset, size_t& byteSize)
{
    VM& vm = exec->vm();
    auto throwScope = DECLARE_THROW_SCOPE(vm);
    // If the given bytes argument is not a BufferSource, a TypeError exception is thrown.
    JSArrayBuffer* arrayBuffer = value.getObject() ? jsDynamicCast<JSArrayBuffer*>(vm, value.getObject()) : nullptr;
    JSArrayBufferView* arrayBufferView = value.getObject() ? jsDynamicCast<JSArrayBufferView*>(vm, value.getObject()) : nullptr;
    if (!(arrayBuffer || arrayBufferView)) {
        throwException(exec, throwScope, createTypeError(exec,
            ASCIILiteral("first argument must be an ArrayBufferView or an ArrayBuffer"), defaultSourceAppender, runtimeTypeForValue(value)));
        return nullptr;
    }

    if (arrayBufferView ? arrayBufferView->isNeutered() : arrayBuffer->impl()->isNeutered()) {
        throwException(exec, throwScope, createTypeError(exec,
            ASCIILiteral("underlying TypedArray has been detatched from the ArrayBuffer"), defaultSourceAppender, runtimeTypeForValue(value)));
        return nullptr;
    }

    byteOffset = arrayBufferView ? arrayBufferView->byteOffset() : 0;
    byteSize = arrayBufferView ? arrayBufferView->length() : arrayBuffer->impl()->byteLength();
    return arrayBufferView ? static_cast<uint8_t*>(arrayBufferView->vector()) : static_cast<uint8_t*>(arrayBuffer->impl()->data());
}

ALWAYS_INLINE bool isWebAssemblyHostFunction(VM& vm, JSObject* object, WebAssemblyFunction*& wasmFunction, WebAssemblyWrapperFunction*& wasmWrapperFunction)
{
    if (object->inherits(vm, WebAssemblyFunction::info())) {
        wasmFunction = jsCast<WebAssemblyFunction*>(object);
        wasmWrapperFunction = nullptr;
        return true;
    }
    if (object->inherits(vm, WebAssemblyWrapperFunction::info())) {
        wasmWrapperFunction = jsCast<WebAssemblyWrapperFunction*>(object);
        wasmFunction = nullptr;
        return true;
    }
    return false;
}

ALWAYS_INLINE bool isWebAssemblyHostFunction(VM& vm, JSValue value, WebAssemblyFunction*& wasmFunction, WebAssemblyWrapperFunction*& wasmWrapperFunction)
{
    if (!value.isObject())
        return false;
    return isWebAssemblyHostFunction(vm, jsCast<JSObject*>(value), wasmFunction, wasmWrapperFunction);
}


ALWAYS_INLINE bool isWebAssemblyHostFunction(VM& vm, JSObject* object)
{
    WebAssemblyFunction* unused;
    WebAssemblyWrapperFunction* unused2;
    return isWebAssemblyHostFunction(vm, object, unused, unused2);
}

} // namespace JSC

#endif // ENABLE(WEBASSEMBLY)
