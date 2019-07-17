/*
 * Copyright (C) 2014 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
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
