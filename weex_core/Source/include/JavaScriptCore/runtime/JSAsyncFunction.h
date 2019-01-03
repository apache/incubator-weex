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

#include "JSFunction.h"

namespace JSC {

class JSAsyncFunction : public JSFunction {
    friend class JIT;
    friend class VM;
public:
    typedef JSFunction Base;

    const static unsigned StructureFlags = Base::StructureFlags;

    DECLARE_EXPORT_INFO;

    static JSAsyncFunction* create(VM&, FunctionExecutable*, JSScope*);
    static JSAsyncFunction* create(VM&, FunctionExecutable*, JSScope*, Structure*);
    static JSAsyncFunction* createWithInvalidatedReallocationWatchpoint(VM&, FunctionExecutable*, JSScope*);

    static size_t allocationSize(size_t inlineCapacity)
    {
        ASSERT_UNUSED(inlineCapacity, !inlineCapacity);
        return sizeof(JSAsyncFunction);
    }

    static Structure* createStructure(VM& vm, JSGlobalObject* globalObject, JSValue prototype)
    {
        ASSERT(globalObject);
        return Structure::create(vm, globalObject, prototype, TypeInfo(JSFunctionType, StructureFlags), info());
    }

private:
    JSAsyncFunction(VM&, FunctionExecutable*, JSScope*, Structure*);

    static JSAsyncFunction* createImpl(VM&, FunctionExecutable*, JSScope*, Structure*);
};

} // namespace JSC
