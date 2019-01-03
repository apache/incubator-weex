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
#ifndef JSObjectRefPrivate_h
#define JSObjectRefPrivate_h

#include <JavaScriptCore/JSObjectRef.h>

#ifdef __cplusplus
extern "C" {
#endif

/*!
 @function
 @abstract Sets a private property on an object.  This private property cannot be accessed from within JavaScript.
 @param ctx The execution context to use.
 @param object The JSObject whose private property you want to set.
 @param propertyName A JSString containing the property's name.
 @param value A JSValue to use as the property's value.  This may be NULL.
 @result true if object can store private data, otherwise false.
 @discussion This API allows you to store JS values directly an object in a way that will be ensure that they are kept alive without exposing them to JavaScript code and without introducing the reference cycles that may occur when using JSValueProtect.

 The default object class does not allocate storage for private data. Only objects created with a non-NULL JSClass can store private properties.
 */
JS_EXPORT bool JSObjectSetPrivateProperty(JSContextRef ctx, JSObjectRef object, JSStringRef propertyName, JSValueRef value);

/*!
 @function
 @abstract Gets a private property from an object.
 @param ctx The execution context to use.
 @param object The JSObject whose private property you want to get.
 @param propertyName A JSString containing the property's name.
 @result The property's value if object has the property, otherwise NULL.
 */
JS_EXPORT JSValueRef JSObjectGetPrivateProperty(JSContextRef ctx, JSObjectRef object, JSStringRef propertyName);

/*!
 @function
 @abstract Deletes a private property from an object.
 @param ctx The execution context to use.
 @param object The JSObject whose private property you want to delete.
 @param propertyName A JSString containing the property's name.
 @result true if object can store private data, otherwise false.
 @discussion The default object class does not allocate storage for private data. Only objects created with a non-NULL JSClass can store private data.
 */
JS_EXPORT bool JSObjectDeletePrivateProperty(JSContextRef ctx, JSObjectRef object, JSStringRef propertyName);

#ifdef __cplusplus
}
#endif

#endif // JSObjectRefPrivate_h
