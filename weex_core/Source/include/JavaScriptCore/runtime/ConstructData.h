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

namespace JSC {

class ArgList;
class ExecState;
class FunctionExecutable;
class JSObject;
class JSScope;

enum class ConstructType : unsigned {
    None,
    Host,
    JS
};

struct ConstructData {
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

// Convenience wrapper so you don't need to deal with CallData and CallType unless you are going to use them.
JSObject* construct(ExecState*, JSValue functionObject, const ArgList&, const char* errorMessage);
JS_EXPORT_PRIVATE JSObject* construct(ExecState*, JSValue constructor, ConstructType, const ConstructData&, const ArgList&, JSValue newTarget);

ALWAYS_INLINE JSObject* construct(ExecState* exec, JSValue constructorObject, ConstructType constructType, const ConstructData& constructData, const ArgList& args)
{
    return construct(exec, constructorObject, constructType, constructData, args, constructorObject);
}

JS_EXPORT_PRIVATE JSObject* profiledConstruct(ExecState*, ProfilingReason, JSValue constructor, ConstructType, const ConstructData&, const ArgList&, JSValue newTarget);

ALWAYS_INLINE JSObject* profiledConstruct(ExecState* exec, ProfilingReason reason, JSValue constructorObject, ConstructType constructType, const ConstructData& constructData, const ArgList& args)
{
    return profiledConstruct(exec, reason, constructorObject, constructType, constructData, args, constructorObject);
}

} // namespace JSC
