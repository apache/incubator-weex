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
#ifndef JSContextRefPrivate_h
#define JSContextRefPrivate_h

#include <JavaScriptCore/JSObjectRef.h>
#include <JavaScriptCore/JSValueRef.h>
#include <JavaScriptCore/WebKitAvailability.h>

#ifndef __cplusplus
#include <stdbool.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*!
@function
@abstract Gets a Backtrace for the existing context
@param ctx The JSContext whose backtrace you want to get
@result A string containing the backtrace
*/
JS_EXPORT JSStringRef JSContextCreateBacktrace(JSContextRef ctx, unsigned maxStackSize) CF_AVAILABLE(10_6, 7_0);
    

/*! 
@typedef JSShouldTerminateCallback
@abstract The callback invoked when script execution has exceeded the allowed
 time limit previously specified via JSContextGroupSetExecutionTimeLimit.
@param ctx The execution context to use.
@param context User specified context data previously passed to
 JSContextGroupSetExecutionTimeLimit.
@discussion If you named your function Callback, you would declare it like this:

 bool Callback(JSContextRef ctx, void* context);

 If you return true, the timed out script will terminate.
 If you return false, the script will run for another period of the allowed
 time limit specified via JSContextGroupSetExecutionTimeLimit.

 Within this callback function, you may call JSContextGroupSetExecutionTimeLimit
 to set a new time limit, or JSContextGroupClearExecutionTimeLimit to cancel the
 timeout.
*/
typedef bool
(*JSShouldTerminateCallback) (JSContextRef ctx, void* context);

/*!
@function
@abstract Sets the script execution time limit.
@param group The JavaScript context group that this time limit applies to.
@param limit The time limit of allowed script execution time in seconds.
@param callback The callback function that will be invoked when the time limit
 has been reached. This will give you a chance to decide if you want to
 terminate the script or not. If you pass a NULL callback, the script will be
 terminated unconditionally when the time limit has been reached.
@param context User data that you can provide to be passed back to you
 in your callback.

 In order to guarantee that the execution time limit will take effect, you will
 need to call JSContextGroupSetExecutionTimeLimit before you start executing
 any scripts.
*/
JS_EXPORT void JSContextGroupSetExecutionTimeLimit(JSContextGroupRef group, double limit, JSShouldTerminateCallback callback, void* context) CF_AVAILABLE(10_6, 7_0);

/*!
@function
@abstract Clears the script execution time limit.
@param group The JavaScript context group that the time limit is cleared on.
*/
JS_EXPORT void JSContextGroupClearExecutionTimeLimit(JSContextGroupRef group) CF_AVAILABLE(10_6, 7_0);

/*!
@function
@abstract Gets a whether or not remote inspection is enabled on the context.
@param ctx The JSGlobalContext whose setting you want to get.
@result The value of the setting, true if remote inspection is enabled, otherwise false.
@discussion Remote inspection is true by default.
*/
JS_EXPORT bool JSGlobalContextGetRemoteInspectionEnabled(JSGlobalContextRef ctx) CF_AVAILABLE(10_10, 8_0);

/*!
@function
@abstract Sets the remote inspection setting for a context.
@param ctx The JSGlobalContext that you want to change.
@param enabled The new remote inspection enabled setting for the context.
*/
JS_EXPORT void JSGlobalContextSetRemoteInspectionEnabled(JSGlobalContextRef ctx, bool enabled) CF_AVAILABLE(10_10, 8_0);    

/*!
@function
@abstract Gets the include native call stack when reporting exceptions setting for a context.
@param ctx The JSGlobalContext whose setting you want to get.
@result The value of the setting, true if remote inspection is enabled, otherwise false.
@discussion This setting is true by default.
*/
JS_EXPORT bool JSGlobalContextGetIncludesNativeCallStackWhenReportingExceptions(JSGlobalContextRef ctx) CF_AVAILABLE(10_10, 8_0);

/*!
@function
@abstract Sets the include native call stack when reporting exceptions setting for a context.
@param ctx The JSGlobalContext that you want to change.
@param includesNativeCallStack The new value of the setting for the context.
*/
JS_EXPORT void JSGlobalContextSetIncludesNativeCallStackWhenReportingExceptions(JSGlobalContextRef ctx, bool includesNativeCallStack) CF_AVAILABLE(10_10, 8_0);

#ifdef __cplusplus
}
#endif

#endif /* JSContextRefPrivate_h */
