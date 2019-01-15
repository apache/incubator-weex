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

#include "GCDeferralContext.h"
#include "Heap.h"
#include "HeapCellInlines.h"
#include "IndexingHeader.h"
#include "JSCallee.h"
#include "JSCell.h"
#include "Structure.h"
#include <type_traits>
#include <wtf/Assertions.h>
#include <wtf/MainThread.h>
#include <wtf/RandomNumber.h>

namespace JSC {

ALWAYS_INLINE VM* Heap::vm() const
{
    return bitwise_cast<VM*>(bitwise_cast<uintptr_t>(this) - OBJECT_OFFSETOF(VM, heap));
}

ALWAYS_INLINE Heap* Heap::heap(const HeapCell* cell)
{
    return cell->heap();
}

inline Heap* Heap::heap(const JSValue v)
{
    if (!v.isCell())
        return 0;
    return heap(v.asCell());
}

inline bool Heap::hasHeapAccess() const
{
    return m_worldState.load() & hasAccessBit;
}

inline bool Heap::collectorBelievesThatTheWorldIsStopped() const
{
    return m_collectorBelievesThatTheWorldIsStopped;
}

ALWAYS_INLINE bool Heap::isMarked(const void* rawCell)
{
    ASSERT(mayBeGCThread() != GCThreadType::Helper);
    HeapCell* cell = bitwise_cast<HeapCell*>(rawCell);
    if (cell->isLargeAllocation())
        return cell->largeAllocation().isMarked();
    MarkedBlock& block = cell->markedBlock();
    return block.isMarked(
        block.vm()->heap.objectSpace().markingVersion(), cell);
}

ALWAYS_INLINE bool Heap::isMarkedConcurrently(const void* rawCell)
{
    HeapCell* cell = bitwise_cast<HeapCell*>(rawCell);
    if (cell->isLargeAllocation())
        return cell->largeAllocation().isMarked();
    MarkedBlock& block = cell->markedBlock();
    return block.isMarkedConcurrently(
        block.vm()->heap.objectSpace().markingVersion(), cell);
}

ALWAYS_INLINE bool Heap::testAndSetMarked(HeapVersion markingVersion, const void* rawCell)
{
    HeapCell* cell = bitwise_cast<HeapCell*>(rawCell);
    if (cell->isLargeAllocation())
        return cell->largeAllocation().testAndSetMarked();
    MarkedBlock& block = cell->markedBlock();
    Dependency dependency = block.aboutToMark(markingVersion);
    return block.testAndSetMarked(cell, dependency);
}

ALWAYS_INLINE size_t Heap::cellSize(const void* rawCell)
{
    return bitwise_cast<HeapCell*>(rawCell)->cellSize();
}

inline void Heap::writeBarrier(const JSCell* from, JSValue to)
{
#if ENABLE(WRITE_BARRIER_PROFILING)
    WriteBarrierCounters::countWriteBarrier();
#endif
    if (!to.isCell())
        return;
    writeBarrier(from, to.asCell());
}

inline void Heap::writeBarrier(const JSCell* from, JSCell* to)
{
#if ENABLE(WRITE_BARRIER_PROFILING)
    WriteBarrierCounters::countWriteBarrier();
#endif
    if (!from)
        return;
    if (!isWithinThreshold(from->cellState(), barrierThreshold()))
        return;
    if (LIKELY(!to))
        return;
    writeBarrierSlowPath(from);
}

inline void Heap::writeBarrier(const JSCell* from)
{
    ASSERT_GC_OBJECT_LOOKS_VALID(const_cast<JSCell*>(from));
    if (!from)
        return;
    if (UNLIKELY(isWithinThreshold(from->cellState(), barrierThreshold())))
        writeBarrierSlowPath(from);
}

inline void Heap::writeBarrierWithoutFence(const JSCell* from)
{
    ASSERT_GC_OBJECT_LOOKS_VALID(const_cast<JSCell*>(from));
    if (!from)
        return;
    if (UNLIKELY(isWithinThreshold(from->cellState(), blackThreshold)))
        addToRememberedSet(from);
}

inline void Heap::mutatorFence()
{
    if (isX86() || UNLIKELY(mutatorShouldBeFenced()))
        WTF::storeStoreFence();
}

template<typename Functor> inline void Heap::forEachCodeBlock(const Functor& func)
{
    forEachCodeBlockImpl(scopedLambdaRef<bool(CodeBlock*)>(func));
}

template<typename Functor> inline void Heap::forEachCodeBlockIgnoringJITPlans(const AbstractLocker& codeBlockSetLocker, const Functor& func)
{
    forEachCodeBlockIgnoringJITPlansImpl(codeBlockSetLocker, scopedLambdaRef<bool(CodeBlock*)>(func));
}

template<typename Functor> inline void Heap::forEachProtectedCell(const Functor& functor)
{
    for (auto& pair : m_protectedValues)
        functor(pair.key);
    m_handleSet.forEachStrongHandle(functor, m_protectedValues);
}

#if USE(FOUNDATION)
template <typename T>
inline void Heap::releaseSoon(RetainPtr<T>&& object)
{
    m_delayedReleaseObjects.append(WTFMove(object));
}
#endif

inline void Heap::incrementDeferralDepth()
{
    ASSERT(!mayBeGCThread() || m_collectorBelievesThatTheWorldIsStopped);
    m_deferralDepth++;
}

inline void Heap::decrementDeferralDepth()
{
    ASSERT(!mayBeGCThread() || m_collectorBelievesThatTheWorldIsStopped);
    m_deferralDepth--;
}

inline void Heap::decrementDeferralDepthAndGCIfNeeded()
{
    ASSERT(!mayBeGCThread() || m_collectorBelievesThatTheWorldIsStopped);
    m_deferralDepth--;
    
    if (UNLIKELY(m_didDeferGCWork)) {
        decrementDeferralDepthAndGCIfNeededSlow();
        
        // Here are the possible relationships between m_deferralDepth and m_didDeferGCWork.
        // Note that prior to the call to decrementDeferralDepthAndGCIfNeededSlow,
        // m_didDeferGCWork had to have been true. Now it can be either false or true. There is
        // nothing we can reliably assert.
        //
        // Possible arrangements of m_didDeferGCWork and !!m_deferralDepth:
        //
        // Both false: We popped out of all DeferGCs and we did whatever work was deferred.
        //
        // Only m_didDeferGCWork is true: We stopped for GC and the GC did DeferGC. This is
        // possible because of how we handle the baseline JIT's worklist. It's also perfectly
        // safe because it only protects reportExtraMemory. We can just ignore this.
        //
        // Only !!m_deferralDepth is true: m_didDeferGCWork had been set spuriously. It is only
        // cleared by decrementDeferralDepthAndGCIfNeededSlow(). So, if we had deferred work but
        // then decrementDeferralDepth()'d, then we might have the bit set even if we GC'd since
        // then.
        //
        // Both true: We're in a recursive ~DeferGC. We wanted to do something about the
        // deferred work, but were unable to.
    }
}

inline HashSet<MarkedArgumentBuffer*>& Heap::markListSet()
{
    if (!m_markListSet)
        m_markListSet = std::make_unique<HashSet<MarkedArgumentBuffer*>>();
    return *m_markListSet;
}

inline void Heap::reportExtraMemoryAllocated(size_t size)
{
    if (size > minExtraMemory) 
        reportExtraMemoryAllocatedSlowCase(size);
}

inline void Heap::deprecatedReportExtraMemory(size_t size)
{
    if (size > minExtraMemory) 
        deprecatedReportExtraMemorySlowCase(size);
}

inline void Heap::acquireAccess()
{
    if (m_worldState.compareExchangeWeak(0, hasAccessBit))
        return;
    acquireAccessSlow();
}

inline bool Heap::hasAccess() const
{
    return m_worldState.loadRelaxed() & hasAccessBit;
}

inline void Heap::releaseAccess()
{
    if (m_worldState.compareExchangeWeak(hasAccessBit, 0))
        return;
    releaseAccessSlow();
}

inline bool Heap::mayNeedToStop()
{
    return m_worldState.loadRelaxed() != hasAccessBit;
}

inline void Heap::stopIfNecessary()
{
    if (mayNeedToStop())
        stopIfNecessarySlow();
}

} // namespace JSC
