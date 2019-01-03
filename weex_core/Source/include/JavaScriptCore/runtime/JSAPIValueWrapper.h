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
#include "JSCell.h"
#include "CallFrame.h"
#include "Structure.h"

namespace JSC {

class JSAPIValueWrapper : public JSCell {
    friend JSValue jsAPIValueWrapper(ExecState*, JSValue);
public:
    typedef JSCell Base;
    static const unsigned StructureFlags = Base::StructureFlags | StructureIsImmortal;

    JSValue value() const { return m_value.get(); }

    static Structure* createStructure(VM& vm, JSGlobalObject* globalObject, JSValue prototype)
    {
        return Structure::create(vm, globalObject, prototype, TypeInfo(APIValueWrapperType, OverridesGetPropertyNames), info());
    }

    DECLARE_EXPORT_INFO;

    static JSAPIValueWrapper* create(ExecState* exec, JSValue value) 
    {
        VM& vm = exec->vm();
        JSAPIValueWrapper* wrapper = new (NotNull, allocateCell<JSAPIValueWrapper>(vm.heap)) JSAPIValueWrapper(exec);
        wrapper->finishCreation(vm, value);
        return wrapper;
    }

protected:
    void finishCreation(VM& vm, JSValue value)
    {
        Base::finishCreation(vm);
        m_value.set(vm, this, value);
        ASSERT(!value.isCell());
    }

private:
    JSAPIValueWrapper(ExecState* exec)
        : JSCell(exec->vm(), exec->vm().apiWrapperStructure.get())
    {
    }

    WriteBarrier<Unknown> m_value;
};

inline JSValue jsAPIValueWrapper(ExecState* exec, JSValue value)
{
    return JSAPIValueWrapper::create(exec, value);
}

} // namespace JSC
