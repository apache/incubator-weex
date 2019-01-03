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

#include "CellState.h"
#include "HandleTypes.h"
#include "IterationStatus.h"
#include "MarkStack.h"
#include "OpaqueRootSet.h"
#include "VisitRaceKey.h"
#include <wtf/MonotonicTime.h>

namespace JSC {

class ConservativeRoots;
class GCThreadSharedData;
class Heap;
class HeapCell;
class HeapSnapshotBuilder;
class MarkedBlock;
class UnconditionalFinalizer;
template<typename T> class Weak;
class WeakReferenceHarvester;
template<typename T> class WriteBarrierBase;

typedef uint32_t HeapVersion;

class SlotVisitor {
    WTF_MAKE_NONCOPYABLE(SlotVisitor);
    WTF_MAKE_FAST_ALLOCATED;

    friend class SetCurrentCellScope;
    friend class Heap;

public:
    SlotVisitor(Heap&, CString codeName);
    ~SlotVisitor();

    MarkStackArray& collectorMarkStack() { return m_collectorStack; }
    MarkStackArray& mutatorMarkStack() { return m_mutatorStack; }
    const MarkStackArray& collectorMarkStack() const { return m_collectorStack; }
    const MarkStackArray& mutatorMarkStack() const { return m_mutatorStack; }
    
    VM& vm();
    const VM& vm() const;
    Heap* heap() const;

    void append(ConservativeRoots&);
    
    template<typename T> void append(const WriteBarrierBase<T>&);
    template<typename T> void appendHidden(const WriteBarrierBase<T>&);
    template<typename Iterator> void append(Iterator begin , Iterator end);
    void appendValues(const WriteBarrierBase<Unknown>*, size_t count);
    void appendValuesHidden(const WriteBarrierBase<Unknown>*, size_t count);
    
    // These don't require you to prove that you have a WriteBarrier<>. That makes sense
    // for:
    //
    // - roots.
    // - sophisticated data structures that barrier through other means (like DFG::Plan and
    //   friends).
    //
    // If you are not a root and you don't know what kind of barrier you have, then you
    // shouldn't call these methods.
    void appendUnbarriered(JSValue);
    void appendUnbarriered(JSValue*, size_t);
    void appendUnbarriered(JSCell*);
    
    template<typename T>
    void append(const Weak<T>& weak);
    
    JS_EXPORT_PRIVATE void addOpaqueRoot(void*);
    
    JS_EXPORT_PRIVATE bool containsOpaqueRoot(void*) const;
    TriState containsOpaqueRootTriState(void*) const;

    bool isEmpty() { return m_collectorStack.isEmpty() && m_mutatorStack.isEmpty(); }

    void didStartMarking();
    void reset();
    void clearMarkStacks();

    size_t bytesVisited() const { return m_bytesVisited; }
    size_t visitCount() const { return m_visitCount; }
    
    void addToVisitCount(size_t value) { m_visitCount += value; }

    void donate();
    void drain(MonotonicTime timeout = MonotonicTime::infinity());
    void donateAndDrain(MonotonicTime timeout = MonotonicTime::infinity());
    
    enum SharedDrainMode { SlaveDrain, MasterDrain };
    enum class SharedDrainResult { Done, TimedOut };
    SharedDrainResult drainFromShared(SharedDrainMode, MonotonicTime timeout = MonotonicTime::infinity());

    SharedDrainResult drainInParallel(MonotonicTime timeout = MonotonicTime::infinity());
    SharedDrainResult drainInParallelPassively(MonotonicTime timeout = MonotonicTime::infinity());

    // Attempts to perform an increment of draining that involves only walking `bytes` worth of data. This
    // is likely to accidentally walk more or less than that. It will usually mark more than bytes. It may
    // mark less than bytes if we're reaching termination or if the global worklist is empty (which may in
    // rare cases happen temporarily even if we're not reaching termination).
    size_t performIncrementOfDraining(size_t bytes);
    
    JS_EXPORT_PRIVATE void mergeIfNecessary();

    // This informs the GC about auxiliary of some size that we are keeping alive. If you don't do
    // this then the space will be freed at end of GC.
    void markAuxiliary(const void* base);

    void reportExtraMemoryVisited(size_t);
#if ENABLE(RESOURCE_USAGE)
    void reportExternalMemoryVisited(size_t);
#endif
    
    void addWeakReferenceHarvester(WeakReferenceHarvester*);
    void addUnconditionalFinalizer(UnconditionalFinalizer*);

    void dump(PrintStream&) const;

    bool isBuildingHeapSnapshot() const { return !!m_heapSnapshotBuilder; }
    
    HeapVersion markingVersion() const { return m_markingVersion; }

    bool mutatorIsStopped() const { return m_mutatorIsStopped; }
    
    Lock& rightToRun() { return m_rightToRun; }
    
    void updateMutatorIsStopped(const AbstractLocker&);
    void updateMutatorIsStopped();
    
    bool hasAcknowledgedThatTheMutatorIsResumed() const;
    bool mutatorIsStoppedIsUpToDate() const;
    
    void optimizeForStoppedMutator();
    
    void didRace(const VisitRaceKey&);
    void didRace(JSCell* cell, const char* reason) { didRace(VisitRaceKey(cell, reason)); }
    
    void visitAsConstraint(const JSCell*);
    
    bool didReachTermination();
    
    void setIgnoreNewOpaqueRoots(bool value) { m_ignoreNewOpaqueRoots = value; }

    void donateAll();
    
    const char* codeName() const { return m_codeName.data(); }

private:
    friend class ParallelModeEnabler;
    
    void appendJSCellOrAuxiliary(HeapCell*);
    void appendHidden(JSValue);
    void appendHidden(JSCell*);

    JS_EXPORT_PRIVATE void appendSlow(JSCell*, Dependency);
    JS_EXPORT_PRIVATE void appendHiddenSlow(JSCell*, Dependency);
    void appendHiddenSlowImpl(JSCell*, Dependency);
    
    template<typename ContainerType>
    void setMarkedAndAppendToMarkStack(ContainerType&, JSCell*, Dependency);
    
    void appendToMarkStack(JSCell*);
    
    template<typename ContainerType>
    void appendToMarkStack(ContainerType&, JSCell*);
    
    void appendToMutatorMarkStack(const JSCell*);
    
    void noteLiveAuxiliaryCell(HeapCell*);
    
    void mergeOpaqueRoots();

    void mergeOpaqueRootsIfProfitable();

    void visitChildren(const JSCell*);
    
    void donateKnownParallel();
    void donateKnownParallel(MarkStackArray& from, MarkStackArray& to);

    void donateAll(const AbstractLocker&);

    bool hasWork(const AbstractLocker&);
    bool didReachTermination(const AbstractLocker&);

    template<typename Func>
    IterationStatus forEachMarkStack(const Func&);

    MarkStackArray& correspondingGlobalStack(MarkStackArray&);

    MarkStackArray m_collectorStack;
    MarkStackArray m_mutatorStack;
    OpaqueRootSet m_opaqueRoots; // Handle-owning data structures not visible to the garbage collector.
    bool m_ignoreNewOpaqueRoots { false }; // Useful as a debugging mode.
    
    size_t m_bytesVisited;
    size_t m_visitCount;
    size_t m_nonCellVisitCount { 0 }; // Used for incremental draining, ignored otherwise.
    bool m_isInParallelMode;

    HeapVersion m_markingVersion;
    
    Heap& m_heap;

    HeapSnapshotBuilder* m_heapSnapshotBuilder { nullptr };
    JSCell* m_currentCell { nullptr };
    bool m_isFirstVisit { false };
    bool m_mutatorIsStopped { false };
    bool m_canOptimizeForStoppedMutator { false };
    Lock m_rightToRun;
    
    CString m_codeName;
    
public:
#if !ASSERT_DISABLED
    bool m_isCheckingForDefaultMarkViolation;
    bool m_isDraining;
#endif
};

class ParallelModeEnabler {
public:
    ParallelModeEnabler(SlotVisitor& stack)
        : m_stack(stack)
    {
        ASSERT(!m_stack.m_isInParallelMode);
        m_stack.m_isInParallelMode = true;
    }
    
    ~ParallelModeEnabler()
    {
        ASSERT(m_stack.m_isInParallelMode);
        m_stack.m_isInParallelMode = false;
    }
    
private:
    SlotVisitor& m_stack;
};

} // namespace JSC
