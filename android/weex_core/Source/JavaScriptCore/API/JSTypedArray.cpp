/*
 * Copyright (C) 2015 Dominic Szablewski (dominic@phoboslab.org)
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
 * THIS SOFTWARE IS PROVIDED BY APPLE COMPUTER, INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE COMPUTER, INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "JSTypedArray.h"

#include "APICast.h"
#include "APIUtils.h"
#include "ClassInfo.h"
#include "Error.h"
#include "JSArrayBufferViewInlines.h"
#include "JSCInlines.h"
#include "JSDataView.h"
#include "JSGenericTypedArrayViewInlines.h"
#include "JSTypedArrays.h"
#include "TypedArrayController.h"
#include <wtf/RefPtr.h>

using namespace JSC;

// Helper functions.

inline JSTypedArrayType toJSTypedArrayType(TypedArrayType type)
{
    switch (type) {
    case JSC::TypeDataView:
    case NotTypedArray:
        return kJSTypedArrayTypeNone;
    case TypeInt8:
        return kJSTypedArrayTypeInt8Array;
    case TypeUint8:
        return kJSTypedArrayTypeUint8Array;
    case TypeUint8Clamped:
        return kJSTypedArrayTypeUint8ClampedArray;
    case TypeInt16:
        return kJSTypedArrayTypeInt16Array;
    case TypeUint16:
        return kJSTypedArrayTypeUint16Array;
    case TypeInt32:
        return kJSTypedArrayTypeInt32Array;
    case TypeUint32:
        return kJSTypedArrayTypeUint32Array;
    case TypeFloat32:
        return kJSTypedArrayTypeFloat32Array;
    case TypeFloat64:
        return kJSTypedArrayTypeFloat64Array;
    }
    RELEASE_ASSERT_NOT_REACHED();
}

inline TypedArrayType toTypedArrayType(JSTypedArrayType type)
{
    switch (type) {
    case kJSTypedArrayTypeArrayBuffer:
    case kJSTypedArrayTypeNone:
        return NotTypedArray;
    case kJSTypedArrayTypeInt8Array:
        return TypeInt8;
    case kJSTypedArrayTypeUint8Array:
        return TypeUint8;
    case kJSTypedArrayTypeUint8ClampedArray:
        return TypeUint8Clamped;
    case kJSTypedArrayTypeInt16Array:
        return TypeInt16;
    case kJSTypedArrayTypeUint16Array:
        return TypeUint16;
    case kJSTypedArrayTypeInt32Array:
        return TypeInt32;
    case kJSTypedArrayTypeUint32Array:
        return TypeUint32;
    case kJSTypedArrayTypeFloat32Array:
        return TypeFloat32;
    case kJSTypedArrayTypeFloat64Array:
        return TypeFloat64;
    }
    RELEASE_ASSERT_NOT_REACHED();
}

static JSObject* createTypedArray(ExecState* exec, JSTypedArrayType type, RefPtr<ArrayBuffer>&& buffer, size_t offset, size_t length)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);
    JSGlobalObject* globalObject = exec->lexicalGlobalObject();
    if (!buffer) {
        throwOutOfMemoryError(exec, scope);
        return nullptr;
    }
    switch (type) {
    case kJSTypedArrayTypeInt8Array:
        return JSInt8Array::create(exec, globalObject->typedArrayStructure(TypeInt8), WTFMove(buffer), offset, length);
    case kJSTypedArrayTypeInt16Array:
        return JSInt16Array::create(exec, globalObject->typedArrayStructure(TypeInt16), WTFMove(buffer), offset, length);
    case kJSTypedArrayTypeInt32Array:
        return JSInt32Array::create(exec, globalObject->typedArrayStructure(TypeInt32), WTFMove(buffer), offset, length);
    case kJSTypedArrayTypeUint8Array:
        return JSUint8Array::create(exec, globalObject->typedArrayStructure(TypeUint8), WTFMove(buffer), offset, length);
    case kJSTypedArrayTypeUint8ClampedArray:
        return JSUint8ClampedArray::create(exec, globalObject->typedArrayStructure(TypeUint8Clamped), WTFMove(buffer), offset, length);
    case kJSTypedArrayTypeUint16Array:
        return JSUint16Array::create(exec, globalObject->typedArrayStructure(TypeUint16), WTFMove(buffer), offset, length);
    case kJSTypedArrayTypeUint32Array:
        return JSUint32Array::create(exec, globalObject->typedArrayStructure(TypeUint32), WTFMove(buffer), offset, length);
    case kJSTypedArrayTypeFloat32Array:
        return JSFloat32Array::create(exec, globalObject->typedArrayStructure(TypeFloat32), WTFMove(buffer), offset, length);
    case kJSTypedArrayTypeFloat64Array:
        return JSFloat64Array::create(exec, globalObject->typedArrayStructure(TypeFloat64), WTFMove(buffer), offset, length);
    case kJSTypedArrayTypeArrayBuffer:
    case kJSTypedArrayTypeNone:
        RELEASE_ASSERT_NOT_REACHED();
    }
    return nullptr;
}

// Implementations of the API functions.

JSTypedArrayType JSValueGetTypedArrayType(JSContextRef ctx, JSValueRef valueRef, JSValueRef*)
{

    ExecState* exec = toJS(ctx);
    VM& vm = exec->vm();
    JSLockHolder locker(exec);

    JSValue value = toJS(exec, valueRef);
    if (!value.isObject())
        return kJSTypedArrayTypeNone;
    JSObject* object = value.getObject();

    if (jsDynamicCast<JSArrayBuffer*>(vm, object))
        return kJSTypedArrayTypeArrayBuffer;

    return toJSTypedArrayType(object->classInfo(vm)->typedArrayStorageType);
}

JSObjectRef JSObjectMakeTypedArray(JSContextRef ctx, JSTypedArrayType arrayType, size_t length, JSValueRef* exception)
{
    ExecState* exec = toJS(ctx);
    JSLockHolder locker(exec);

    if (arrayType == kJSTypedArrayTypeNone || arrayType == kJSTypedArrayTypeArrayBuffer)
        return nullptr;

    unsigned elementByteSize = elementSize(toTypedArrayType(arrayType));

    auto buffer = ArrayBuffer::tryCreate(length, elementByteSize);
    JSObject* result = createTypedArray(exec, arrayType, WTFMove(buffer), 0, length);
    if (handleExceptionIfNeeded(exec, exception) == ExceptionStatus::DidThrow)
        return nullptr;
    return toRef(result);
}

JSObjectRef JSObjectMakeTypedArrayWithBytesNoCopy(JSContextRef ctx, JSTypedArrayType arrayType, void* bytes, size_t length, JSTypedArrayBytesDeallocator destructor, void* destructorContext, JSValueRef* exception)
{
    ExecState* exec = toJS(ctx);
    JSLockHolder locker(exec);

    if (arrayType == kJSTypedArrayTypeNone || arrayType == kJSTypedArrayTypeArrayBuffer)
        return nullptr;

    unsigned elementByteSize = elementSize(toTypedArrayType(arrayType));

    RefPtr<ArrayBuffer> buffer = ArrayBuffer::createFromBytes(bytes, length, [=](void* p) {
        if (destructor)
            destructor(p, destructorContext);
    });
    JSObject* result = createTypedArray(exec, arrayType, WTFMove(buffer), 0, length / elementByteSize);
    if (handleExceptionIfNeeded(exec, exception) == ExceptionStatus::DidThrow)
        return nullptr;
    return toRef(result);
}

JSObjectRef JSObjectMakeTypedArrayWithArrayBuffer(JSContextRef ctx, JSTypedArrayType arrayType, JSObjectRef jsBufferRef, JSValueRef* exception)
{
    ExecState* exec = toJS(ctx);
    VM& vm = exec->vm();
    JSLockHolder locker(exec);

    if (arrayType == kJSTypedArrayTypeNone || arrayType == kJSTypedArrayTypeArrayBuffer)
        return nullptr;

    JSArrayBuffer* jsBuffer = jsDynamicCast<JSArrayBuffer*>(vm, toJS(jsBufferRef));
    if (!jsBuffer) {
        setException(exec, exception, createTypeError(exec, "JSObjectMakeTypedArrayWithArrayBuffer expects buffer to be an Array Buffer object"));
        return nullptr;
    }

    RefPtr<ArrayBuffer> buffer = jsBuffer->impl();
    unsigned elementByteSize = elementSize(toTypedArrayType(arrayType));

    JSObject* result = createTypedArray(exec, arrayType, WTFMove(buffer), 0, buffer->byteLength() / elementByteSize);
    if (handleExceptionIfNeeded(exec, exception) == ExceptionStatus::DidThrow)
        return nullptr;
    return toRef(result);
}

JSObjectRef JSObjectMakeTypedArrayWithArrayBufferAndOffset(JSContextRef ctx, JSTypedArrayType arrayType, JSObjectRef jsBufferRef, size_t offset, size_t length, JSValueRef* exception)
{
    ExecState* exec = toJS(ctx);
    VM& vm = exec->vm();
    JSLockHolder locker(exec);

    if (arrayType == kJSTypedArrayTypeNone || arrayType == kJSTypedArrayTypeArrayBuffer)
        return nullptr;

    JSArrayBuffer* jsBuffer = jsDynamicCast<JSArrayBuffer*>(vm, toJS(jsBufferRef));
    if (!jsBuffer) {
        setException(exec, exception, createTypeError(exec, "JSObjectMakeTypedArrayWithArrayBuffer expects buffer to be an Array Buffer object"));
        return nullptr;
    }

    JSObject* result = createTypedArray(exec, arrayType, jsBuffer->impl(), offset, length);
    if (handleExceptionIfNeeded(exec, exception) == ExceptionStatus::DidThrow)
        return nullptr;
    return toRef(result);
}

void* JSObjectGetTypedArrayBytesPtr(JSContextRef ctx, JSObjectRef objectRef, JSValueRef*)
{
    ExecState* exec = toJS(ctx);
    VM& vm = exec->vm();
    JSLockHolder locker(exec);
    JSObject* object = toJS(objectRef);

    if (JSArrayBufferView* typedArray = jsDynamicCast<JSArrayBufferView*>(vm, object)) {
        ArrayBuffer* buffer = typedArray->possiblySharedBuffer();
        buffer->pinAndLock();
        return buffer->data();
    }
    return nullptr;
}

size_t JSObjectGetTypedArrayLength(JSContextRef ctx, JSObjectRef objectRef, JSValueRef*)
{
    ExecState* exec = toJS(ctx);
    VM& vm = exec->vm();
    JSObject* object = toJS(objectRef);

    if (JSArrayBufferView* typedArray = jsDynamicCast<JSArrayBufferView*>(vm, object))
        return typedArray->length();

    return 0;
}

size_t JSObjectGetTypedArrayByteLength(JSContextRef ctx, JSObjectRef objectRef, JSValueRef*)
{
    ExecState* exec = toJS(ctx);
    VM& vm = exec->vm();
    JSObject* object = toJS(objectRef);

    if (JSArrayBufferView* typedArray = jsDynamicCast<JSArrayBufferView*>(vm, object))
        return typedArray->length() * elementSize(typedArray->classInfo(vm)->typedArrayStorageType);

    return 0;
}

size_t JSObjectGetTypedArrayByteOffset(JSContextRef ctx, JSObjectRef objectRef, JSValueRef*)
{
    ExecState* exec = toJS(ctx);
    VM& vm = exec->vm();
    JSObject* object = toJS(objectRef);

    if (JSArrayBufferView* typedArray = jsDynamicCast<JSArrayBufferView*>(vm, object))
        return typedArray->byteOffset();

    return 0;
}

JSObjectRef JSObjectGetTypedArrayBuffer(JSContextRef ctx, JSObjectRef objectRef, JSValueRef*)
{
    ExecState* exec = toJS(ctx);
    VM& vm = exec->vm();
    JSLockHolder locker(exec);
    JSObject* object = toJS(objectRef);

    if (JSArrayBufferView* typedArray = jsDynamicCast<JSArrayBufferView*>(vm, object))
        return toRef(exec->vm().m_typedArrayController->toJS(exec, typedArray->globalObject(), typedArray->possiblySharedBuffer()));

    return nullptr;
}

JSObjectRef JSObjectMakeArrayBufferWithBytesNoCopy(JSContextRef ctx, void* bytes, size_t byteLength, JSTypedArrayBytesDeallocator bytesDeallocator, void* deallocatorContext, JSValueRef* exception)
{
    ExecState* exec = toJS(ctx);
    JSLockHolder locker(exec);

    auto buffer = ArrayBuffer::createFromBytes(bytes, byteLength, [=](void* p) {
        if (bytesDeallocator)
            bytesDeallocator(p, deallocatorContext);
    });

    JSArrayBuffer* jsBuffer = JSArrayBuffer::create(exec->vm(), exec->lexicalGlobalObject()->arrayBufferStructure(ArrayBufferSharingMode::Default), WTFMove(buffer));
    if (handleExceptionIfNeeded(exec, exception) == ExceptionStatus::DidThrow)
        return nullptr;

    return toRef(jsBuffer);
}

void* JSObjectGetArrayBufferBytesPtr(JSContextRef ctx, JSObjectRef objectRef, JSValueRef*)
{
    ExecState* exec = toJS(ctx);
    VM& vm = exec->vm();
    JSLockHolder locker(exec);
    JSObject* object = toJS(objectRef);

    if (JSArrayBuffer* jsBuffer = jsDynamicCast<JSArrayBuffer*>(vm, object)) {
        ArrayBuffer* buffer = jsBuffer->impl();
        buffer->pinAndLock();
        return buffer->data();
    }
    return nullptr;
}

size_t JSObjectGetArrayBufferByteLength(JSContextRef ctx, JSObjectRef objectRef, JSValueRef*)
{
    ExecState* exec = toJS(ctx);
    VM& vm = exec->vm();
    JSObject* object = toJS(objectRef);

    if (JSArrayBuffer* jsBuffer = jsDynamicCast<JSArrayBuffer*>(vm, object))
        return jsBuffer->impl()->byteLength();
    
    return 0;
}
