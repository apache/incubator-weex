/*
 * Copyright (C) 2012-2017 Apple Inc. All rights reserved.
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
