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
#ifndef JSContextRefInternal_h
#define JSContextRefInternal_h

#include "JSContextRefPrivate.h"

#if USE(CF)
#include <CoreFoundation/CFRunLoop.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if USE(CF)
/*!
@function
@abstract Gets the run loop used by the Web Inspector debugger when evaluating JavaScript in this context.
@param ctx The JSGlobalContext whose setting you want to get.
*/
JS_EXPORT CFRunLoopRef JSGlobalContextGetDebuggerRunLoop(JSGlobalContextRef ctx) CF_AVAILABLE(10_10, 8_0);

/*!
@function
@abstract Sets the run loop used by the Web Inspector debugger when evaluating JavaScript in this context.
@param ctx The JSGlobalContext that you want to change.
@param runLoop The new value of the setting for the context.
*/
JS_EXPORT void JSGlobalContextSetDebuggerRunLoop(JSGlobalContextRef ctx, CFRunLoopRef runLoop) CF_AVAILABLE(10_10, 8_0);
#endif

#ifdef __cplusplus
}
#endif

#endif // JSContextRefInternal_h
