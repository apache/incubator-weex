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
#ifndef JSScriptRefPrivate_h
#define JSScriptRefPrivate_h

#include <JavaScriptCore/JSContextRef.h>
#include <JavaScriptCore/JSStringRef.h>
#include <JavaScriptCore/JSValueRef.h>

/*! @typedef JSScriptRef A JavaScript script reference. */
typedef struct OpaqueJSScript* JSScriptRef;

#ifdef __cplusplus
extern "C" {
#endif

/*!
 @function
 @abstract Creates a script reference from an ascii string, without copying or taking ownership of the string
 @param contextGroup The context group the script is to be used in.
 @param url The source url to be reported in errors and exceptions.
 @param startingLineNumber An integer value specifying the script's starting line number in the file located at sourceURL. This is only used when reporting exceptions. The value is one-based, so the first line is line 1 and invalid values are clamped to 1.
 @param source The source string.  This is required to be pure ASCII and to never be deallocated.
 @param length The length of the source string.
 @param errorMessage A pointer to a JSStringRef in which to store the parse error message if the source is not valid. Pass NULL if you do not care to store an error message.
 @param errorLine A pointer to an int in which to store the line number of a parser error. Pass NULL if you do not care to store an error line.
 @result A JSScriptRef for the provided source, or NULL if any non-ASCII character is found in source or if the source is not a valid JavaScript program. Ownership follows the Create Rule.
 @discussion Use this function to create a reusable script reference with a constant
 buffer as the backing string.  The source string must outlive the global context.
 */
JS_EXPORT JSScriptRef JSScriptCreateReferencingImmortalASCIIText(JSContextGroupRef contextGroup, JSStringRef url, int startingLineNumber, const char* source, size_t length, JSStringRef* errorMessage, int* errorLine);

/*!
 @function
 @abstract Creates a script reference from a string
 @param contextGroup The context group the script is to be used in.
 @param url The source url to be reported in errors and exceptions.
 @param startingLineNumber An integer value specifying the script's starting line number in the file located at sourceURL. This is only used when reporting exceptions. The value is one-based, so the first line is line 1 and invalid values are clamped to 1.
 @param source The source string.
 @param errorMessage A pointer to a JSStringRef in which to store the parse error message if the source is not valid. Pass NULL if you do not care to store an error message.
 @param errorLine A pointer to an int in which to store the line number of a parser error. Pass NULL if you do not care to store an error line.
 @result A JSScriptRef for the provided source, or NULL is the source is not a valid JavaScript program.  Ownership follows the Create Rule.
 */
JS_EXPORT JSScriptRef JSScriptCreateFromString(JSContextGroupRef contextGroup, JSStringRef url, int startingLineNumber, JSStringRef source, JSStringRef* errorMessage, int* errorLine);

/*!
 @function
 @abstract Retains a JavaScript script.
 @param script The script to retain.
 */
JS_EXPORT void JSScriptRetain(JSScriptRef script);

/*!
 @function
 @abstract Releases a JavaScript script.
 @param script The script to release.
 */
JS_EXPORT void JSScriptRelease(JSScriptRef script);

/*!
 @function
 @abstract Evaluates a JavaScript script.
 @param ctx The execution context to use.
 @param script The JSScript to evaluate.
 @param thisValue The value to use as "this" when evaluating the script.
 @param exception A pointer to a JSValueRef in which to store an exception, if any. Pass NULL if you do not care to store an exception.
 @result The JSValue that results from evaluating script, or NULL if an exception is thrown.
 */
JS_EXPORT JSValueRef JSScriptEvaluate(JSContextRef ctx, JSScriptRef script, JSValueRef thisValue, JSValueRef* exception);


#ifdef __cplusplus
}
#endif

#endif /* JSScriptRefPrivate_h */
