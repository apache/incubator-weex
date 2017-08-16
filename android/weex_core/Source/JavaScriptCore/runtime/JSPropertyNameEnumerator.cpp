/*
 * Copyright (C) 2014-2016 Apple Inc. All rights reserved.
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

#include "config.h"
#include "JSPropertyNameEnumerator.h"

#include "JSCInlines.h"
#include "LockDuringMarking.h"
#include "StrongInlines.h"

namespace JSC {

const ClassInfo JSPropertyNameEnumerator::s_info = { "JSPropertyNameEnumerator", 0, 0, CREATE_METHOD_TABLE(JSPropertyNameEnumerator) };

JSPropertyNameEnumerator* JSPropertyNameEnumerator::create(VM& vm)
{
    if (!vm.emptyPropertyNameEnumerator.get()) {
        PropertyNameArray propertyNames(&vm, PropertyNameMode::Strings);
        vm.emptyPropertyNameEnumerator = Strong<JSCell>(vm, create(vm, 0, 0, 0, WTFMove(propertyNames)));
    }
    return jsCast<JSPropertyNameEnumerator*>(vm.emptyPropertyNameEnumerator.get());
}

JSPropertyNameEnumerator* JSPropertyNameEnumerator::create(VM& vm, Structure* structure, uint32_t indexedLength, uint32_t numberStructureProperties, PropertyNameArray&& propertyNames)
{
    StructureID structureID = structure ? structure->id() : 0;
    uint32_t inlineCapacity = structure ? structure->inlineCapacity() : 0;
    JSPropertyNameEnumerator* enumerator = new (NotNull, 
        allocateCell<JSPropertyNameEnumerator>(vm.heap)) JSPropertyNameEnumerator(vm, structureID, inlineCapacity);
    enumerator->finishCreation(vm, indexedLength, numberStructureProperties, propertyNames.releaseData());
    return enumerator;
}

JSPropertyNameEnumerator::JSPropertyNameEnumerator(VM& vm, StructureID structureID, uint32_t inlineCapacity)
    : JSCell(vm, vm.propertyNameEnumeratorStructure.get())
    , m_cachedStructureID(structureID)
    , m_cachedInlineCapacity(inlineCapacity)
{
}

void JSPropertyNameEnumerator::finishCreation(VM& vm, uint32_t indexedLength, uint32_t endStructurePropertyIndex, RefPtr<PropertyNameArrayData>&& identifiers)
{
    Base::finishCreation(vm);

    PropertyNameArrayData::PropertyNameVector& vector = identifiers->propertyNameVector();

    m_indexedLength = indexedLength;
    m_endStructurePropertyIndex = endStructurePropertyIndex;
    m_endGenericPropertyIndex = vector.size();

    {
        auto locker = lockDuringMarking(vm.heap, *this);
        m_propertyNames.resizeToFit(vector.size());
    }
    for (unsigned i = 0; i < vector.size(); ++i) {
        const Identifier& identifier = vector[i];
        m_propertyNames[i].set(vm, this, jsString(&vm, identifier.string()));
    }
}

void JSPropertyNameEnumerator::destroy(JSCell* cell)
{
    static_cast<JSPropertyNameEnumerator*>(cell)->JSPropertyNameEnumerator::~JSPropertyNameEnumerator();
}

void JSPropertyNameEnumerator::visitChildren(JSCell* cell, SlotVisitor& visitor)
{
    Base::visitChildren(cell, visitor);
    JSPropertyNameEnumerator* thisObject = jsCast<JSPropertyNameEnumerator*>(cell);
    auto locker = holdLock(*thisObject);
    for (auto& propertyName : thisObject->m_propertyNames)
        visitor.append(propertyName);
    visitor.append(thisObject->m_prototypeChain);
}

} // namespace JSC
