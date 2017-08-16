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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "TypedArrayCTest.h"

#include "JavaScriptCore.h"
#include <wtf/Assertions.h>

extern "C" void JSSynchronousGarbageCollectForDebugging(JSContextRef);

static void id(void*, void*) { }
static void freePtr(void* ptr, void*)
{
    free(ptr);
}

static const unsigned numLengths = 3;

static const unsigned lengths[numLengths] =
{
    0,
    1,
    10,
};

static const unsigned byteSizes[kJSTypedArrayTypeArrayBuffer] =
{
    1, // kJSTypedArrayTypeInt8Array
    2, // kJSTypedArrayTypeInt16Array
    4, // kJSTypedArrayTypeInt32Array
    1, // kJSTypedArrayTypeUint8Array
    1, // kJSTypedArrayTypeUint8ClampedArray
    2, // kJSTypedArrayTypeUint16Array
    4, // kJSTypedArrayTypeUint32Array
    4, // kJSTypedArrayTypeFloat32Array
    8, // kJSTypedArrayTypeFloat64Array
};

static const char* typeToString[kJSTypedArrayTypeArrayBuffer] =
{
    "kJSTypedArrayTypeInt8Array",
    "kJSTypedArrayTypeInt16Array",
    "kJSTypedArrayTypeInt32Array",
    "kJSTypedArrayTypeUint8Array",
    "kJSTypedArrayTypeUint8ClampedArray",
    "kJSTypedArrayTypeUint16Array",
    "kJSTypedArrayTypeUint32Array",
    "kJSTypedArrayTypeFloat32Array",
    "kJSTypedArrayTypeFloat64Array",
};

inline int unexpectedException(const char* name)
{
    fprintf(stderr, "%s FAILED: unexpected exception\n", name);
    return 1;
}

static int assertEqualsAsNumber(JSGlobalContextRef context, JSValueRef value, double expectedValue)
{
    double number = JSValueToNumber(context, value, nullptr);
    if (number != expectedValue && !(isnan(number) && isnan(expectedValue))) {
        fprintf(stderr, "assertEqualsAsNumber FAILED: %p, %lf\n", value, expectedValue);
        return 1;
    }
    return 0;
}

static int testAccess(JSGlobalContextRef context, JSObjectRef typedArray, JSTypedArrayType type, unsigned elementLength, void* expectedPtr = nullptr, JSObjectRef expectedBuffer = nullptr, unsigned expectedOffset = 0)
{
    JSValueRef exception = nullptr;
    // Test typedArray basic functions.
    JSTypedArrayType actualType = JSValueGetTypedArrayType(context, typedArray, &exception);
    if (type != actualType || exception) {
        fprintf(stderr, "TypedArray type FAILED: %p, got: %s, expected: %s\n", typedArray, typeToString[actualType], typeToString[type]);
        return 1;
    }

    unsigned length = JSObjectGetTypedArrayLength(context, typedArray, &exception);
    if (elementLength != length || exception) {
        fprintf(stderr, "TypedArray length FAILED: %p (%s), got: %d, expected: %d\n", typedArray, typeToString[type], length, elementLength);
        return 1;
    }

    unsigned byteLength = JSObjectGetTypedArrayByteLength(context, typedArray, &exception);
    unsigned expectedLength = byteSizes[type] * elementLength;
    if (byteLength != expectedLength || exception) {
        fprintf(stderr, "TypedArray byteLength FAILED: %p (%s), got: %d, expected: %d\n", typedArray, typeToString[type], byteLength, expectedLength);
        return 1;
    }

    unsigned offset = JSObjectGetTypedArrayByteOffset(context, typedArray, &exception);
    if (expectedOffset != offset || exception) {
        fprintf(stderr, "TypedArray byteOffset FAILED: %p (%s), got: %d, expected: %d\n", typedArray, typeToString[type], offset, expectedOffset);
        return 1;
    }

    void* ptr = JSObjectGetTypedArrayBytesPtr(context, typedArray, &exception);
    if (exception)
        return unexpectedException("TypedArray get bytes ptr");

    JSObjectRef buffer = JSObjectGetTypedArrayBuffer(context, typedArray, &exception);
    if (exception)
        return unexpectedException("TypedArray get buffer");

    void* bufferPtr = JSObjectGetArrayBufferBytesPtr(context, buffer, &exception);
    if (exception)
        return unexpectedException("ArrayBuffer get bytes ptr");

    if (bufferPtr != ptr) {
        fprintf(stderr, "FAIL: TypedArray bytes ptr and ArrayBuffer byte ptr were not the same: %p (%s) TypedArray: %p, ArrayBuffer: %p\n", typedArray, typeToString[type], ptr, bufferPtr);
        return 1;
    }

    if (expectedPtr && ptr != expectedPtr) {
        fprintf(stderr, "FAIL: TypedArray bytes ptr and the ptr used to construct the array were not the same: %p (%s) TypedArray: %p, bytes ptr: %p\n", typedArray, typeToString[type], ptr, expectedPtr);
        return 1;
    }

    if (expectedBuffer && expectedBuffer != buffer) {
        fprintf(stderr, "FAIL: TypedArray buffer and the ArrayBuffer buffer used to construct the array were not the same: %p (%s) TypedArray buffer: %p, data: %p\n", typedArray, typeToString[type], buffer, expectedBuffer);
        return 1;
    }

    return 0;
}

static int testConstructors(JSGlobalContextRef context, JSTypedArrayType type, unsigned length)
{
    int failed = 0;
    JSValueRef exception = nullptr;
    JSObjectRef typedArray;

    // Test create with length.
    typedArray = JSObjectMakeTypedArray(context, type, length, &exception);
    failed = failed || exception || testAccess(context, typedArray, type, length);

    void* ptr = calloc(length, byteSizes[type]); // This is to be freed by data
    JSObjectRef data = JSObjectMakeArrayBufferWithBytesNoCopy(context, ptr, length * byteSizes[type], freePtr, nullptr, &exception);
    failed = failed || exception;

    // Test create with existing ptr.
    typedArray = JSObjectMakeTypedArrayWithBytesNoCopy(context, type, ptr, length * byteSizes[type], id, nullptr, &exception);
    failed = failed || exception || testAccess(context, typedArray, type, length, ptr);

    // Test create with existing ArrayBuffer.
    typedArray = JSObjectMakeTypedArrayWithArrayBuffer(context, type, data, &exception);
    failed = failed || exception || testAccess(context, typedArray, type, length, ptr, data);

    // Test create with existing ArrayBuffer and offset.
    typedArray = JSObjectMakeTypedArrayWithArrayBufferAndOffset(context, type, data, 0, length, &exception);
    failed = failed || exception || testAccess(context, typedArray, type, length, ptr, data);

    typedArray = JSObjectMakeTypedArrayWithArrayBufferAndOffset(context, type, data, byteSizes[type], length-1, &exception);
    if (!length)
        failed = failed || !exception;
    else
        failed = failed || testAccess(context, typedArray, type, length-1, ptr, data, byteSizes[type]) || exception;

    exception = nullptr;
    typedArray = JSObjectMakeTypedArrayWithArrayBufferAndOffset(context, type, data, byteSizes[type], 3, &exception);
    if (length < 2)
        failed = failed || !exception;
    else
        failed = failed || testAccess(context, typedArray, type, 3, ptr, data, byteSizes[type]) || exception;

    if (byteSizes[type] > 1) {
        exception = nullptr;
        typedArray = JSObjectMakeTypedArrayWithArrayBufferAndOffset(context, type, data, 1, length-1, &exception);
        failed = failed || !exception;
    }

    typedArray = JSObjectMakeTypedArrayWithArrayBufferAndOffset(context, type, data, byteSizes[type], length, &exception);
    failed = failed || !exception;

    exception = nullptr;
    typedArray = JSObjectMakeTypedArrayWithArrayBufferAndOffset(context, type, data, byteSizes[type], 0, &exception);
    if (!length)
        failed = failed || !exception;
    else
        failed = failed || testAccess(context, typedArray, type, 0, ptr, data, byteSizes[type]) || exception;

    return failed;
}

template <typename Functor>
static int forEachTypedArrayType(const Functor& functor)
{
    int failed = 0;
    for (unsigned i = 0; i < kJSTypedArrayTypeArrayBuffer; i++)
        failed = failed || functor(static_cast<JSTypedArrayType>(i));
    return failed;
}

int testTypedArrayCAPI()
{
    int failed = 0;
    JSGlobalContextRef context = JSGlobalContextCreate(nullptr);

    failed = failed || forEachTypedArrayType([&](JSTypedArrayType type) {
        int failed = 0;
        for (unsigned i = 0; i < numLengths; i++)
            failed = failed || testConstructors(context, type, lengths[i]);
        return failed;
    });

    // Test making a typedArray from scratch length.
    volatile JSObjectRef typedArray = JSObjectMakeTypedArray(context, kJSTypedArrayTypeUint32Array, 10, nullptr);
    JSObjectRef data = JSObjectGetTypedArrayBuffer(context, typedArray, nullptr);
    unsigned* buffer = static_cast<unsigned*>(JSObjectGetArrayBufferBytesPtr(context, data, nullptr));

    ASSERT(JSObjectGetTypedArrayLength(context, typedArray, nullptr) == 10);

    // Test buffer is connected to typedArray.
    buffer[1] = 1;
    JSValueRef v = JSObjectGetPropertyAtIndex(context, typedArray, 1, nullptr);
    failed = failed || assertEqualsAsNumber(context, v, 1);

    // Test passing a buffer from a new array to an old array
    typedArray = JSObjectMakeTypedArrayWithBytesNoCopy(context, kJSTypedArrayTypeUint32Array, buffer, 40, id, nullptr, nullptr);
    buffer = static_cast<unsigned*>(JSObjectGetTypedArrayBytesPtr(context, typedArray, nullptr));
    ASSERT(buffer[1] == 1);
    buffer[1] = 20;
    ASSERT(((unsigned*)JSObjectGetArrayBufferBytesPtr(context, data, nullptr))[1] == 20);

    // Test constructing with data and the data returned are the same even with an offset.
    typedArray = JSObjectMakeTypedArrayWithArrayBufferAndOffset(context, kJSTypedArrayTypeUint32Array, data, 4, 9, nullptr);
    failed = failed || assertEqualsAsNumber(context, JSObjectGetPropertyAtIndex(context, typedArray, 0, nullptr), 20);
    ASSERT(data == JSObjectGetTypedArrayBuffer(context, typedArray, nullptr));

    // Test attempting to allocate an array too big for memory.
    forEachTypedArrayType([&](JSTypedArrayType type) {
        JSValueRef exception = nullptr;
        JSObjectMakeTypedArray(context, type, UINT_MAX, &exception);
        return !exception;
    });

    JSGlobalContextRelease(context);

    if (!failed)
        printf("PASS: Typed Array C API Tests.\n");
    else
        printf("FAIL: Some Typed Array C API Tests failed.\n");

    return failed;
}
