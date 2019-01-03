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
#pragma once

#include "CallData.h"
#include "JSCJSValue.h"
#include <wtf/NakedPtr.h>

namespace JSC {

class Exception;
class ExecState;
class JSObject;
class ParserError;
class SourceCode;
class VM;
class JSInternalPromise;

JS_EXPORT_PRIVATE bool checkSyntax(VM&, const SourceCode&, ParserError&);
JS_EXPORT_PRIVATE bool checkSyntax(ExecState*, const SourceCode&, JSValue* exception = 0);
JS_EXPORT_PRIVATE bool checkModuleSyntax(ExecState*, const SourceCode&, ParserError&);

JS_EXPORT_PRIVATE JSValue evaluate(ExecState*, const SourceCode&, JSValue thisValue, NakedPtr<Exception>& returnedException);
inline JSValue evaluate(ExecState* exec, const SourceCode& sourceCode, JSValue thisValue = JSValue())
{
    NakedPtr<Exception> unused;
    return evaluate(exec, sourceCode, thisValue, unused);
}

JS_EXPORT_PRIVATE JSValue profiledEvaluate(ExecState*, ProfilingReason, const SourceCode&, JSValue thisValue, NakedPtr<Exception>& returnedException);
inline JSValue profiledEvaluate(ExecState* exec, ProfilingReason reason, const SourceCode& sourceCode, JSValue thisValue = JSValue())
{
    NakedPtr<Exception> unused;
    return profiledEvaluate(exec, reason, sourceCode, thisValue, unused);
}

JS_EXPORT_PRIVATE JSValue evaluateWithScopeExtension(ExecState*, const SourceCode&, JSObject* scopeExtension, NakedPtr<Exception>& returnedException);

// Load the module source and evaluate it.
JS_EXPORT_PRIVATE JSInternalPromise* loadAndEvaluateModule(ExecState*, const String& moduleName, JSValue scriptFetcher = jsUndefined());
JS_EXPORT_PRIVATE JSInternalPromise* loadAndEvaluateModule(ExecState*, const SourceCode&, JSValue scriptFetcher = jsUndefined());

// Fetch the module source, and instantiate the module record.
JS_EXPORT_PRIVATE JSInternalPromise* loadModule(ExecState*, const String& moduleName, JSValue scriptFetcher = jsUndefined());
JS_EXPORT_PRIVATE JSInternalPromise* loadModule(ExecState*, const SourceCode&, JSValue scriptFetcher = jsUndefined());

// Link and evaluate the already linked module. This function is called in a sync manner.
JS_EXPORT_PRIVATE JSValue linkAndEvaluateModule(ExecState*, const Identifier& moduleKey, JSValue scriptFetcher = jsUndefined());

JS_EXPORT_PRIVATE JSInternalPromise* importModule(ExecState*, const Identifier& moduleKey, JSValue scriptFetcher);

} // namespace JSC
