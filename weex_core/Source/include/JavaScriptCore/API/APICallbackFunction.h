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
#ifndef APICallbackFunction_h
#define APICallbackFunction_h

#include "APICast.h"
#include "Error.h"
#include "JSCallbackConstructor.h"
#include "JSLock.h"
#include <wtf/Vector.h>

namespace JSC {

struct APICallbackFunction {

template <typename T> static EncodedJSValue JSC_HOST_CALL call(ExecState*);
template <typename T> static EncodedJSValue JSC_HOST_CALL construct(ExecState*);

};

template <typename T>
EncodedJSValue JSC_HOST_CALL APICallbackFunction::call(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);
    JSContextRef execRef = toRef(exec);
    JSObjectRef functionRef = toRef(exec->jsCallee());
    JSObjectRef thisObjRef = toRef(jsCast<JSObject*>(exec->thisValue().toThis(exec, NotStrictMode)));

    int argumentCount = static_cast<int>(exec->argumentCount());
    Vector<JSValueRef, 16> arguments;
    arguments.reserveInitialCapacity(argumentCount);
    for (int i = 0; i < argumentCount; i++)
        arguments.uncheckedAppend(toRef(exec, exec->uncheckedArgument(i)));

    JSValueRef exception = 0;
    JSValueRef result;
    {
        JSLock::DropAllLocks dropAllLocks(exec);
        result = jsCast<T*>(toJS(functionRef))->functionCallback()(execRef, functionRef, thisObjRef, argumentCount, arguments.data(), &exception);
    }
    if (exception)
        throwException(exec, scope, toJS(exec, exception));

    // result must be a valid JSValue.
    if (!result)
        return JSValue::encode(jsUndefined());

    return JSValue::encode(toJS(exec, result));
}

template <typename T>
EncodedJSValue JSC_HOST_CALL APICallbackFunction::construct(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);
    JSObject* constructor = exec->jsCallee();
    JSContextRef ctx = toRef(exec);
    JSObjectRef constructorRef = toRef(constructor);

    JSObjectCallAsConstructorCallback callback = jsCast<T*>(constructor)->constructCallback();
    if (callback) {
        size_t argumentCount = exec->argumentCount();
        Vector<JSValueRef, 16> arguments;
        arguments.reserveInitialCapacity(argumentCount);
        for (size_t i = 0; i < argumentCount; ++i)
            arguments.uncheckedAppend(toRef(exec, exec->uncheckedArgument(i)));

        JSValueRef exception = 0;
        JSObjectRef result;
        {
            JSLock::DropAllLocks dropAllLocks(exec);
            result = callback(ctx, constructorRef, argumentCount, arguments.data(), &exception);
        }
        if (exception) {
            throwException(exec, scope, toJS(exec, exception));
            return JSValue::encode(toJS(exec, exception));
        }
        // result must be a valid JSValue.
        if (!result)
            return throwVMTypeError(exec, scope);
        return JSValue::encode(toJS(result));
    }
    
    return JSValue::encode(toJS(JSObjectMake(ctx, jsCast<JSCallbackConstructor*>(constructor)->classRef(), 0)));
}

} // namespace JSC

#endif // APICallbackFunction_h
