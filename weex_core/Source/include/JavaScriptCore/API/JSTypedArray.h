/**
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */
#ifndef JSTypedArray_h
#define JSTypedArray_h

#include <JavaScriptCore/JSBase.h>
#include <JavaScriptCore/JSValueRef.h>

#ifdef __cplusplus
extern "C" {
#endif

// ------------- Typed Array functions --------------

/*!
 @function
 @abstract           Creates a JavaScript Typed Array object with the given number of elements.
 @param ctx          The execution context to use.
 @param arrayType    A value identifying the type of array to create. If arrayType is kJSTypedArrayTypeNone or kJSTypedArrayTypeArrayBuffer then NULL will be returned.
 @param length       The number of elements to be in the new Typed Array.
 @param exception    A pointer to a JSValueRef in which to store an exception, if any. Pass NULL if you do not care to store an exception.
 @result             A JSObjectRef that is a Typed Array with all elements set to zero or NULL if there was an error.
 */
JS_EXPORT JSObjectRef JSObjectMakeTypedArray(JSContextRef ctx, JSTypedArrayType arrayType, size_t length, JSValueRef* exception) CF_AVAILABLE(10_12, 10_0);

/*!
 @function
 @abstract                 Creates a JavaScript Typed Array object from an existing pointer.
 @param ctx                The execution context to use.
 @param arrayType          A value identifying the type of array to create. If arrayType is kJSTypedArrayTypeNone or kJSTypedArrayTypeArrayBuffer then NULL will be returned.
 @param bytes              A pointer to the byte buffer to be used as the backing store of the Typed Array object.
 @param byteLength         The number of bytes pointed to by the parameter bytes.
 @param bytesDeallocator   The allocator to use to deallocate the external buffer when the JSTypedArrayData object is deallocated.
 @param deallocatorContext A pointer to pass back to the deallocator.
 @param exception          A pointer to a JSValueRef in which to store an exception, if any. Pass NULL if you do not care to store an exception.
 @result                   A JSObjectRef Typed Array whose backing store is the same as the one pointed to by bytes or NULL if there was an error.
 @discussion               If an exception is thrown during this function the bytesDeallocator will always be called.
 */
JS_EXPORT JSObjectRef JSObjectMakeTypedArrayWithBytesNoCopy(JSContextRef ctx, JSTypedArrayType arrayType, void* bytes, size_t byteLength, JSTypedArrayBytesDeallocator bytesDeallocator, void* deallocatorContext, JSValueRef* exception) CF_AVAILABLE(10_12, 10_0);

/*!
 @function
 @abstract           Creates a JavaScript Typed Array object from an existing JavaScript Array Buffer object.
 @param ctx          The execution context to use.
 @param arrayType    A value identifying the type of array to create. If arrayType is kJSTypedArrayTypeNone or kJSTypedArrayTypeArrayBuffer then NULL will be returned.
 @param buffer       An Array Buffer object that should be used as the backing store for the created JavaScript Typed Array object.
 @param exception    A pointer to a JSValueRef in which to store an exception, if any. Pass NULL if you do not care to store an exception.
 @result             A JSObjectRef that is a Typed Array or NULL if there was an error. The backing store of the Typed Array will be buffer.
 */
JS_EXPORT JSObjectRef JSObjectMakeTypedArrayWithArrayBuffer(JSContextRef ctx, JSTypedArrayType arrayType, JSObjectRef buffer, JSValueRef* exception) CF_AVAILABLE(10_12, 10_0);

/*!
 @function
 @abstract           Creates a JavaScript Typed Array object from an existing JavaScript Array Buffer object with the given offset and length.
 @param ctx          The execution context to use.
 @param arrayType    A value identifying the type of array to create. If arrayType is kJSTypedArrayTypeNone or kJSTypedArrayTypeArrayBuffer then NULL will be returned.
 @param buffer       An Array Buffer object that should be used as the backing store for the created JavaScript Typed Array object.
 @param byteOffset   The byte offset for the created Typed Array. byteOffset should aligned with the element size of arrayType.
 @param length       The number of elements to include in the Typed Array.
 @param exception    A pointer to a JSValueRef in which to store an exception, if any. Pass NULL if you do not care to store an exception.
 @result             A JSObjectRef that is a Typed Array or NULL if there was an error. The backing store of the Typed Array will be buffer.
 */
JS_EXPORT JSObjectRef JSObjectMakeTypedArrayWithArrayBufferAndOffset(JSContextRef ctx, JSTypedArrayType arrayType, JSObjectRef buffer, size_t byteOffset, size_t length, JSValueRef* exception) CF_AVAILABLE(10_12, 10_0);

/*!
 @function
 @abstract           Returns a temporary pointer to the backing store of a JavaScript Typed Array object.
 @param ctx          The execution context to use.
 @param object       The Typed Array object whose backing store pointer to return.
 @param exception    A pointer to a JSValueRef in which to store an exception, if any. Pass NULL if you do not care to store an exception.
 @result             A pointer to the raw data buffer that serves as object's backing store or NULL if object is not a Typed Array object.
 @discussion         The pointer returned by this function is temporary and is not guaranteed to remain valid across JavaScriptCore API calls.
 */
JS_EXPORT void* JSObjectGetTypedArrayBytesPtr(JSContextRef ctx, JSObjectRef object, JSValueRef* exception) CF_AVAILABLE(10_12, 10_0);

/*!
 @function
 @abstract           Returns the length of a JavaScript Typed Array object.
 @param ctx          The execution context to use.
 @param object       The Typed Array object whose length to return.
 @param exception    A pointer to a JSValueRef in which to store an exception, if any. Pass NULL if you do not care to store an exception.
 @result             The length of the Typed Array object or 0 if the object is not a Typed Array object.
 */
JS_EXPORT size_t JSObjectGetTypedArrayLength(JSContextRef ctx, JSObjectRef object, JSValueRef* exception) CF_AVAILABLE(10_12, 10_0);

/*!
 @function
 @abstract           Returns the byte length of a JavaScript Typed Array object.
 @param ctx          The execution context to use.
 @param object       The Typed Array object whose byte length to return.
 @param exception    A pointer to a JSValueRef in which to store an exception, if any. Pass NULL if you do not care to store an exception.
 @result             The byte length of the Typed Array object or 0 if the object is not a Typed Array object.
 */
JS_EXPORT size_t JSObjectGetTypedArrayByteLength(JSContextRef ctx, JSObjectRef object, JSValueRef* exception) CF_AVAILABLE(10_12, 10_0);

/*!
 @function
 @abstract           Returns the byte offset of a JavaScript Typed Array object.
 @param ctx          The execution context to use.
 @param object       The Typed Array object whose byte offset to return.
 @param exception    A pointer to a JSValueRef in which to store an exception, if any. Pass NULL if you do not care to store an exception.
 @result             The byte offset of the Typed Array object or 0 if the object is not a Typed Array object.
 */
JS_EXPORT size_t JSObjectGetTypedArrayByteOffset(JSContextRef ctx, JSObjectRef object, JSValueRef* exception) CF_AVAILABLE(10_12, 10_0);

/*!
 @function
 @abstract           Returns the JavaScript Array Buffer object that is used as the backing of a JavaScript Typed Array object.
 @param ctx          The execution context to use.
 @param object       The JSObjectRef whose Typed Array type data pointer to obtain.
 @param exception    A pointer to a JSValueRef in which to store an exception, if any. Pass NULL if you do not care to store an exception.
 @result             A JSObjectRef with a JSTypedArrayType of kJSTypedArrayTypeArrayBuffer or NULL if object is not a Typed Array.
 */
JS_EXPORT JSObjectRef JSObjectGetTypedArrayBuffer(JSContextRef ctx, JSObjectRef object, JSValueRef* exception) CF_AVAILABLE(10_12, 10_0);

// ------------- Array Buffer functions -------------

/*!
 @function
 @abstract                 Creates a JavaScript Array Buffer object from an existing pointer.
 @param ctx                The execution context to use.
 @param bytes              A pointer to the byte buffer to be used as the backing store of the Typed Array object.
 @param byteLength         The number of bytes pointed to by the parameter bytes.
 @param bytesDeallocator   The allocator to use to deallocate the external buffer when the Typed Array data object is deallocated.
 @param deallocatorContext A pointer to pass back to the deallocator.
 @param exception          A pointer to a JSValueRef in which to store an exception, if any. Pass NULL if you do not care to store an exception.
 @result                   A JSObjectRef Array Buffer whose backing store is the same as the one pointed to by bytes or NULL if there was an error.
 @discussion               If an exception is thrown during this function the bytesDeallocator will always be called.
 */
JS_EXPORT JSObjectRef JSObjectMakeArrayBufferWithBytesNoCopy(JSContextRef ctx, void* bytes, size_t byteLength, JSTypedArrayBytesDeallocator bytesDeallocator, void* deallocatorContext, JSValueRef* exception) CF_AVAILABLE(10_12, 10_0);

/*!
 @function
 @abstract         Returns a pointer to the data buffer that serves as the backing store for a JavaScript Typed Array object.
 @param object     The Array Buffer object whose internal backing store pointer to return.
 @param exception  A pointer to a JSValueRef in which to store an exception, if any. Pass NULL if you do not care to store an exception.
 @result           A pointer to the raw data buffer that serves as object's backing store or NULL if object is not an Array Buffer object.
 @discussion       The pointer returned by this function is temporary and is not guaranteed to remain valid across JavaScriptCore API calls.
 */
JS_EXPORT void* JSObjectGetArrayBufferBytesPtr(JSContextRef ctx, JSObjectRef object, JSValueRef* exception) CF_AVAILABLE(10_12, 10_0);

/*!
 @function
 @abstract         Returns the number of bytes in a JavaScript data object.
 @param ctx        The execution context to use.
 @param object     The JS Arary Buffer object whose length in bytes to return.
 @param exception  A pointer to a JSValueRef in which to store an exception, if any. Pass NULL if you do not care to store an exception.
 @result           The number of bytes stored in the data object.
 */
JS_EXPORT size_t JSObjectGetArrayBufferByteLength(JSContextRef ctx, JSObjectRef object, JSValueRef* exception) CF_AVAILABLE(10_12, 10_0);

#ifdef __cplusplus
}
#endif

#endif /* JSTypedArray_h */
