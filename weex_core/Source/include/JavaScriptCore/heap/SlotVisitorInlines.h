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

#include "SlotVisitor.h"
#include "Weak.h"
#include "WeakInlines.h"

namespace JSC {

ALWAYS_INLINE void SlotVisitor::appendUnbarriered(JSValue* slot, size_t count)
{
    for (size_t i = count; i--;)
        appendUnbarriered(slot[i]);
}

ALWAYS_INLINE void SlotVisitor::appendUnbarriered(JSCell* cell)
{
    // This needs to be written in a very specific way to ensure that it gets inlined
    // properly. In particular, it appears that using templates here breaks ALWAYS_INLINE.
    
    if (!cell)
        return;
    
    Dependency dependency;
    if (UNLIKELY(cell->isLargeAllocation())) {
        dependency = nullDependency();
        if (LIKELY(cell->largeAllocation().isMarked())) {
            if (LIKELY(!m_heapSnapshotBuilder))
                return;
        }
    } else {
        MarkedBlock& block = cell->markedBlock();
        dependency = block.aboutToMark(m_markingVersion);
        if (LIKELY(block.isMarked(cell, dependency))) {
            if (LIKELY(!m_heapSnapshotBuilder))
                return;
        }
    }
    
    appendSlow(cell, dependency);
}

ALWAYS_INLINE void SlotVisitor::appendUnbarriered(JSValue value)
{
    if (value.isCell())
        appendUnbarriered(value.asCell());
}

ALWAYS_INLINE void SlotVisitor::appendHidden(JSValue value)
{
    if (value.isCell())
        appendHidden(value.asCell());
}

ALWAYS_INLINE void SlotVisitor::appendHidden(JSCell* cell)
{
    // This needs to be written in a very specific way to ensure that it gets inlined
    // properly. In particular, it appears that using templates here breaks ALWAYS_INLINE.
    
    if (!cell)
        return;
    
    Dependency dependency;
    if (UNLIKELY(cell->isLargeAllocation())) {
        dependency = nullDependency();
        if (LIKELY(cell->largeAllocation().isMarked()))
            return;
    } else {
        MarkedBlock& block = cell->markedBlock();
        dependency = block.aboutToMark(m_markingVersion);
        if (LIKELY(block.isMarked(cell, dependency)))
            return;
    }
    
    appendHiddenSlow(cell, dependency);
}

template<typename T>
ALWAYS_INLINE void SlotVisitor::append(const Weak<T>& weak)
{
    appendUnbarriered(weak.get());
}

template<typename T>
ALWAYS_INLINE void SlotVisitor::append(const WriteBarrierBase<T>& slot)
{
    appendUnbarriered(slot.get());
}

template<typename T>
ALWAYS_INLINE void SlotVisitor::appendHidden(const WriteBarrierBase<T>& slot)
{
    appendHidden(slot.get());
}

template<typename Iterator>
ALWAYS_INLINE void SlotVisitor::append(Iterator begin, Iterator end)
{
    for (auto it = begin; it != end; ++it)
        append(*it);
}

ALWAYS_INLINE void SlotVisitor::appendValues(const WriteBarrierBase<Unknown>* barriers, size_t count)
{
    for (size_t i = 0; i < count; ++i)
        append(barriers[i]);
}

ALWAYS_INLINE void SlotVisitor::appendValuesHidden(const WriteBarrierBase<Unknown>* barriers, size_t count)
{
    for (size_t i = 0; i < count; ++i)
        appendHidden(barriers[i]);
}

inline void SlotVisitor::reportExtraMemoryVisited(size_t size)
{
    if (m_isFirstVisit) {
        heap()->reportExtraMemoryVisited(size);
        m_nonCellVisitCount += size;
    }
}

#if ENABLE(RESOURCE_USAGE)
inline void SlotVisitor::reportExternalMemoryVisited(size_t size)
{
    if (m_isFirstVisit)
        heap()->reportExternalMemoryVisited(size);
}
#endif

inline Heap* SlotVisitor::heap() const
{
    return &m_heap;
}

inline VM& SlotVisitor::vm()
{
    return *m_heap.m_vm;
}

inline const VM& SlotVisitor::vm() const
{
    return *m_heap.m_vm;
}

template<typename Func>
IterationStatus SlotVisitor::forEachMarkStack(const Func& func)
{
    if (func(m_collectorStack) == IterationStatus::Done)
        return IterationStatus::Done;
    if (func(m_mutatorStack) == IterationStatus::Done)
        return IterationStatus::Done;
    return IterationStatus::Continue;
}

} // namespace JSC
