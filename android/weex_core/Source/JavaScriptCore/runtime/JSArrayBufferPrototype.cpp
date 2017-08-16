/*
 * Copyright (C) 2013, 2016 Apple Inc. All rights reserved.
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
#include "JSArrayBufferPrototype.h"

#include "Error.h"
#include "ExceptionHelpers.h"
#include "JSArrayBuffer.h"
#include "JSFunction.h"
#include "JSCInlines.h"
#include "TypedArrayAdaptors.h"

namespace JSC {

static EncodedJSValue JSC_HOST_CALL arrayBufferProtoFuncSlice(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    JSFunction* callee = jsCast<JSFunction*>(exec->jsCallee());
    
    JSArrayBuffer* thisObject = jsDynamicCast<JSArrayBuffer*>(vm, exec->thisValue());
    if (!thisObject)
        return throwVMTypeError(exec, scope, ASCIILiteral("Receiver of slice must be an array buffer."));
    
    if (!exec->argumentCount())
        return throwVMTypeError(exec, scope, ASCIILiteral("Slice requires at least one argument."));
    
    int32_t begin = exec->argument(0).toInt32(exec);
    RETURN_IF_EXCEPTION(scope, encodedJSValue());
    
    int32_t end;
    if (exec->argumentCount() >= 2) {
        end = exec->uncheckedArgument(1).toInt32(exec);
        RETURN_IF_EXCEPTION(scope, encodedJSValue());
    } else
        end = thisObject->impl()->byteLength();
    
    RefPtr<ArrayBuffer> newBuffer = thisObject->impl()->slice(begin, end);
    if (!newBuffer)
        return JSValue::encode(throwOutOfMemoryError(exec, scope));
    
    Structure* structure = callee->globalObject()->arrayBufferStructure(newBuffer->sharingMode());
    
    JSArrayBuffer* result = JSArrayBuffer::create(vm, structure, WTFMove(newBuffer));
    
    return JSValue::encode(result);
}

// http://tc39.github.io/ecmascript_sharedmem/shmem.html#sec-get-arraybuffer.prototype.bytelength
static EncodedJSValue JSC_HOST_CALL arrayBufferProtoGetterFuncByteLength(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);
    JSValue thisValue = exec->thisValue();
    if (!thisValue.isObject())
        return throwVMTypeError(exec, scope, ASCIILiteral("Receiver should be an array buffer but was not an object"));

    auto* thisObject = jsDynamicCast<JSArrayBuffer*>(vm, thisValue);
    if (!thisObject)
        return throwVMTypeError(exec, scope, ASCIILiteral("Receiver should be an array buffer"));
    if (thisObject->isShared())
        return throwVMTypeError(exec, scope, ASCIILiteral("Receiver should not be a shared array buffer"));

    scope.release();

    return JSValue::encode(jsNumber(thisObject->impl()->byteLength()));
}

// http://tc39.github.io/ecmascript_sharedmem/shmem.html#StructuredData.SharedArrayBuffer.prototype.get_byteLength
static EncodedJSValue JSC_HOST_CALL sharedArrayBufferProtoGetterFuncByteLength(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);
    JSValue thisValue = exec->thisValue();
    if (!thisValue.isObject())
        return throwVMTypeError(exec, scope, ASCIILiteral("Receiver should be an array buffer but was not an object"));

    auto* thisObject = jsDynamicCast<JSArrayBuffer*>(vm, thisValue);
    if (!thisObject)
        return throwVMTypeError(exec, scope, ASCIILiteral("Receiver should be an array buffer"));
    if (!thisObject->isShared())
        return throwVMTypeError(exec, scope, ASCIILiteral("Receiver should be a shared array buffer"));

    scope.release();

    return JSValue::encode(jsNumber(thisObject->impl()->byteLength()));
}

const ClassInfo JSArrayBufferPrototype::s_info = {
    "ArrayBufferPrototype", &Base::s_info, 0, CREATE_METHOD_TABLE(JSArrayBufferPrototype)
};

JSArrayBufferPrototype::JSArrayBufferPrototype(VM& vm, Structure* structure, ArrayBufferSharingMode sharingMode)
    : Base(vm, structure)
    , m_sharingMode(sharingMode)
{
}

void JSArrayBufferPrototype::finishCreation(VM& vm, JSGlobalObject* globalObject)
{
    Base::finishCreation(vm);
    
    JSC_NATIVE_FUNCTION_WITHOUT_TRANSITION(vm.propertyNames->slice, arrayBufferProtoFuncSlice, DontEnum, 2);
    putDirectWithoutTransition(vm, vm.propertyNames->toStringTagSymbol, jsString(&vm, arrayBufferSharingModeName(m_sharingMode)), DontEnum | ReadOnly);
    if (m_sharingMode == ArrayBufferSharingMode::Default)
        JSC_NATIVE_GETTER(vm.propertyNames->byteLength, arrayBufferProtoGetterFuncByteLength, DontEnum | ReadOnly);
    else
        JSC_NATIVE_GETTER(vm.propertyNames->byteLength, sharedArrayBufferProtoGetterFuncByteLength, DontEnum | ReadOnly);
}

JSArrayBufferPrototype* JSArrayBufferPrototype::create(VM& vm, JSGlobalObject* globalObject, Structure* structure, ArrayBufferSharingMode sharingMode)
{
    JSArrayBufferPrototype* prototype =
        new (NotNull, allocateCell<JSArrayBufferPrototype>(vm.heap))
        JSArrayBufferPrototype(vm, structure, sharingMode);
    prototype->finishCreation(vm, globalObject);
    return prototype;
}

Structure* JSArrayBufferPrototype::createStructure(
    VM& vm, JSGlobalObject* globalObject, JSValue prototype)
{
    return Structure::create(
        vm, globalObject, prototype, TypeInfo(ObjectType, StructureFlags), info());
}

} // namespace JSC

