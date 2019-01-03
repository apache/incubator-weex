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
#ifndef APIUtils_h
#define APIUtils_h

#include "Exception.h"
#include "JSCJSValue.h"
#include "JSGlobalObjectInspectorController.h"
#include "JSValueRef.h"

enum class ExceptionStatus {
    DidThrow,
    DidNotThrow
};

inline ExceptionStatus handleExceptionIfNeeded(JSC::ExecState* exec, JSValueRef* returnedExceptionRef)
{
    JSC::VM& vm = exec->vm();
    auto scope = DECLARE_CATCH_SCOPE(vm);
    if (UNLIKELY(scope.exception())) {
        JSC::Exception* exception = scope.exception();
        if (returnedExceptionRef)
            *returnedExceptionRef = toRef(exec, exception->value());
        scope.clearException();
#if ENABLE(REMOTE_INSPECTOR)
        exec->vmEntryGlobalObject()->inspectorController().reportAPIException(exec, exception);
#endif
        return ExceptionStatus::DidThrow;
    }
    return ExceptionStatus::DidNotThrow;
}

inline void setException(JSC::ExecState* exec, JSValueRef* returnedExceptionRef, JSC::JSValue exception)
{
    if (returnedExceptionRef)
        *returnedExceptionRef = toRef(exec, exception);
#if ENABLE(REMOTE_INSPECTOR)
    exec->vmEntryGlobalObject()->inspectorController().reportAPIException(exec, JSC::Exception::create(exec->vm(), exception));
#endif
}

#endif /* APIUtils_h */
