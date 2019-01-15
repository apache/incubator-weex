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

#include "FunctionExecutable.h"
#include "JSFunction.h"
#include "NativeExecutable.h"

namespace JSC {

inline JSFunction* JSFunction::createWithInvalidatedReallocationWatchpoint(
    VM& vm, FunctionExecutable* executable, JSScope* scope)
{
    ASSERT(executable->singletonFunction()->hasBeenInvalidated());
    return createImpl(vm, executable, scope, scope->globalObject(vm)->functionStructure());
}

inline JSFunction::JSFunction(VM& vm, FunctionExecutable* executable, JSScope* scope, Structure* structure)
    : Base(vm, scope, structure)
    , m_executable(vm, this, executable)
    , m_rareData()
{
}

inline FunctionExecutable* JSFunction::jsExecutable() const
{
    ASSERT(!isHostFunctionNonInline());
    return static_cast<FunctionExecutable*>(m_executable.get());
}

inline bool JSFunction::isHostFunction() const
{
    ASSERT(m_executable);
    return m_executable->isHostFunction();
}

inline Intrinsic JSFunction::intrinsic() const
{
    return executable()->intrinsic();
}

inline bool JSFunction::isBuiltinFunction() const
{
    return !isHostFunction() && jsExecutable()->isBuiltinFunction();
}

inline bool JSFunction::isHostOrBuiltinFunction() const
{
    return isHostFunction() || isBuiltinFunction();
}

inline bool JSFunction::isClassConstructorFunction() const
{
    return !isHostFunction() && jsExecutable()->isClassConstructorFunction();
}

inline NativeFunction JSFunction::nativeFunction()
{
    ASSERT(isHostFunctionNonInline());
    return static_cast<NativeExecutable*>(m_executable.get())->function();
}

inline NativeFunction JSFunction::nativeConstructor()
{
    ASSERT(isHostFunctionNonInline());
    return static_cast<NativeExecutable*>(m_executable.get())->constructor();
}

inline bool isHostFunction(JSValue value, NativeFunction nativeFunction)
{
    JSFunction* function = jsCast<JSFunction*>(getJSFunction(value));
    if (!function || !function->isHostFunction())
        return false;
    return function->nativeFunction() == nativeFunction;
}

inline bool JSFunction::hasReifiedLength() const
{
    return m_rareData ? m_rareData->hasReifiedLength() : false;
}

inline bool JSFunction::hasReifiedName() const
{
    return m_rareData ? m_rareData->hasReifiedName() : false;
}

} // namespace JSC
