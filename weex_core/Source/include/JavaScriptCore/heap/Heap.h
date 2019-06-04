/*
 *  Copyright (C) 1999-2000 Harri Porten (porten@kde.org)
 *  Copyright (C) 2001 Peter Kelly (pmk@post.com)
 *  Copyright (C) 2003-2017 Apple Inc. All rights reserved.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#pragma once

#include "ArrayBuffer.h"
#include "CellState.h"
#include "CollectionScope.h"
#include "CollectorPhase.h"
#include "DeleteAllCodeEffort.h"
#include "GCConductor.h"
#include "GCIncomingRefCountedSet.h"
#include "HandleSet.h"
#include "HandleStack.h"
#include "HeapObserver.h"
#include "ListableHandler.h"
#include "MarkedBlock.h"
#include "MarkedBlockSet.h"
#include "MarkedSpace.h"
#include "MutatorState.h"
#include "Options.h"
#include "StructureIDTable.h"
#include "TinyBloomFilter.h"
#include "UnconditionalFinalizer.h"
#include "VisitRaceKey.h"
#include "WeakHandleOwner.h"
#include "WeakReferenceHarvester.h"
#include "WriteBarrierSupport.h"
#include <wtf/AutomaticThread.h>
#include <wtf/Deque.h>
#include <wtf/HashCountedSet.h>
#include <wtf/HashSet.h>
#include <wtf/ParallelHelperPool.h>

namespace JSC {

class CodeBlock;
class CodeBlockSet;
class CollectingScope;
class ConservativeRoots;
class GCDeferralContext;
class EdenGCActivityCallback;
class ExecutableBase;
class FullGCActivityCallback;
class GCActivityCallback;
class GCAwareJITStubRoutine;
class Heap;
class HeapProfiler;
class HeapVerifier;
class IncrementalSweeper;
class JITStubRoutine;
class JITStubRoutineSet;
class JSCell;
class JSValue;
class LLIntOffsetsExtractor;
class MachineThreads;
class MarkStackArray;
class MarkedAllocator;
class MarkedArgumentBuffer;
class MarkingConstraint;
class MarkingConstraintSet;
class MutatorScheduler;
class RunningScope;
class SlotVisitor;
class SpaceTimeMutatorScheduler;
class StopIfNecessaryTimer;
class SweepingScope;
class VM;
struct CurrentThreadState;

namespace DFG {
class SpeculativeJIT;
class Worklist;
}

typedef HashCountedSet<JSCell*> ProtectCountSet;
typedef HashCountedSet<const char*> TypeCountSet;

enum HeapType { SmallHeap, LargeHeap };

class HeapUtil;

class Heap {
    WTF_MAKE_NONCOPYABLE(Heap);
public:
    friend class JIT;
    friend class DFG::SpeculativeJIT;
    static Heap* heap(const JSValue); // 0 for immediate values
    static Heap* heap(const HeapCell*);

    // This constant determines how many blocks we iterate between checks of our 
    // deadline when calling Heap::isPagedOut. Decreasing it will cause us to detect 
    // overstepping our deadline more quickly, while increasing it will cause 
    // our scan to run faster. 
    static const unsigned s_timeCheckResolution = 16;

    static bool isMarked(const void*);
    static bool isMarkedConcurrently(const void*);
    static bool testAndSetMarked(HeapVersion, const void*);
    
    static size_t cellSize(const void*);

    void writeBarrier(const JSCell* from);
    void writeBarrier(const JSCell* from, JSValue to);
    void writeBarrier(const JSCell* from, JSCell* to);
    
    void writeBarrierWithoutFence(const JSCell* from);
    
    void mutatorFence();
    
    // Take this if you know that from->cellState() < barrierThreshold.
    JS_EXPORT_PRIVATE void writeBarrierSlowPath(const JSCell* from);

    Heap(VM*, HeapType);
    ~Heap();
    void lastChanceToFinalize();
    void releaseDelayedReleasedObjects();

    VM* vm() const;

    MarkedSpace& objectSpace() { return m_objectSpace; }
    MachineThreads& machineThreads() { return *m_machineThreads; }

    SlotVisitor& collectorSlotVisitor() { return *m_collectorSlotVisitor; }

    JS_EXPORT_PRIVATE GCActivityCallback* fullActivityCallback();
    JS_EXPORT_PRIVATE GCActivityCallback* edenActivityCallback();
    JS_EXPORT_PRIVATE void setGarbageCollectionTimerEnabled(bool);

    JS_EXPORT_PRIVATE IncrementalSweeper* sweeper();

    void addObserver(HeapObserver* observer) { m_observers.append(observer); }
    void removeObserver(HeapObserver* observer) { m_observers.removeFirst(observer); }

    MutatorState mutatorState() const { return m_mutatorState; }
    std::optional<CollectionScope> collectionScope() const { return m_collectionScope; }
    bool hasHeapAccess() const;
    bool collectorBelievesThatTheWorldIsStopped() const;

    // We're always busy on the collection threads. On the main thread, this returns true if we're
    // helping heap.
    JS_EXPORT_PRIVATE bool isCurrentThreadBusy();
    
    typedef void (*Finalizer)(JSCell*);
    JS_EXPORT_PRIVATE void addFinalizer(JSCell*, Finalizer);
    void addExecutable(ExecutableBase*);

    void notifyIsSafeToCollect();
    bool isSafeToCollect() const { return m_isSafeToCollect; }

    JS_EXPORT_PRIVATE bool isHeapSnapshotting() const;

    JS_EXPORT_PRIVATE void collectAllGarbageIfNotDoneRecently();
    JS_EXPORT_PRIVATE void collectAllGarbage();
    JS_EXPORT_PRIVATE void sweepSynchronously();

    bool shouldCollectHeuristic();
    
    // Queue up a collection. Returns immediately. This will not queue a collection if a collection
    // of equal or greater strength exists. Full collections are stronger than std::nullopt collections
    // and std::nullopt collections are stronger than Eden collections. std::nullopt means that the GC can
    // choose Eden or Full. This implies that if you request a GC while that GC is ongoing, nothing
    // will happen.
    JS_EXPORT_PRIVATE void collectAsync(std::optional<CollectionScope> = std::nullopt);
    
    // Queue up a collection and wait for it to complete. This won't return until you get your own
    // complete collection. For example, if there was an ongoing asynchronous collection at the time
    // you called this, then this would wait for that one to complete and then trigger your
    // collection and then return. In weird cases, there could be multiple GC requests in the backlog
    // and this will wait for that backlog before running its GC and returning.
    JS_EXPORT_PRIVATE void collectSync(std::optional<CollectionScope> = std::nullopt);
    
    void collectIfNecessaryOrDefer(GCDeferralContext* = nullptr);

    void completeAllJITPlans();
    
    // Use this API to report non-GC memory referenced by GC objects. Be sure to
    // call both of these functions: Calling only one may trigger catastropic
    // memory growth.
    void reportExtraMemoryAllocated(size_t);
    JS_EXPORT_PRIVATE void reportExtraMemoryVisited(size_t);

#if ENABLE(RESOURCE_USAGE)
    // Use this API to report the subset of extra memory that lives outside this process.
    JS_EXPORT_PRIVATE void reportExternalMemoryVisited(size_t);
    size_t externalMemorySize() { return m_externalMemorySize; }
#endif

    // Use this API to report non-GC memory if you can't use the better API above.
    void deprecatedReportExtraMemory(size_t);

    JS_EXPORT_PRIVATE void reportAbandonedObjectGraph();

    JS_EXPORT_PRIVATE void protect(JSValue);
    JS_EXPORT_PRIVATE bool unprotect(JSValue); // True when the protect count drops to 0.
    
    JS_EXPORT_PRIVATE size_t extraMemorySize(); // Non-GC memory referenced by GC objects.
    JS_EXPORT_PRIVATE size_t size();
    JS_EXPORT_PRIVATE size_t capacity();
    JS_EXPORT_PRIVATE size_t objectCount();
    JS_EXPORT_PRIVATE size_t globalObjectCount();
    JS_EXPORT_PRIVATE size_t protectedObjectCount();
    JS_EXPORT_PRIVATE size_t protectedGlobalObjectCount();
    JS_EXPORT_PRIVATE std::unique_ptr<TypeCountSet> protectedObjectTypeCounts();
    JS_EXPORT_PRIVATE std::unique_ptr<TypeCountSet> objectTypeCounts();

    HashSet<MarkedArgumentBuffer*>& markListSet();
    
    template<typename Functor> void forEachProtectedCell(const Functor&);
    template<typename Functor> void forEachCodeBlock(const Functor&);
    template<typename Functor> void forEachCodeBlockIgnoringJITPlans(const AbstractLocker& codeBlockSetLocker, const Functor&);

    HandleSet* handleSet() { return &m_handleSet; }
    HandleStack* handleStack() { return &m_handleStack; }

    void willStartIterating();
    void didFinishIterating();

    Seconds lastFullGCLength() const { return m_lastFullGCLength; }
    Seconds lastEdenGCLength() const { return m_lastEdenGCLength; }
    void increaseLastFullGCLength(Seconds amount) { m_lastFullGCLength += amount; }

    size_t sizeBeforeLastEdenCollection() const { return m_sizeBeforeLastEdenCollect; }
    size_t sizeAfterLastEdenCollection() const { return m_sizeAfterLastEdenCollect; }
    size_t sizeBeforeLastFullCollection() const { return m_sizeBeforeLastFullCollect; }
    size_t sizeAfterLastFullCollection() const { return m_sizeAfterLastFullCollect; }

    void deleteAllCodeBlocks(DeleteAllCodeEffort);
    void deleteAllUnlinkedCodeBlocks(DeleteAllCodeEffort);

    void didAllocate(size_t);
    bool isPagedOut(double deadline);
    
    const JITStubRoutineSet& jitStubRoutines() { return *m_jitStubRoutines; }
    
    void addReference(JSCell*, ArrayBuffer*);
    
    bool isDeferred() const { return !!m_deferralDepth; }

    StructureIDTable& structureIDTable() { return m_structureIDTable; }

    CodeBlockSet& codeBlockSet() { return *m_codeBlocks; }

#if USE(FOUNDATION)
    template<typename T> void releaseSoon(RetainPtr<T>&&);
#endif

    JS_EXPORT_PRIVATE void registerWeakGCMap(void* weakGCMap, std::function<void()> pruningCallback);
    JS_EXPORT_PRIVATE void unregisterWeakGCMap(void* weakGCMap);

    void addLogicallyEmptyWeakBlock(WeakBlock*);

#if ENABLE(RESOURCE_USAGE)
    size_t blockBytesAllocated() const { return m_blockBytesAllocated; }
#endif

    void didAllocateBlock(size_t capacity);
    void didFreeBlock(size_t capacity);
    void setTimerThread();
    bool mutatorShouldBeFenced() const { return m_mutatorShouldBeFenced; }
    const bool* addressOfMutatorShouldBeFenced() const { return &m_mutatorShouldBeFenced; }
    
    unsigned barrierThreshold() const { return m_barrierThreshold; }
    const unsigned* addressOfBarrierThreshold() const { return &m_barrierThreshold; }

    // If true, the GC believes that the mutator is currently messing with the heap. We call this
    // "having heap access". The GC may block if the mutator is in this state. If false, the GC may
    // currently be doing things to the heap that make the heap unsafe to access for the mutator.
    bool hasAccess() const;
    
    // If the mutator does not currently have heap access, this function will acquire it. If the GC
    // is currently using the lack of heap access to do dangerous things to the heap then this
    // function will block, waiting for the GC to finish. It's not valid to call this if the mutator
    // already has heap access. The mutator is required to precisely track whether or not it has
    // heap access.
    //
    // It's totally fine to acquireAccess() upon VM instantiation and keep it that way. This is how
    // WebCore uses us. For most other clients, JSLock does acquireAccess()/releaseAccess() for you.
    void acquireAccess();
    
    // Releases heap access. If the GC is blocking waiting to do bad things to the heap, it will be
    // allowed to run now.
    //
    // Ordinarily, you should use the ReleaseHeapAccessScope to release and then reacquire heap
    // access. You should do this anytime you're about do perform a blocking operation, like waiting
    // on the ParkingLot.
    void releaseAccess();
    
    // This is like a super optimized way of saying:
    //
    //     releaseAccess()
    //     acquireAccess()
    //
    // The fast path is an inlined relaxed load and branch. The slow path will block the mutator if
    // the GC wants to do bad things to the heap.
    //
    // All allocations logically call this. As an optimization to improve GC progress, you can call
    // this anywhere that you can afford a load-branch and where an object allocation would have been
    // safe.
    //
    // The GC will also push a stopIfNecessary() event onto the runloop of the thread that
    // instantiated the VM whenever it wants the mutator to stop. This means that if you never block
    // but instead use the runloop to wait for events, then you could safely run in a mode where the
    // mutator has permanent heap access (like the DOM does). If you have good event handling
    // discipline (i.e. you don't block the runloop) then you can be sure that stopIfNecessary() will
    // already be called for you at the right times.
    void stopIfNecessary();
    
    // This gives the conn to the collector.
    void relinquishConn();
    
    bool mayNeedToStop();

    void performIncrement(size_t bytes);
    
    // This is a much stronger kind of stopping of the collector, and it may require waiting for a
    // while. This is meant to be a legacy API for clients of collectAllGarbage that expect that there
    // is no GC before or after that function call. After calling this, you are free to start GCs
    // yourself but you can be sure that none are running.
    //
    // This both prevents new collections from being started asynchronously and waits for any
    // outstanding collections to complete.
    void preventCollection();
    void allowCollection();
    
    uint64_t mutatorExecutionVersion() const { return m_mutatorExecutionVersion; }
    
    JS_EXPORT_PRIVATE void addMarkingConstraint(std::unique_ptr<MarkingConstraint>);
    
    size_t numOpaqueRoots() const { return m_opaqueRoots.size(); }

#if USE(CF)
    CFRunLoopRef runLoop() const { return m_runLoop.get(); }
    JS_EXPORT_PRIVATE void setRunLoop(CFRunLoopRef);
#endif // USE(CF)

    HeapVerifier* verifier() const { return m_verifier.get(); }

private:
    friend class AllocatingScope;
    friend class CodeBlock;
    friend class CollectingScope;
    friend class DeferGC;
    friend class DeferGCForAWhile;
    friend class GCAwareJITStubRoutine;
    friend class GCLogging;
    friend class GCThread;
    friend class HandleSet;
    friend class HeapUtil;
    friend class HeapVerifier;
    friend class JITStubRoutine;
    friend class LLIntOffsetsExtractor;
    friend class MarkedSpace;
    friend class MarkedAllocator;
    friend class MarkedBlock;
    friend class RunningScope;
    friend class SlotVisitor;
    friend class SpaceTimeMutatorScheduler;
    friend class StochasticSpaceTimeMutatorScheduler;
    friend class SweepingScope;
    friend class IncrementalSweeper;
    friend class HeapStatistics;
    friend class VM;
    friend class WeakSet;

    class Thread;
    friend class Thread;

    static const size_t minExtraMemory = 256;
    
    class FinalizerOwner : public WeakHandleOwner {
        void finalize(Handle<Unknown>, void* context) override;
    };

    JS_EXPORT_PRIVATE bool isValidAllocation(size_t);
    JS_EXPORT_PRIVATE void reportExtraMemoryAllocatedSlowCase(size_t);
    JS_EXPORT_PRIVATE void deprecatedReportExtraMemorySlowCase(size_t);
    
    bool shouldCollectInCollectorThread(const AbstractLocker&);
    void collectInCollectorThread();
    
    void checkConn(GCConductor);

    enum class RunCurrentPhaseResult {
        Finished,
        Continue,
        NeedCurrentThreadState
    };
    RunCurrentPhaseResult runCurrentPhase(GCConductor, CurrentThreadState*);
    
    // Returns true if we should keep doing things.
    bool runNotRunningPhase(GCConductor);
    bool runBeginPhase(GCConductor);
    bool runFixpointPhase(GCConductor);
    bool runConcurrentPhase(GCConductor);
    bool runReloopPhase(GCConductor);
    bool runEndPhase(GCConductor);
    bool changePhase(GCConductor, CollectorPhase);
    bool finishChangingPhase(GCConductor);
    
    void collectInMutatorThread();
    
    void stopThePeriphery(GCConductor);
    void resumeThePeriphery();
    
    // Returns true if the mutator is stopped, false if the mutator has the conn now.
    bool stopTheMutator();
    void resumeTheMutator();
    
    void stopIfNecessarySlow();
    bool stopIfNecessarySlow(unsigned extraStateBits);
    
    template<typename Func>
    void waitForCollector(const Func&);
    
    JS_EXPORT_PRIVATE void acquireAccessSlow();
    JS_EXPORT_PRIVATE void releaseAccessSlow();
    
    bool handleGCDidJIT(unsigned);
    void handleGCDidJIT();
    
    bool handleNeedFinalize(unsigned);
    void handleNeedFinalize();
    
    bool relinquishConn(unsigned);
    void finishRelinquishingConn();
    
    void setGCDidJIT();
    void setNeedFinalize();
    void waitWhileNeedFinalize();
    
    void setMutatorWaiting();
    void clearMutatorWaiting();
    void notifyThreadStopping(const AbstractLocker&);
    
    typedef uint64_t Ticket;
    Ticket requestCollection(std::optional<CollectionScope>);
    void waitForCollection(Ticket);
    
    void suspendCompilerThreads();
    void willStartCollection(std::optional<CollectionScope>);
    void prepareForMarking();
    
    void gatherStackRoots(ConservativeRoots&);
    void gatherJSStackRoots(ConservativeRoots&);
    void gatherScratchBufferRoots(ConservativeRoots&);
    void beginMarking();
    void visitCompilerWorklistWeakReferences();
    void removeDeadCompilerWorklistEntries();
    void updateObjectCounts();
    void endMarking();

    void reapWeakHandles();
    void pruneStaleEntriesFromWeakGCMaps();
    void sweepArrayBuffers();
    void snapshotUnswept();
    void deleteSourceProviderCaches();
    void notifyIncrementalSweeper();
    void harvestWeakReferences();
    void finalizeUnconditionalFinalizers();
    void clearUnmarkedExecutables();
    void deleteUnmarkedCompiledCode();
    JS_EXPORT_PRIVATE void addToRememberedSet(const JSCell*);
    void updateAllocationLimits();
    void didFinishCollection();
    void resumeCompilerThreads();
    void gatherExtraHeapSnapshotData(HeapProfiler&);
    void removeDeadHeapSnapshotNodes(HeapProfiler&);
    void finalize();
    void sweepLargeAllocations();
    
    void sweepAllLogicallyEmptyWeakBlocks();
    bool sweepNextLogicallyEmptyWeakBlock();

    bool shouldDoFullCollection(std::optional<CollectionScope> requestedCollectionScope) const;

    void incrementDeferralDepth();
    void decrementDeferralDepth();
    void decrementDeferralDepthAndGCIfNeeded();
    JS_EXPORT_PRIVATE void decrementDeferralDepthAndGCIfNeededSlow();

    size_t visitCount();
    size_t bytesVisited();
    
    void forEachCodeBlockImpl(const ScopedLambda<bool(CodeBlock*)>&);
    void forEachCodeBlockIgnoringJITPlansImpl(const AbstractLocker& codeBlockSetLocker, const ScopedLambda<bool(CodeBlock*)>&);
    
    void setMutatorShouldBeFenced(bool value);
    
    void addCoreConstraints();
    
    template<typename Func>
    void iterateExecutingAndCompilingCodeBlocks(const Func&);
    
    template<typename Func>
    void iterateExecutingAndCompilingCodeBlocksWithoutHoldingLocks(const Func&);
    
    void assertSharedMarkStacksEmpty();

    const HeapType m_heapType;
    const size_t m_ramSize;
    const size_t m_minBytesPerCycle;
    size_t m_sizeAfterLastCollect;
    size_t m_sizeAfterLastFullCollect;
    size_t m_sizeBeforeLastFullCollect;
    size_t m_sizeAfterLastEdenCollect;
    size_t m_sizeBeforeLastEdenCollect;

    size_t m_bytesAllocatedThisCycle;
    size_t m_bytesAbandonedSinceLastFullCollect;
    size_t m_maxEdenSize;
    size_t m_maxHeapSize;
    bool m_shouldDoFullCollection;
    size_t m_totalBytesVisited;
    size_t m_totalBytesVisitedThisCycle;
    double m_incrementBalance { 0 };
    
    std::optional<CollectionScope> m_collectionScope;
    std::optional<CollectionScope> m_lastCollectionScope;
    MutatorState m_mutatorState { MutatorState::Running };
    StructureIDTable m_structureIDTable;
    MarkedSpace m_objectSpace;
    GCIncomingRefCountedSet<ArrayBuffer> m_arrayBuffers;
    size_t m_extraMemorySize;
    size_t m_deprecatedExtraMemorySize;

    HashSet<const JSCell*> m_copyingRememberedSet;

    ProtectCountSet m_protectedValues;
    std::unique_ptr<HashSet<MarkedArgumentBuffer*>> m_markListSet;

    std::unique_ptr<MachineThreads> m_machineThreads;
    
    std::unique_ptr<SlotVisitor> m_collectorSlotVisitor;
    std::unique_ptr<SlotVisitor> m_mutatorSlotVisitor;
    std::unique_ptr<MarkStackArray> m_mutatorMarkStack;

    Lock m_raceMarkStackLock;
    std::unique_ptr<MarkStackArray> m_raceMarkStack;

    std::unique_ptr<MarkingConstraintSet> m_constraintSet;

    // We pool the slot visitors used by parallel marking threads. It's useful to be able to
    // enumerate over them, and it's useful to have them cache some small amount of memory from
    // one GC to the next. GC marking threads claim these at the start of marking, and return
    // them at the end.
    Vector<std::unique_ptr<SlotVisitor>> m_parallelSlotVisitors;
    Vector<SlotVisitor*> m_availableParallelSlotVisitors;
    Lock m_parallelSlotVisitorLock;
    
    template<typename Func>
    void forEachSlotVisitor(const Func&);

    HandleSet m_handleSet;
    HandleStack m_handleStack;
    std::unique_ptr<CodeBlockSet> m_codeBlocks;
    std::unique_ptr<JITStubRoutineSet> m_jitStubRoutines;
    FinalizerOwner m_finalizerOwner;
    
    bool m_isSafeToCollect;

    bool m_mutatorShouldBeFenced { Options::forceFencedBarrier() };
    unsigned m_barrierThreshold { Options::forceFencedBarrier() ? tautologicalThreshold : blackThreshold };

    VM* m_vm;
    Seconds m_lastFullGCLength;
    Seconds m_lastEdenGCLength;

    Vector<ExecutableBase*> m_executables;

    Vector<WeakBlock*> m_logicallyEmptyWeakBlocks;
    size_t m_indexOfNextLogicallyEmptyWeakBlockToSweep { WTF::notFound };
    
#if USE(CF)
    RetainPtr<CFRunLoopRef> m_runLoop;
#endif // USE(CF)
    RefPtr<FullGCActivityCallback> m_fullActivityCallback;
    RefPtr<GCActivityCallback> m_edenActivityCallback;
    RefPtr<IncrementalSweeper> m_sweeper;
    RefPtr<StopIfNecessaryTimer> m_stopIfNecessaryTimer;

    Vector<HeapObserver*> m_observers;

    unsigned m_deferralDepth;
    bool m_didDeferGCWork { false };

    std::unique_ptr<HeapVerifier> m_verifier;

#if USE(FOUNDATION)
    Vector<RetainPtr<CFTypeRef>> m_delayedReleaseObjects;
    unsigned m_delayedReleaseRecursionCount;
#endif

    HashMap<void*, std::function<void()>> m_weakGCMaps;
    
    Lock m_visitRaceLock;

    Lock m_markingMutex;
    Condition m_markingConditionVariable;
    std::unique_ptr<MarkStackArray> m_sharedCollectorMarkStack;
    std::unique_ptr<MarkStackArray> m_sharedMutatorMarkStack;
    unsigned m_numberOfActiveParallelMarkers { 0 };
    unsigned m_numberOfWaitingParallelMarkers { 0 };
    bool m_parallelMarkersShouldExit { false };

    Lock m_opaqueRootsMutex;
    HashSet<const void*> m_opaqueRoots;

    static const size_t s_blockFragmentLength = 32;

    ListableHandler<WeakReferenceHarvester>::List m_weakReferenceHarvesters;
    ListableHandler<UnconditionalFinalizer>::List m_unconditionalFinalizers;

    ParallelHelperClient m_helperClient;

#if ENABLE(RESOURCE_USAGE)
    size_t m_blockBytesAllocated { 0 };
    size_t m_externalMemorySize { 0 };
#endif
    
    std::unique_ptr<MutatorScheduler> m_scheduler;
    
    static const unsigned mutatorHasConnBit = 1u << 0u; // Must also be protected by threadLock.
    static const unsigned stoppedBit = 1u << 1u; // Only set when !hasAccessBit
    static const unsigned hasAccessBit = 1u << 2u;
    static const unsigned gcDidJITBit = 1u << 3u; // Set when the GC did some JITing, so on resume we need to cpuid.
    static const unsigned needFinalizeBit = 1u << 4u;
    static const unsigned mutatorWaitingBit = 1u << 5u; // Allows the mutator to use this as a condition variable.
    Atomic<unsigned> m_worldState;
    bool m_collectorBelievesThatTheWorldIsStopped { false };
    MonotonicTime m_beforeGC;
    MonotonicTime m_afterGC;
    MonotonicTime m_stopTime;
    
    Deque<std::optional<CollectionScope>> m_requests;
    Ticket m_lastServedTicket { 0 };
    Ticket m_lastGrantedTicket { 0 };
    CollectorPhase m_currentPhase { CollectorPhase::NotRunning };
    CollectorPhase m_nextPhase { CollectorPhase::NotRunning };
    bool m_threadShouldStop { false };
    bool m_threadIsStopping { false };
    bool m_mutatorDidRun { true };
    uint64_t m_mutatorExecutionVersion { 0 };
    Box<Lock> m_threadLock;
    RefPtr<AutomaticThreadCondition> m_threadCondition; // The mutator must not wait on this. It would cause a deadlock.
    RefPtr<AutomaticThread> m_thread;
    
    Lock m_collectContinuouslyLock;
    Condition m_collectContinuouslyCondition;
    bool m_shouldStopCollectingContinuously { false };
    ThreadIdentifier m_collectContinuouslyThread { 0 };
    
    MonotonicTime m_lastGCStartTime;
    MonotonicTime m_lastGCEndTime;
    MonotonicTime m_currentGCStartTime;
    
    uintptr_t m_barriersExecuted { 0 };
    
    CurrentThreadState* m_currentThreadState { nullptr };
};

} // namespace JSC
