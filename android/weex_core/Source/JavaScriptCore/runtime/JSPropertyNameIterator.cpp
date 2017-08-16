/*
 * Copyright (C) 2015-2016 Apple, Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "JSPropertyNameIterator.h"

#include "IteratorOperations.h"
#include "JSCInlines.h"
#include "JSPropertyNameEnumerator.h"

namespace JSC {

static EncodedJSValue JSC_HOST_CALL propertyNameIteratorFuncNext(ExecState*);

const ClassInfo JSPropertyNameIterator::s_info = { "PropertyName Iterator", &Base::s_info, 0, CREATE_METHOD_TABLE(JSPropertyNameIterator) };

JSPropertyNameIterator::JSPropertyNameIterator(VM& vm, Structure* structure, JSObject* object, JSPropertyNameEnumerator* enumerator)
    : Base(vm, structure)
    , m_iteratedObject(vm, this, object)
    , m_propertyNameEnumerator(vm, this, enumerator)
    , m_enumerationPhase(EnumerationPhase::IndexedNames)
    , m_cursor(0)
{
}

JSPropertyNameIterator* JSPropertyNameIterator::clone(ExecState* exec)
{
    auto iterator = JSPropertyNameIterator::create(exec, exec->jsCallee()->globalObject()->propertyNameIteratorStructure(), m_iteratedObject.get(), m_propertyNameEnumerator.get());
    iterator->m_enumerationPhase = m_enumerationPhase;
    iterator->m_cursor = m_cursor;
    return iterator;
}

JSPropertyNameIterator* JSPropertyNameIterator::create(ExecState* exec, Structure* structure, JSObject* iteratedObject)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);
    JSPropertyNameEnumerator* enumerator = propertyNameEnumerator(exec, iteratedObject);
    RETURN_IF_EXCEPTION(scope, nullptr);
    return JSPropertyNameIterator::create(exec, structure, iteratedObject, enumerator);
}

JSPropertyNameIterator* JSPropertyNameIterator::create(ExecState* exec, Structure* structure, JSObject* iteratedObject, JSPropertyNameEnumerator* enumerator)
{
    VM& vm = exec->vm();
    JSPropertyNameIterator* instance = new (NotNull, allocateCell<JSPropertyNameIterator>(vm.heap)) JSPropertyNameIterator(vm, structure, iteratedObject, enumerator);
    instance->finishCreation(vm, structure->globalObject());
    return instance;
}

void JSPropertyNameIterator::finishCreation(VM& vm, JSGlobalObject* globalObject)
{
    Base::finishCreation(vm);
    ASSERT(inherits(vm, info()));
    JSC_NATIVE_FUNCTION(vm.propertyNames->next, propertyNameIteratorFuncNext, DontEnum, 0);
}

void JSPropertyNameIterator::visitChildren(JSCell* cell, SlotVisitor& visitor)
{
    JSPropertyNameIterator* thisObject = jsCast<JSPropertyNameIterator*>(cell);
    ASSERT_GC_OBJECT_INHERITS(thisObject, info());
    Base::visitChildren(thisObject, visitor);
    visitor.append(thisObject->m_iteratedObject);
    visitor.append(thisObject->m_propertyNameEnumerator);
}

bool JSPropertyNameIterator::next(ExecState* exec, JSValue& output)
{
    if (m_enumerationPhase == EnumerationPhase::IndexedNames) {
        for (; m_cursor < m_propertyNameEnumerator->indexedLength();) {
            uint32_t index = m_cursor++;
            if (m_iteratedObject->hasProperty(exec, index)) {
                output = jsString(exec, Identifier::from(exec, index).string());
                return true;
            }
        }
        m_cursor = 0;
        m_enumerationPhase = EnumerationPhase::StructureNames;
    }

    if (m_enumerationPhase == EnumerationPhase::StructureNames) {
        for (; m_cursor < m_propertyNameEnumerator->endStructurePropertyIndex();) {
            uint32_t index = m_cursor++;
            JSString* propertyName = m_propertyNameEnumerator->propertyNameAtIndex(index);
            ASSERT(propertyName);
            if (m_iteratedObject->structure(exec->vm())->id() == m_propertyNameEnumerator->cachedStructureID()) {
                output = propertyName;
                return true;
            }

            if (m_iteratedObject->hasProperty(exec, propertyName->toIdentifier(exec))) {
                output = propertyName;
                return true;
            }
        }
        ASSERT(m_cursor >= m_propertyNameEnumerator->endStructurePropertyIndex());
        // Use the same m_cursor in the GenericNames phase.
        m_enumerationPhase = EnumerationPhase::GenericNames;
    }

    if (m_enumerationPhase == EnumerationPhase::GenericNames) {
        for (; m_cursor < m_propertyNameEnumerator->endGenericPropertyIndex();) {
            uint32_t index = m_cursor++;
            JSString* propertyName = m_propertyNameEnumerator->propertyNameAtIndex(index);
            ASSERT(propertyName);
            if (m_iteratedObject->hasProperty(exec, propertyName->toIdentifier(exec))) {
                output = propertyName;
                return true;
            }
        }
        m_enumerationPhase = EnumerationPhase::Done;
    }

    return false;
}

// ------------------------------ PropertyNameIterator Functions ----------------------------

EncodedJSValue JSC_HOST_CALL propertyNameIteratorFuncNext(ExecState* exec)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    JSPropertyNameIterator* iterator = jsDynamicCast<JSPropertyNameIterator*>(vm, exec->thisValue());
    if (!iterator)
        return JSValue::encode(throwTypeError(exec, scope, ASCIILiteral("Cannot call PropertyNameIterator.next() on a non-PropertyNameIterator object")));

    JSValue result;
    if (iterator->next(exec, result))
        return JSValue::encode(createIteratorResultObject(exec, result, false));
    return JSValue::encode(createIteratorResultObject(exec, jsUndefined(), true));
}

} // namespace JSC
