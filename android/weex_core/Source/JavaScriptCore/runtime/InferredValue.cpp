/*
 * Copyright (C) 2015-2016 Apple Inc. All rights reserved.
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
#include "InferredValue.h"

#include "JSCInlines.h"

namespace JSC {

const ClassInfo InferredValue::s_info = { "InferredValue", 0, 0, CREATE_METHOD_TABLE(InferredValue) };

InferredValue* InferredValue::create(VM& vm)
{
    InferredValue* result = new (NotNull, allocateCell<InferredValue>(vm.heap)) InferredValue(vm);
    result->finishCreation(vm);
    return result;
}

void InferredValue::destroy(JSCell* cell)
{
    InferredValue* inferredValue = static_cast<InferredValue*>(cell);
    inferredValue->InferredValue::~InferredValue();
}

Structure* InferredValue::createStructure(VM& vm, JSGlobalObject* globalObject, JSValue prototype)
{
    return Structure::create(vm, globalObject, prototype, TypeInfo(CellType, StructureFlags), info());
}

void InferredValue::visitChildren(JSCell* cell, SlotVisitor& visitor)
{
    InferredValue* inferredValue = jsCast<InferredValue*>(cell);
    
    auto locker = holdLock(*inferredValue);
    
    if (inferredValue->m_set.hasBeenInvalidated()) {
        if (inferredValue->m_cleanup && !inferredValue->m_cleanup->isOnList())
            inferredValue->m_cleanup = nullptr;
        return;
    }
    
    JSValue value = inferredValue->m_value.get();
    if (!value)
        return;
    if (!value.isCell())
        return;
    
    if (!inferredValue->m_cleanup)
        inferredValue->m_cleanup = std::make_unique<ValueCleanup>(inferredValue);
    visitor.addUnconditionalFinalizer(inferredValue->m_cleanup.get());
}

InferredValue::InferredValue(VM& vm)
    : Base(vm, vm.inferredValueStructure.get())
    , m_set(ClearWatchpoint)
{
}

InferredValue::~InferredValue()
{
}

void InferredValue::notifyWriteSlow(VM& vm, JSValue value, const FireDetail& detail)
{
    ASSERT(!!value);
    switch (m_set.state()) {
    case ClearWatchpoint:
        m_value.set(vm, this, value);
        m_set.startWatching();
        return;
        
    case IsWatched:
        ASSERT(!!m_value);
        if (m_value.get() == value)
            return;
        invalidate(vm, detail);
        return;
        
    case IsInvalidated:
        ASSERT_NOT_REACHED();
        return;
    }
    
    ASSERT_NOT_REACHED();
}

void InferredValue::notifyWriteSlow(VM& vm, JSValue value, const char* reason)
{
    notifyWriteSlow(vm, value, StringFireDetail(reason));
}

InferredValue::ValueCleanup::ValueCleanup(InferredValue* owner)
    : m_owner(owner)
{
}

InferredValue::ValueCleanup::~ValueCleanup()
{
}

void InferredValue::ValueCleanup::finalizeUnconditionally()
{
    JSValue value = m_owner->m_value.get();
    
    // Concurrent GC means that this could have changed since we installed the finalizer.
    if (!value || !value.isCell())
        return;
    
    if (Heap::isMarked(value.asCell()))
        return;
    
    m_owner->invalidate(*m_owner->vm(), StringFireDetail("InferredValue clean-up during GC"));
}

} // namespace JSC

