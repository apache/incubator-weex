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

#include "ExceptionScope.h"

namespace JSC {

class ExecState;
class JSObject;

#if ENABLE(EXCEPTION_SCOPE_VERIFICATION)

// If a function can throw a JS exception, it should declare a ThrowScope at the
// top of the function (as early as possible) using the DECLARE_THROW_SCOPE macro.
// Declaring a ThrowScope in a function means that the function may throw an
// exception that its caller will have to handle.

class ThrowScope : public ExceptionScope {
public:
    JS_EXPORT_PRIVATE ThrowScope(VM&, ExceptionEventLocation);
    JS_EXPORT_PRIVATE ~ThrowScope();

    ThrowScope(const ThrowScope&) = delete;
    ThrowScope(ThrowScope&&) = default;

    JS_EXPORT_PRIVATE void throwException(ExecState*, Exception*);
    JS_EXPORT_PRIVATE JSValue throwException(ExecState*, JSValue);
    JS_EXPORT_PRIVATE JSObject* throwException(ExecState*, JSObject*);

    void release() { m_isReleased = true; }

    JS_EXPORT_PRIVATE void printIfNeedCheck(const char* functionName, const char* file, unsigned line);

private:
    void simulateThrow();

    bool m_isReleased { false };
};

#define DECLARE_THROW_SCOPE(vm__) \
    JSC::ThrowScope((vm__), JSC::ExceptionEventLocation(__FUNCTION__, __FILE__, __LINE__))

#define throwScopePrintIfNeedCheck(scope__) \
    scope__.printIfNeedCheck(__FUNCTION__, __FILE__, __LINE__)

#else // not ENABLE(EXCEPTION_SCOPE_VERIFICATION)

class ThrowScope : public ExceptionScope {
public:
    ALWAYS_INLINE ThrowScope(VM& vm)
        : ExceptionScope(vm)
    { }
    ThrowScope(const ThrowScope&) = delete;
    ThrowScope(ThrowScope&&) = default;

    ALWAYS_INLINE void throwException(ExecState* exec, Exception* exception) { m_vm.throwException(exec, exception); }
    ALWAYS_INLINE JSValue throwException(ExecState* exec, JSValue value) { return m_vm.throwException(exec, value); }
    ALWAYS_INLINE JSObject* throwException(ExecState* exec, JSObject* obj) { return m_vm.throwException(exec, obj); }

    ALWAYS_INLINE void release() { }
};

#define DECLARE_THROW_SCOPE(vm__) \
    JSC::ThrowScope((vm__))

#endif // ENABLE(EXCEPTION_SCOPE_VERIFICATION)

ALWAYS_INLINE void throwException(ExecState* exec, ThrowScope& scope, Exception* exception)
{
    scope.throwException(exec, exception);
}

ALWAYS_INLINE JSValue throwException(ExecState* exec, ThrowScope& scope, JSValue value)
{
    return scope.throwException(exec, value);
}

ALWAYS_INLINE JSObject* throwException(ExecState* exec, ThrowScope& scope, JSObject* obj)
{
    return scope.throwException(exec, obj);
}

} // namespace JSC
