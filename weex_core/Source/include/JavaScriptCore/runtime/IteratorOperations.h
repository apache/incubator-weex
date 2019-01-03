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
#include "JSObject.h"
#include "ThrowScope.h"

namespace JSC {

JSValue iteratorNext(ExecState*, JSValue iterator, JSValue);
JSValue iteratorNext(ExecState*, JSValue iterator);
JS_EXPORT_PRIVATE JSValue iteratorValue(ExecState*, JSValue iterator);
bool iteratorComplete(ExecState*, JSValue iterator);
JS_EXPORT_PRIVATE JSValue iteratorStep(ExecState*, JSValue iterator);
JS_EXPORT_PRIVATE void iteratorClose(ExecState*, JSValue iterator);
JS_EXPORT_PRIVATE JSObject* createIteratorResultObject(ExecState*, JSValue, bool done);

Structure* createIteratorResultObjectStructure(VM&, JSGlobalObject&);

JS_EXPORT_PRIVATE bool hasIteratorMethod(ExecState&, JSValue);
JS_EXPORT_PRIVATE JSValue iteratorForIterable(ExecState*, JSValue iterable);

template <typename CallBackType>
void forEachInIterable(ExecState* exec, JSValue iterable, const CallBackType& callback)
{
    auto& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    JSValue iterator = iteratorForIterable(exec, iterable);
    RETURN_IF_EXCEPTION(scope, void());
    while (true) {
        JSValue next = iteratorStep(exec, iterator);
        if (UNLIKELY(scope.exception()) || next.isFalse())
            return;

        JSValue nextValue = iteratorValue(exec, next);
        RETURN_IF_EXCEPTION(scope, void());

        callback(vm, exec, nextValue);
        if (UNLIKELY(scope.exception())) {
            scope.release();
            iteratorClose(exec, iterator);
            return;
        }
    }
}

} // namespace JSC
