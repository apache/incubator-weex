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

#include "JSCJSValue.h"
#include <wtf/NakedPtr.h>

namespace JSC {

class ArgList;
class Exception;
class ExecState;
class FunctionExecutable;
class JSObject;
class JSScope;

enum class CallType : unsigned {
    None,
    Host,
    JS
};

typedef EncodedJSValue (JSC_HOST_CALL *NativeFunction)(ExecState*);

struct CallData {
    union {
        struct {
            NativeFunction function;
        } native;
        struct {
            FunctionExecutable* functionExecutable;
            JSScope* scope;
        } js;
    };
};

enum class ProfilingReason {
    API,
    Microtask,
    Other
};

JS_EXPORT_PRIVATE JSValue call(ExecState*, JSValue functionObject, CallType, const CallData&, JSValue thisValue, const ArgList&);
JS_EXPORT_PRIVATE JSValue call(ExecState*, JSValue functionObject, CallType, const CallData&, JSValue thisValue, const ArgList&, NakedPtr<Exception>& returnedException);

JS_EXPORT_PRIVATE JSValue profiledCall(ExecState*, ProfilingReason, JSValue functionObject, CallType, const CallData&, JSValue thisValue, const ArgList&);
JS_EXPORT_PRIVATE JSValue profiledCall(ExecState*, ProfilingReason, JSValue functionObject, CallType, const CallData&, JSValue thisValue, const ArgList&, NakedPtr<Exception>& returnedException);

} // namespace JSC
