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
#ifndef JSWeakObjectMapRefPrivate_h
#define JSWeakObjectMapRefPrivate_h

#include <JavaScriptCore/JSContextRef.h>
#include <JavaScriptCore/JSValueRef.h>

#ifdef __cplusplus
extern "C" {
#endif
    
/*! @typedef JSWeakObjectMapRef A weak map for storing JSObjectRefs */
typedef struct OpaqueJSWeakObjectMap* JSWeakObjectMapRef;

/*! 
 @typedef JSWeakMapDestroyedCallback
 @abstract The callback invoked when a JSWeakObjectMapRef is being destroyed.
 @param map The map that is being destroyed.
 @param data The private data (if any) that was associated with the map instance.
 */
typedef void (*JSWeakMapDestroyedCallback)(JSWeakObjectMapRef map, void* data);

/*!
 @function
 @abstract Creates a weak value map that can be used to reference user defined objects without preventing them from being collected.
 @param ctx The execution context to use.
 @param data A void* to set as the map's private data. Pass NULL to specify no private data.
 @param destructor A function to call when the weak map is destroyed.
 @result A JSWeakObjectMapRef bound to the given context, data and destructor.
 @discussion The JSWeakObjectMapRef can be used as a storage mechanism to hold custom JS objects without forcing those objects to
 remain live as JSValueProtect would.
 */
JS_EXPORT JSWeakObjectMapRef JSWeakObjectMapCreate(JSContextRef ctx, void* data, JSWeakMapDestroyedCallback destructor);

/*!
 @function
 @abstract Associates a JSObjectRef with the given key in a JSWeakObjectMap.
 @param ctx The execution context to use.
 @param map The map to operate on.
 @param key The key to associate a weak reference with.
 @param object The user defined object to associate with the key.
 */
JS_EXPORT void JSWeakObjectMapSet(JSContextRef ctx, JSWeakObjectMapRef map, void* key, JSObjectRef object);

/*!
 @function
 @abstract Retrieves the JSObjectRef associated with a key.
 @param ctx The execution context to use.
 @param map The map to query.
 @param key The key to search for.
 @result Either the live object associated with the provided key, or NULL.
 */
JS_EXPORT JSObjectRef JSWeakObjectMapGet(JSContextRef ctx, JSWeakObjectMapRef map, void* key);

/*!
 @function
 @abstract Removes the entry for the given key if the key is present, otherwise it has no effect.
 @param ctx The execution context to use.
 @param map The map to use.
 @param key The key to remove.
 */
JS_EXPORT void JSWeakObjectMapRemove(JSContextRef ctx, JSWeakObjectMapRef map, void* key);

#ifdef __cplusplus
}
#endif

#endif // JSWeakObjectMapPrivate_h
