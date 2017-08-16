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

#include "config.h"
#include "SlotVisitor.h"

#include "CPU.h"
#include "ConservativeRoots.h"
#include "GCSegmentedArrayInlines.h"
#include "HeapCellInlines.h"
#include "HeapProfiler.h"
#include "HeapSnapshotBuilder.h"
#include "JSArray.h"
#include "JSDestructibleObject.h"
#include "JSObject.h"
#include "JSString.h"
#include "JSCInlines.h"
#include "SlotVisitorInlines.h"
#include "StopIfNecessaryTimer.h"
#include "SuperSampler.h"
#include "VM.h"
#include <wtf/Lock.h>

namespace JSC {

#if ENABLE(GC_VALIDATION)
static void validate(JSCell* cell)
{
    RELEASE_ASSERT(cell);

    if (!cell->structure()) {
        dataLogF("cell at %p has a null structure\n" , cell);
        CRASH();
    }

    // Both the cell's structure, and the cell's structure's structure should be the Structure Structure.
    // I hate this sentence.
    VM& vm = *cell->vm();
    if (cell->structure()->structure()->JSCell::classInfo(vm) != cell->structure()->JSCell::classInfo(vm)) {
        const char* parentClassName = 0;
        const char* ourClassName = 0;
        if (cell->structure()->structure() && cell->structure()->structure()->JSCell::classInfo(vm))
            parentClassName = cell->structure()->structure()->JSCell::classInfo(vm)->className;
        if (cell->structure()->JSCell::classInfo(vm))
            ourClassName = cell->structure()->JSCell::classInfo(vm)->className;
        dataLogF("parent structure (%p <%s>) of cell at %p doesn't match cell's structure (%p <%s>)\n",
            cell->structure()->structure(), parentClassName, cell, cell->structure(), ourClassName);
        CRASH();
    }

    // Make sure we can walk the ClassInfo chain
    const ClassInfo* info = cell->classInfo(vm);
    do { } while ((info = info->parentClass));
}
#endif

SlotVisitor::SlotVisitor(Heap& heap, CString codeName)
    : m_bytesVisited(0)
    , m_visitCount(0)
    , m_isInParallelMode(false)
    , m_markingVersion(MarkedSpace::initialVersion)
    , m_heap(heap)
    , m_codeName(codeName)
#if !ASSERT_DISABLED
    , m_isCheckingForDefaultMarkViolation(false)
    , m_isDraining(false)
#endif
{
}

SlotVisitor::~SlotVisitor()
{
    clearMarkStacks();
}

void SlotVisitor::didStartMarking()
{
    if (heap()->collectionScope() == CollectionScope::Full)
        RELEASE_ASSERT(m_opaqueRoots.isEmpty()); // Should have merged by now.
    else
        reset();

    if (HeapProfiler* heapProfiler = vm().heapProfiler())
        m_heapSnapshotBuilder = heapProfiler->activeSnapshotBuilder();
    
    m_markingVersion = heap()->objectSpace().markingVersion();
}

void SlotVisitor::reset()
{
    RELEASE_ASSERT(!m_opaqueRoots.size());
    m_bytesVisited = 0;
    m_visitCount = 0;
    m_heapSnapshotBuilder = nullptr;
    RELEASE_ASSERT(!m_currentCell);
}

void SlotVisitor::clearMarkStacks()
{
    forEachMarkStack(
        [&] (MarkStackArray& stack) -> IterationStatus {
            stack.clear();
            return IterationStatus::Continue;
        });
}

void SlotVisitor::append(ConservativeRoots& conservativeRoots)
{
    HeapCell** roots = conservativeRoots.roots();
    size_t size = conservativeRoots.size();
    for (size_t i = 0; i < size; ++i)
        appendJSCellOrAuxiliary(roots[i]);
}

void SlotVisitor::appendJSCellOrAuxiliary(HeapCell* heapCell)
{
    if (!heapCell)
        return;
    
    ASSERT(!m_isCheckingForDefaultMarkViolation);
    
    auto validateCell = [&] (JSCell* jsCell) {
        StructureID structureID = jsCell->structureID();
        
        auto die = [&] (const char* text) {
            WTF::dataFile().atomically(
                [&] (PrintStream& out) {
                    out.print(text);
                    out.print("GC type: ", heap()->collectionScope(), "\n");
                    out.print("Object at: ", RawPointer(jsCell), "\n");
#if USE(JSVALUE64)
                    out.print("Structure ID: ", structureID, " (0x", format("%x", structureID), ")\n");
                    out.print("Structure ID table size: ", heap()->structureIDTable().size(), "\n");
#else
                    out.print("Structure: ", RawPointer(structureID), "\n");
#endif
                    out.print("Object contents:");
                    for (unsigned i = 0; i < 2; ++i)
                        out.print(" ", format("0x%016llx", bitwise_cast<uint64_t*>(jsCell)[i]));
                    out.print("\n");
                    CellContainer container = jsCell->cellContainer();
                    out.print("Is marked: ", container.isMarked(jsCell), "\n");
                    out.print("Is newly allocated: ", container.isNewlyAllocated(jsCell), "\n");
                    if (container.isMarkedBlock()) {
                        MarkedBlock& block = container.markedBlock();
                        out.print("Block: ", RawPointer(&block), "\n");
                        block.handle().dumpState(out);
                        out.print("\n");
                        out.print("Is marked raw: ", block.isMarkedRaw(jsCell), "\n");
                        out.print("Marking version: ", block.markingVersion(), "\n");
                        out.print("Heap marking version: ", heap()->objectSpace().markingVersion(), "\n");
                        out.print("Is newly allocated raw: ", block.handle().isNewlyAllocated(jsCell), "\n");
                        out.print("Newly allocated version: ", block.handle().newlyAllocatedVersion(), "\n");
                        out.print("Heap newly allocated version: ", heap()->objectSpace().newlyAllocatedVersion(), "\n");
                    }
                    UNREACHABLE_FOR_PLATFORM();
                });
        };
        
        // It's not OK for the structure to be null at any GC scan point. We must not GC while
        // an object is not fully initialized.
        if (!structureID)
            die("GC scan found corrupt object: structureID is zero!\n");
        
        // It's not OK for the structure to be nuked at any GC scan point.
        if (isNuked(structureID))
            die("GC scan found object in bad state: structureID is nuked!\n");
        
#if USE(JSVALUE64)
        // This detects the worst of the badness.
        if (structureID >= heap()->structureIDTable().size())
            die("GC scan found corrupt object: structureID is out of bounds!\n");
#endif
    };
    
    // In debug mode, we validate before marking since this makes it clearer what the problem
    // was. It's also slower, so we don't do it normally.
    if (!ASSERT_DISABLED && heapCell->cellKind() == HeapCell::JSCell)
        validateCell(static_cast<JSCell*>(heapCell));
    
    if (Heap::testAndSetMarked(m_markingVersion, heapCell))
        return;
    
    switch (heapCell->cellKind()) {
    case HeapCell::JSCell: {
        // We have ample budget to perform validation here.
    
        JSCell* jsCell = static_cast<JSCell*>(heapCell);
        validateCell(jsCell);
        
        jsCell->setCellState(CellState::PossiblyGrey);

        appendToMarkStack(jsCell);
        return;
    }
        
    case HeapCell::Auxiliary: {
        noteLiveAuxiliaryCell(heapCell);
        return;
    } }
}

void SlotVisitor::appendSlow(JSCell* cell, Dependency dependency)
{
    if (UNLIKELY(m_heapSnapshotBuilder))
        m_heapSnapshotBuilder->appendEdge(m_currentCell, cell);
    
    appendHiddenSlowImpl(cell, dependency);
}

void SlotVisitor::appendHiddenSlow(JSCell* cell, Dependency dependency)
{
    appendHiddenSlowImpl(cell, dependency);
}

ALWAYS_INLINE void SlotVisitor::appendHiddenSlowImpl(JSCell* cell, Dependency dependency)
{
    ASSERT(!m_isCheckingForDefaultMarkViolation);

#if ENABLE(GC_VALIDATION)
    validate(cell);
#endif
    
    if (cell->isLargeAllocation())
        setMarkedAndAppendToMarkStack(cell->largeAllocation(), cell, dependency);
    else
        setMarkedAndAppendToMarkStack(cell->markedBlock(), cell, dependency);
}

template<typename ContainerType>
ALWAYS_INLINE void SlotVisitor::setMarkedAndAppendToMarkStack(ContainerType& container, JSCell* cell, Dependency dependency)
{
    if (container.testAndSetMarked(cell, dependency, TransactionAbortLikelihood::Unlikely))
        return;
    
    ASSERT(cell->structure());
    
    // Indicate that the object is grey and that:
    // In case of concurrent GC: it's the first time it is grey in this GC cycle.
    // In case of eden collection: it's a new object that became grey rather than an old remembered object.
    cell->setCellState(CellState::PossiblyGrey);
    
    appendToMarkStack(container, cell);
}

void SlotVisitor::appendToMarkStack(JSCell* cell)
{
    if (cell->isLargeAllocation())
        appendToMarkStack(cell->largeAllocation(), cell);
    else
        appendToMarkStack(cell->markedBlock(), cell);
}

template<typename ContainerType>
ALWAYS_INLINE void SlotVisitor::appendToMarkStack(ContainerType& container, JSCell* cell)
{
    ASSERT(Heap::isMarkedConcurrently(cell));
    ASSERT(!cell->isZapped());
    
    container.noteMarked();
    
    m_visitCount++;
    m_bytesVisited += container.cellSize();

    m_collectorStack.append(cell);
}

void SlotVisitor::appendToMutatorMarkStack(const JSCell* cell)
{
    m_mutatorStack.append(cell);
}

void SlotVisitor::markAuxiliary(const void* base)
{
    HeapCell* cell = bitwise_cast<HeapCell*>(base);
    
    ASSERT(cell->heap() == heap());
    
    if (Heap::testAndSetMarked(m_markingVersion, cell))
        return;
    
    noteLiveAuxiliaryCell(cell);
}

void SlotVisitor::noteLiveAuxiliaryCell(HeapCell* cell)
{
    // We get here once per GC under these circumstances:
    //
    // Eden collection: if the cell was allocated since the last collection and is live somehow.
    //
    // Full collection: if the cell is live somehow.
    
    CellContainer container = cell->cellContainer();
    
    container.assertValidCell(vm(), cell);
    container.noteMarked();
    
    m_visitCount++;

    size_t cellSize = container.cellSize();
    m_bytesVisited += cellSize;
    m_nonCellVisitCount += cellSize;
}

class SetCurrentCellScope {
public:
    SetCurrentCellScope(SlotVisitor& visitor, const JSCell* cell)
        : m_visitor(visitor)
    {
        ASSERT(!m_visitor.m_currentCell);
        m_visitor.m_currentCell = const_cast<JSCell*>(cell);
    }

    ~SetCurrentCellScope()
    {
        ASSERT(m_visitor.m_currentCell);
        m_visitor.m_currentCell = nullptr;
    }

private:
    SlotVisitor& m_visitor;
};

ALWAYS_INLINE void SlotVisitor::visitChildren(const JSCell* cell)
{
    ASSERT(Heap::isMarkedConcurrently(cell));
    
    SetCurrentCellScope currentCellScope(*this, cell);
    
    if (false) {
        dataLog("Visiting ", RawPointer(cell));
        if (!m_isFirstVisit)
            dataLog(" (subsequent)");
        dataLog("\n");
    }
    
    // Funny story: it's possible for the object to be black already, if we barrier the object at
    // about the same time that it's marked. That's fine. It's a gnarly and super-rare race. It's
    // not clear to me that it would be correct or profitable to bail here if the object is already
    // black.
    
    cell->setCellState(CellState::PossiblyBlack);
    
    WTF::storeLoadFence();
    
    switch (cell->type()) {
    case StringType:
        JSString::visitChildren(const_cast<JSCell*>(cell), *this);
        break;
        
    case FinalObjectType:
        JSFinalObject::visitChildren(const_cast<JSCell*>(cell), *this);
        break;

    case ArrayType:
        JSArray::visitChildren(const_cast<JSCell*>(cell), *this);
        break;
        
    default:
        // FIXME: This could be so much better.
        // https://bugs.webkit.org/show_bug.cgi?id=162462
        cell->methodTable(vm())->visitChildren(const_cast<JSCell*>(cell), *this);
        break;
    }
    
    if (UNLIKELY(m_heapSnapshotBuilder)) {
        if (m_isFirstVisit)
            m_heapSnapshotBuilder->appendNode(const_cast<JSCell*>(cell));
    }
}

void SlotVisitor::visitAsConstraint(const JSCell* cell)
{
    m_isFirstVisit = false;
    visitChildren(cell);
}

void SlotVisitor::donateKnownParallel(MarkStackArray& from, MarkStackArray& to)
{
    // NOTE: Because we re-try often, we can afford to be conservative, and
    // assume that donating is not profitable.

    // Avoid locking when a thread reaches a dead end in the object graph.
    if (from.size() < 2)
        return;

    // If there's already some shared work queued up, be conservative and assume
    // that donating more is not profitable.
    if (to.size())
        return;

    // If we're contending on the lock, be conservative and assume that another
    // thread is already donating.
    std::unique_lock<Lock> lock(m_heap.m_markingMutex, std::try_to_lock);
    if (!lock.owns_lock())
        return;

    // Otherwise, assume that a thread will go idle soon, and donate.
    from.donateSomeCellsTo(to);

    m_heap.m_markingConditionVariable.notifyAll();
}

void SlotVisitor::donateKnownParallel()
{
    forEachMarkStack(
        [&] (MarkStackArray& stack) -> IterationStatus {
            donateKnownParallel(stack, correspondingGlobalStack(stack));
            return IterationStatus::Continue;
        });
}

void SlotVisitor::updateMutatorIsStopped(const AbstractLocker&)
{
    m_mutatorIsStopped = (m_heap.collectorBelievesThatTheWorldIsStopped() & m_canOptimizeForStoppedMutator);
}

void SlotVisitor::updateMutatorIsStopped()
{
    if (mutatorIsStoppedIsUpToDate())
        return;
    updateMutatorIsStopped(holdLock(m_rightToRun));
}

bool SlotVisitor::hasAcknowledgedThatTheMutatorIsResumed() const
{
    return !m_mutatorIsStopped;
}

bool SlotVisitor::mutatorIsStoppedIsUpToDate() const
{
    return m_mutatorIsStopped == (m_heap.collectorBelievesThatTheWorldIsStopped() & m_canOptimizeForStoppedMutator);
}

void SlotVisitor::optimizeForStoppedMutator()
{
    m_canOptimizeForStoppedMutator = true;
}

NEVER_INLINE void SlotVisitor::drain(MonotonicTime timeout)
{
    if (!m_isInParallelMode) {
        dataLog("FATAL: attempting to drain when not in parallel mode.\n");
        RELEASE_ASSERT_NOT_REACHED();
    }
    
    auto locker = holdLock(m_rightToRun);
    
    while (!hasElapsed(timeout)) {
        updateMutatorIsStopped(locker);
        IterationStatus status = forEachMarkStack(
            [&] (MarkStackArray& stack) -> IterationStatus {
                if (stack.isEmpty())
                    return IterationStatus::Continue;

                stack.refill();
                
                m_isFirstVisit = (&stack == &m_collectorStack);

                for (unsigned countdown = Options::minimumNumberOfScansBetweenRebalance(); stack.canRemoveLast() && countdown--;)
                    visitChildren(stack.removeLast());
                return IterationStatus::Done;
            });
        if (status == IterationStatus::Continue)
            break;
        
        m_rightToRun.safepoint();
        donateKnownParallel();
    }
    
    mergeIfNecessary();
}

size_t SlotVisitor::performIncrementOfDraining(size_t bytesRequested)
{
    RELEASE_ASSERT(m_isInParallelMode);

    size_t cellsRequested = bytesRequested / MarkedBlock::atomSize;
    {
        auto locker = holdLock(m_heap.m_markingMutex);
        forEachMarkStack(
            [&] (MarkStackArray& stack) -> IterationStatus {
                cellsRequested -= correspondingGlobalStack(stack).transferTo(stack, cellsRequested);
                return cellsRequested ? IterationStatus::Continue : IterationStatus::Done;
            });
    }

    size_t cellBytesVisited = 0;
    m_nonCellVisitCount = 0;

    auto bytesVisited = [&] () -> size_t {
        return cellBytesVisited + m_nonCellVisitCount;
    };

    auto isDone = [&] () -> bool {
        return bytesVisited() >= bytesRequested;
    };
    
    {
        auto locker = holdLock(m_rightToRun);
        
        while (!isDone()) {
            updateMutatorIsStopped(locker);
            IterationStatus status = forEachMarkStack(
                [&] (MarkStackArray& stack) -> IterationStatus {
                    if (stack.isEmpty() || isDone())
                        return IterationStatus::Continue;

                    stack.refill();
                    
                    m_isFirstVisit = (&stack == &m_collectorStack);

                    unsigned countdown = Options::minimumNumberOfScansBetweenRebalance();
                    while (countdown && stack.canRemoveLast() && !isDone()) {
                        const JSCell* cell = stack.removeLast();
                        cellBytesVisited += cell->cellSize();
                        visitChildren(cell);
                        countdown--;
                    }
                    return IterationStatus::Done;
                });
            if (status == IterationStatus::Continue)
                break;
            m_rightToRun.safepoint();
            donateKnownParallel();
        }
    }

    donateAll();
    mergeIfNecessary();

    return bytesVisited();
}

bool SlotVisitor::didReachTermination()
{
    LockHolder locker(m_heap.m_markingMutex);
    return didReachTermination(locker);
}

bool SlotVisitor::didReachTermination(const AbstractLocker&)
{
    return isEmpty()
        && !m_heap.m_numberOfActiveParallelMarkers
        && m_heap.m_sharedCollectorMarkStack->isEmpty()
        && m_heap.m_sharedMutatorMarkStack->isEmpty();
}

bool SlotVisitor::hasWork(const AbstractLocker&)
{
    return !m_heap.m_sharedCollectorMarkStack->isEmpty()
        || !m_heap.m_sharedMutatorMarkStack->isEmpty();
}

NEVER_INLINE SlotVisitor::SharedDrainResult SlotVisitor::drainFromShared(SharedDrainMode sharedDrainMode, MonotonicTime timeout)
{
    ASSERT(m_isInParallelMode);
    
    ASSERT(Options::numberOfGCMarkers());

    bool isActive = false;
    while (true) {
        {
            LockHolder locker(m_heap.m_markingMutex);
            if (isActive)
                m_heap.m_numberOfActiveParallelMarkers--;
            m_heap.m_numberOfWaitingParallelMarkers++;

            if (sharedDrainMode == MasterDrain) {
                while (true) {
                    if (hasElapsed(timeout))
                        return SharedDrainResult::TimedOut;

                    if (didReachTermination(locker)) {
                        m_heap.m_markingConditionVariable.notifyAll();
                        return SharedDrainResult::Done;
                    }
                    
                    if (hasWork(locker))
                        break;

                    m_heap.m_markingConditionVariable.waitUntil(m_heap.m_markingMutex, timeout);
                }
            } else {
                ASSERT(sharedDrainMode == SlaveDrain);

                if (hasElapsed(timeout))
                    return SharedDrainResult::TimedOut;
                
                if (didReachTermination(locker)) {
                    m_heap.m_markingConditionVariable.notifyAll();
                    
                    // If we're in concurrent mode, then we know that the mutator will eventually do
                    // the right thing because:
                    // - It's possible that the collector has the conn. In that case, the collector will
                    //   wake up from the notification above. This will happen if the app released heap
                    //   access. Native apps can spend a lot of time with heap access released.
                    // - It's possible that the mutator will allocate soon. Then it will check if we
                    //   reached termination. This is the most likely outcome in programs that allocate
                    //   a lot.
                    // - WebCore never releases access. But WebCore has a runloop. The runloop will check
                    //   if we reached termination.
                    // So, this tells the runloop that it's got things to do.
                    m_heap.m_stopIfNecessaryTimer->scheduleSoon();
                }

                auto isReady = [&] () -> bool {
                    return hasWork(locker)
                        || m_heap.m_parallelMarkersShouldExit;
                };

                m_heap.m_markingConditionVariable.waitUntil(m_heap.m_markingMutex, timeout, isReady);
                
                if (m_heap.m_parallelMarkersShouldExit)
                    return SharedDrainResult::Done;
            }

            forEachMarkStack(
                [&] (MarkStackArray& stack) -> IterationStatus {
                    stack.stealSomeCellsFrom(
                        correspondingGlobalStack(stack),
                        m_heap.m_numberOfWaitingParallelMarkers);
                    return IterationStatus::Continue;
                });

            m_heap.m_numberOfActiveParallelMarkers++;
            m_heap.m_numberOfWaitingParallelMarkers--;
        }
        
        drain(timeout);
        isActive = true;
    }
}

SlotVisitor::SharedDrainResult SlotVisitor::drainInParallel(MonotonicTime timeout)
{
    donateAndDrain(timeout);
    return drainFromShared(MasterDrain, timeout);
}

SlotVisitor::SharedDrainResult SlotVisitor::drainInParallelPassively(MonotonicTime timeout)
{
    ASSERT(m_isInParallelMode);
    
    ASSERT(Options::numberOfGCMarkers());
    
    if (Options::numberOfGCMarkers() == 1
        || (m_heap.m_worldState.load() & Heap::mutatorWaitingBit)
        || !m_heap.hasHeapAccess()
        || m_heap.collectorBelievesThatTheWorldIsStopped()) {
        // This is an optimization over drainInParallel() when we have a concurrent mutator but
        // otherwise it is not profitable.
        return drainInParallel(timeout);
    }

    LockHolder locker(m_heap.m_markingMutex);
    donateAll(locker);
    
    for (;;) {
        if (hasElapsed(timeout))
            return SharedDrainResult::TimedOut;
        
        if (didReachTermination(locker)) {
            m_heap.m_markingConditionVariable.notifyAll();
            return SharedDrainResult::Done;
        }
        
        m_heap.m_markingConditionVariable.waitUntil(m_heap.m_markingMutex, timeout);
    }
}

void SlotVisitor::donateAll()
{
    if (isEmpty())
        return;
    
    donateAll(holdLock(m_heap.m_markingMutex));
}

void SlotVisitor::donateAll(const AbstractLocker&)
{
    forEachMarkStack(
        [&] (MarkStackArray& stack) -> IterationStatus {
            stack.transferTo(correspondingGlobalStack(stack));
            return IterationStatus::Continue;
        });

    m_heap.m_markingConditionVariable.notifyAll();
}

void SlotVisitor::addOpaqueRoot(void* root)
{
    if (!root)
        return;
    
    if (m_ignoreNewOpaqueRoots)
        return;
    
    if (Options::numberOfGCMarkers() == 1) {
        // Put directly into the shared HashSet.
        m_heap.m_opaqueRoots.add(root);
        return;
    }
    // Put into the local set, but merge with the shared one every once in
    // a while to make sure that the local sets don't grow too large.
    mergeOpaqueRootsIfProfitable();
    m_opaqueRoots.add(root);
}

bool SlotVisitor::containsOpaqueRoot(void* root) const
{
    if (!root)
        return false;
    
    ASSERT(!m_isInParallelMode);
    return m_heap.m_opaqueRoots.contains(root);
}

TriState SlotVisitor::containsOpaqueRootTriState(void* root) const
{
    if (!root)
        return FalseTriState;
    
    if (m_opaqueRoots.contains(root))
        return TrueTriState;
    std::lock_guard<Lock> lock(m_heap.m_opaqueRootsMutex);
    if (m_heap.m_opaqueRoots.contains(root))
        return TrueTriState;
    return MixedTriState;
}

void SlotVisitor::mergeIfNecessary()
{
    if (m_opaqueRoots.isEmpty())
        return;
    mergeOpaqueRoots();
}

void SlotVisitor::mergeOpaqueRootsIfProfitable()
{
    if (static_cast<unsigned>(m_opaqueRoots.size()) < Options::opaqueRootMergeThreshold())
        return;
    mergeOpaqueRoots();
}
    
void SlotVisitor::donate()
{
    if (!m_isInParallelMode) {
        dataLog("FATAL: Attempting to donate when not in parallel mode.\n");
        RELEASE_ASSERT_NOT_REACHED();
    }
    
    if (Options::numberOfGCMarkers() == 1)
        return;
    
    donateKnownParallel();
}

void SlotVisitor::donateAndDrain(MonotonicTime timeout)
{
    donate();
    drain(timeout);
}

void SlotVisitor::mergeOpaqueRoots()
{
    {
        std::lock_guard<Lock> lock(m_heap.m_opaqueRootsMutex);
        for (auto* root : m_opaqueRoots)
            m_heap.m_opaqueRoots.add(root);
    }
    m_opaqueRoots.clear();
}

void SlotVisitor::addWeakReferenceHarvester(WeakReferenceHarvester* weakReferenceHarvester)
{
    m_heap.m_weakReferenceHarvesters.addThreadSafe(weakReferenceHarvester);
}

void SlotVisitor::addUnconditionalFinalizer(UnconditionalFinalizer* unconditionalFinalizer)
{
    m_heap.m_unconditionalFinalizers.addThreadSafe(unconditionalFinalizer);
}

void SlotVisitor::didRace(const VisitRaceKey& race)
{
    if (Options::verboseVisitRace())
        dataLog(toCString("GC visit race: ", race, "\n"));
    
    auto locker = holdLock(heap()->m_raceMarkStackLock);
    JSCell* cell = race.cell();
    cell->setCellState(CellState::PossiblyGrey);
    heap()->m_raceMarkStack->append(cell);
}

void SlotVisitor::dump(PrintStream& out) const
{
    out.print("Collector: [", pointerListDump(collectorMarkStack()), "], Mutator: [", pointerListDump(mutatorMarkStack()), "]");
}

MarkStackArray& SlotVisitor::correspondingGlobalStack(MarkStackArray& stack)
{
    if (&stack == &m_collectorStack)
        return *m_heap.m_sharedCollectorMarkStack;
    RELEASE_ASSERT(&stack == &m_mutatorStack);
    return *m_heap.m_sharedMutatorMarkStack;
}

} // namespace JSC
