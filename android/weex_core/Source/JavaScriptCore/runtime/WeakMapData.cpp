/*
 * Copyright (C) 2013, 2015 Apple Inc. All rights reserved.
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
#include "WeakMapData.h"

#include "ExceptionHelpers.h"
#include "JSCInlines.h"

#include <wtf/MathExtras.h>

namespace JSC {

const ClassInfo WeakMapData::s_info = { "WeakMapData", 0, 0, CREATE_METHOD_TABLE(WeakMapData) };

WeakMapData::WeakMapData(VM& vm)
    : Base(vm, vm.weakMapDataStructure.get())
    , m_deadKeyCleaner(this)
{
}

void WeakMapData::finishCreation(VM& vm)
{
    Base::finishCreation(vm);
}

void WeakMapData::destroy(JSCell* cell)
{
    static_cast<WeakMapData*>(cell)->~WeakMapData();
}

size_t WeakMapData::estimatedSize(JSCell* cell)
{
    WeakMapData* thisObj = jsCast<WeakMapData*>(cell);
    return Base::estimatedSize(cell) + (thisObj->m_map.capacity() * (sizeof(JSObject*) + sizeof(WriteBarrier<Unknown>)));
}

void WeakMapData::visitChildren(JSCell* cell, SlotVisitor& visitor)
{
    Base::visitChildren(cell, visitor);
    WeakMapData* thisObj = jsCast<WeakMapData*>(cell);
    visitor.addUnconditionalFinalizer(&thisObj->m_deadKeyCleaner);
    visitor.addWeakReferenceHarvester(&thisObj->m_deadKeyCleaner);

    // Rough approximation of the external storage needed for the hashtable.
    // This isn't exact, but it is close enough, and proportional to the actual
    // external memory usage.
    visitor.reportExtraMemoryVisited(thisObj->m_map.capacity() * (sizeof(JSObject*) + sizeof(WriteBarrier<Unknown>)));
}

void WeakMapData::set(VM& vm, JSObject* key, JSValue value)
{
    // Here we force the write barrier on the key.
    auto result = m_map.add(WriteBarrier<JSObject>(vm, this, key).get(), WriteBarrier<Unknown>());
    result.iterator->value.set(vm, this, value);
}

JSValue WeakMapData::get(JSObject* key)
{
    auto iter = m_map.find(key);
    if (iter == m_map.end())
        return jsUndefined();
    return iter->value.get();
}

bool WeakMapData::remove(JSObject* key)
{
    auto iter = m_map.find(key);
    if (iter == m_map.end())
        return false;

    m_map.remove(iter);
    return true;
}

bool WeakMapData::contains(JSObject* key)
{
    return m_map.contains(key);
}

void WeakMapData::clear()
{
    m_map.clear();
}

void WeakMapData::DeadKeyCleaner::visitWeakReferences(SlotVisitor& visitor)
{
    m_liveKeyCount = 0;
    for (auto& pair : m_target->m_map) {
        if (!Heap::isMarked(pair.key))
            continue;
        m_liveKeyCount++;
        visitor.append(pair.value);
    }
    RELEASE_ASSERT(m_liveKeyCount <= m_target->m_map.size());
}

void WeakMapData::DeadKeyCleaner::finalizeUnconditionally()
{
    if (m_liveKeyCount > m_target->m_map.size() / 2) {
        RELEASE_ASSERT(m_liveKeyCount <= m_target->m_map.size());
        int deadCount = m_target->m_map.size() - m_liveKeyCount;
        if (!deadCount)
            return;
        Vector<JSObject*> deadEntries;
        deadEntries.reserveCapacity(deadCount);
        for (auto& pair : m_target->m_map) {
            if (Heap::isMarked(pair.key))
                continue;
            deadEntries.uncheckedAppend(pair.key);
        }
        for (auto& deadEntry : deadEntries)
            m_target->m_map.remove(deadEntry);
    } else {
        MapType newMap;
        for (auto& pair : m_target->m_map) {
            if (!Heap::isMarked(pair.key))
                continue;
            newMap.add(pair.key, pair.value);
        }
        m_target->m_map.swap(newMap);
    }
}

}
