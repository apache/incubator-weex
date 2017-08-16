/*
 * Copyright (C) 2011-2016 Apple Inc. All rights reserved.
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
#include "SparseArrayValueMap.h"

#include "ClassInfo.h"
#include "GetterSetter.h"
#include "JSObject.h"
#include "JSCInlines.h"
#include "PropertySlot.h"
#include "SlotVisitor.h"
#include "Structure.h"
#include "TypeError.h"

namespace JSC {

const ClassInfo SparseArrayValueMap::s_info = { "SparseArrayValueMap", 0, 0, CREATE_METHOD_TABLE(SparseArrayValueMap) };

SparseArrayValueMap::SparseArrayValueMap(VM& vm)
    : Base(vm, vm.sparseArrayValueMapStructure.get())
    , m_flags(Normal)
    , m_reportedCapacity(0)
{
}

SparseArrayValueMap::~SparseArrayValueMap()
{
}

void SparseArrayValueMap::finishCreation(VM& vm)
{
    Base::finishCreation(vm);
}

SparseArrayValueMap* SparseArrayValueMap::create(VM& vm)
{
    SparseArrayValueMap* result = new (NotNull, allocateCell<SparseArrayValueMap>(vm.heap)) SparseArrayValueMap(vm);
    result->finishCreation(vm);
    return result;
}

void SparseArrayValueMap::destroy(JSCell* cell)
{
    static_cast<SparseArrayValueMap*>(cell)->SparseArrayValueMap::~SparseArrayValueMap();
}

Structure* SparseArrayValueMap::createStructure(VM& vm, JSGlobalObject* globalObject, JSValue prototype)
{
    return Structure::create(vm, globalObject, prototype, TypeInfo(CellType, StructureFlags), info());
}

SparseArrayValueMap::AddResult SparseArrayValueMap::add(JSObject* array, unsigned i)
{
    AddResult result;
    size_t capacity;
    {
        auto locker = holdLock(*this);
        SparseArrayEntry entry;
        entry.setWithoutWriteBarrier(jsUndefined());
        
        result = m_map.add(i, entry);
        capacity = m_map.capacity();
    }
    if (capacity != m_reportedCapacity) {
        // FIXME: Adopt reportExtraMemoryVisited, and switch to reportExtraMemoryAllocated.
        // https://bugs.webkit.org/show_bug.cgi?id=142595
        Heap::heap(array)->deprecatedReportExtraMemory((capacity - m_reportedCapacity) * (sizeof(unsigned) + sizeof(WriteBarrier<Unknown>)));
        m_reportedCapacity = capacity;
    }
    return result;
}

void SparseArrayValueMap::remove(iterator it)
{
    auto locker = holdLock(*this);
    m_map.remove(it);
}

void SparseArrayValueMap::remove(unsigned i)
{
    auto locker = holdLock(*this);
    m_map.remove(i);
}

bool SparseArrayValueMap::putEntry(ExecState* exec, JSObject* array, unsigned i, JSValue value, bool shouldThrow)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);
    ASSERT(value);
    
    AddResult result = add(array, i);
    SparseArrayEntry& entry = result.iterator->value;

    // To save a separate find & add, we first always add to the sparse map.
    // In the uncommon case that this is a new property, and the array is not
    // extensible, this is not the right thing to have done - so remove again.
    if (result.isNewEntry && !array->isStructureExtensible()) {
        remove(result.iterator);
        return typeError(exec, scope, shouldThrow, ASCIILiteral(ReadonlyPropertyWriteError));
    }
    
    scope.release();
    return entry.put(exec, array, this, value, shouldThrow);
}

bool SparseArrayValueMap::putDirect(ExecState* exec, JSObject* array, unsigned i, JSValue value, unsigned attributes, PutDirectIndexMode mode)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);
    ASSERT(value);
    
    bool shouldThrow = (mode == PutDirectIndexShouldThrow);

    AddResult result = add(array, i);
    SparseArrayEntry& entry = result.iterator->value;

    // To save a separate find & add, we first always add to the sparse map.
    // In the uncommon case that this is a new property, and the array is not
    // extensible, this is not the right thing to have done - so remove again.
    if (mode != PutDirectIndexLikePutDirect && result.isNewEntry && !array->isStructureExtensible()) {
        remove(result.iterator);
        return typeError(exec, scope, shouldThrow, ASCIILiteral(NonExtensibleObjectPropertyDefineError));
    }

    if (entry.attributes & ReadOnly)
        return typeError(exec, scope, shouldThrow, ASCIILiteral(ReadonlyPropertyWriteError));

    entry.attributes = attributes;
    entry.set(vm, this, value);
    return true;
}

void SparseArrayEntry::get(JSObject* thisObject, PropertySlot& slot) const
{
    JSValue value = Base::get();
    ASSERT(value);

    if (LIKELY(!value.isGetterSetter())) {
        slot.setValue(thisObject, attributes, value);
        return;
    }

    slot.setGetterSlot(thisObject, attributes, jsCast<GetterSetter*>(value));
}

void SparseArrayEntry::get(PropertyDescriptor& descriptor) const
{
    descriptor.setDescriptor(Base::get(), attributes);
}

bool SparseArrayEntry::put(ExecState* exec, JSValue thisValue, SparseArrayValueMap* map, JSValue value, bool shouldThrow)
{
    VM& vm = exec->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    if (!(attributes & Accessor)) {
        if (attributes & ReadOnly)
            return typeError(exec, scope, shouldThrow, ASCIILiteral(ReadonlyPropertyWriteError));

        set(vm, map, value);
        return true;
    }

    scope.release();
    return callSetter(exec, thisValue, Base::get(), value, shouldThrow ? StrictMode : NotStrictMode);
}

JSValue SparseArrayEntry::getNonSparseMode() const
{
    ASSERT(!attributes);
    return Base::get();
}

void SparseArrayValueMap::visitChildren(JSCell* thisObject, SlotVisitor& visitor)
{
    Base::visitChildren(thisObject, visitor);
    
    SparseArrayValueMap* thisMap = jsCast<SparseArrayValueMap*>(thisObject);
    auto locker = holdLock(*thisMap);
    iterator end = thisMap->m_map.end();
    for (iterator it = thisMap->m_map.begin(); it != end; ++it)
        visitor.append(it->value);
}

} // namespace JSC

