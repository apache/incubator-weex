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
#ifndef JSBasePrivate_h
#define JSBasePrivate_h

#include <JavaScriptCore/JSBase.h>
#include <JavaScriptCore/WebKitAvailability.h>

#ifdef __cplusplus
extern "C" {
#endif

/*!
@function
@abstract Reports an object's non-GC memory payload to the garbage collector.
@param ctx The execution context to use.
@param size The payload's size, in bytes.
@discussion Use this function to notify the garbage collector that a GC object
owns a large non-GC memory region. Calling this function will encourage the
garbage collector to collect soon, hoping to reclaim that large non-GC memory
region.
*/
JS_EXPORT void JSReportExtraMemoryCost(JSContextRef ctx, size_t size) CF_AVAILABLE(10_6, 7_0);

JS_EXPORT void JSDisableGCTimer(void);

#ifdef __cplusplus
}
#endif

#endif /* JSBasePrivate_h */
