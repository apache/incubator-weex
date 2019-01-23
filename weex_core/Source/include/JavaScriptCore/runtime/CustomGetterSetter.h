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

#include "JSCell.h"
#include "PropertySlot.h"
#include "PutPropertySlot.h"
#include "Structure.h"

namespace JSC {
namespace DOMJIT {
class GetterSetter;
}

class CustomGetterSetter final : public JSCell {
public:
    typedef JSCell Base;
    static const unsigned StructureFlags = Base::StructureFlags | StructureIsImmortal;

    typedef PropertySlot::GetValueFunc CustomGetter;
    typedef PutPropertySlot::PutValueFunc CustomSetter;

    static CustomGetterSetter* create(VM& vm, CustomGetter customGetter, CustomSetter customSetter, DOMJIT::GetterSetter* domJIT = nullptr)
    {
        CustomGetterSetter* customGetterSetter = new (NotNull, allocateCell<CustomGetterSetter>(vm.heap)) CustomGetterSetter(vm, customGetter, customSetter, domJIT);
        customGetterSetter->finishCreation(vm);
        return customGetterSetter;
    }

    CustomGetterSetter::CustomGetter getter() const { return m_getter; }
    CustomGetterSetter::CustomSetter setter() const { return m_setter; }
    DOMJIT::GetterSetter* domJIT() const { return m_domJIT; }

    static Structure* createStructure(VM& vm, JSGlobalObject* globalObject, JSValue prototype)
    {
        return Structure::create(vm, globalObject, prototype, TypeInfo(CustomGetterSetterType, StructureFlags), info());
    }
        
    DECLARE_EXPORT_INFO;

private:
    CustomGetterSetter(VM& vm, CustomGetter getter, CustomSetter setter, DOMJIT::GetterSetter* domJIT)
        : JSCell(vm, vm.customGetterSetterStructure.get())
        , m_getter(getter)
        , m_setter(setter)
        , m_domJIT(domJIT)
    {
    }

    CustomGetter m_getter;
    CustomSetter m_setter;
    DOMJIT::GetterSetter* m_domJIT;
};

JS_EXPORT_PRIVATE bool callCustomSetter(ExecState*, CustomGetterSetter::CustomSetter, bool isAccessor, JSValue thisValue, JSValue);
JS_EXPORT_PRIVATE bool callCustomSetter(ExecState*, JSValue customGetterSetter, bool isAccessor, JSObject* slotBase, JSValue thisValue, JSValue);

} // namespace JSC
