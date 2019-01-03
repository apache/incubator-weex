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

#include "AllocatorAttributes.h"
#include "FreeList.h"
#include "MarkedBlock.h"
#include <wtf/FastBitVector.h>
#include <wtf/SentinelLinkedList.h>
#include <wtf/Vector.h>

namespace JSC {

class GCDeferralContext;
class Heap;
class MarkedSpace;
class LLIntOffsetsExtractor;

#define FOR_EACH_MARKED_ALLOCATOR_BIT(macro) \
    macro(live, Live) /* The set of block indices that have actual blocks. */\
    macro(empty, Empty) /* The set of all blocks that have no live objects and nothing to destroy. */ \
    macro(allocated, Allocated) /* The set of all blocks that are full of live objects. */\
    macro(canAllocateButNotEmpty, CanAllocateButNotEmpty) /* The set of all blocks are neither empty nor retired (i.e. are more than minMarkedBlockUtilization full). */ \
    macro(eden, Eden) /* The set of all blocks that have new objects since the last GC. */\
    macro(unswept, Unswept) /* The set of all blocks that could be swept by the incremental sweeper. */\
    \
    /* These are computed during marking. */\
    macro(markingNotEmpty, MarkingNotEmpty) /* The set of all blocks that are not empty. */ \
    macro(markingRetired, MarkingRetired) /* The set of all blocks that are retired. */

// FIXME: We defined canAllocateButNotEmpty and empty to be exclusive:
//
//     canAllocateButNotEmpty & empty == 0
//
// Instead of calling it canAllocate and making it inclusive:
//
//     canAllocate & empty == empty
//
// The latter is probably better. I'll leave it to a future bug to fix that, since breathing on
// this code leads to regressions for days, and it's not clear that making this change would
// improve perf since it would not change the collector's behavior, and either way the allocator
// has to look at both bitvectors.
// https://bugs.webkit.org/show_bug.cgi?id=162121

// Note that this collector supports overlapping allocator state with marking state, since in a
// concurrent collector you allow allocation while marking is running. So it's best to visualize a
// full mutable->eden collect->mutate->full collect cycle and see how the bits above get affected.
// The example below tries to be exhaustive about what happens to the bits, but omits a lot of
// things that happen to other state.
//
// Create allocator
// - all bits are empty
// Start allocating in some block
// - allocate the block and set the live bit.
// - the empty bit for the block flickers on and then gets immediately cleared by sweeping.
// - set the eden bit.
// Finish allocating in that block
// - set the allocated bit.
// Do that to a lot of blocks and then start an eden collection.
// - beginMarking() has nothing to do.
// - by default we have cleared markingNotEmpty/markingRetired bits.
// - marking builds up markingNotEmpty/markingRetired bits.
// We do endMarking()
// - clear all allocated bits.
// - for destructor blocks: fragmented = live & ~markingRetired
// - for non-destructor blocks:
//       empty = live & ~markingNotEmpty
//       fragmented = live & markingNotEmpty & ~markingRetired
// Snapshotting.
// - unswept |= eden
// Prepare for allocation.
// - clear eden
// Finish collection.
// Allocate in some block that had some free and some live objects.
// - clear the canAllocateButNotEmpty bit
// - clear the unswept bit
// - set the eden bit
// Finish allocating (set the allocated bit).
// Allocate in some block that was completely empty.
// - clear the empty bit
// - clear the unswept bit
// - set the eden bit.
// Finish allocating (set the allocated bit).
// Allocate in some block that was completely empty in another allocator.
// - clear the empty bit
// - clear all bits in that allocator
// - set the live bit in another allocator and the empty bit.
// - clear the empty, unswept bits.
// - set the eden bit.
// Finish allocating (set the allocated bit).
// Start a full collection.
// - beginMarking() clears markingNotEmpty, markingRetired
// - the heap version is incremented
// - marking rebuilds markingNotEmpty/markingretired bits.
// We do endMarking()
// - clear all allocated bits.
// - set canAllocateButNotEmpty/empty the same way as in eden collection.
// Snapshotting.
// - unswept = live
// prepare for allocation.
// - clear eden.
// Finish collection.
//
// Notice how in this scheme, the empty/canAllocateButNotEmpty state stays separate from the
// markingNotEmpty/markingRetired state. This is one step towards having separated allocation and
// marking state.

class MarkedAllocator {
    friend class LLIntOffsetsExtractor;

public:
    static ptrdiff_t offsetOfFreeList();
    static ptrdiff_t offsetOfCellSize();

    MarkedAllocator(Heap*, Subspace*, size_t cellSize);
    void lastChanceToFinalize();
    void prepareForAllocation();
    void stopAllocating();
    void resumeAllocating();
    void beginMarkingForFullCollection();
    void endMarking();
    void snapshotUnsweptForEdenCollection();
    void snapshotUnsweptForFullCollection();
    void sweep();
    void shrink();
    void assertNoUnswept();
    size_t cellSize() const { return m_cellSize; }
    const AllocatorAttributes& attributes() const { return m_attributes; }
    bool needsDestruction() const { return m_attributes.destruction == NeedsDestruction; }
    DestructionMode destruction() const { return m_attributes.destruction; }
    HeapCell::Kind cellKind() const { return m_attributes.cellKind; }
    void* allocate(GCDeferralContext* = nullptr);
    void* tryAllocate(GCDeferralContext* = nullptr);
    Heap* heap() { return m_heap; }

    template<typename Functor> void forEachBlock(const Functor&);
    template<typename Functor> void forEachNotEmptyBlock(const Functor&);
    
    void addBlock(MarkedBlock::Handle*);
    void removeBlock(MarkedBlock::Handle*);

    bool isPagedOut(double deadline);
    
    static size_t blockSizeForBytes(size_t);
    
    Lock& bitvectorLock() { return m_bitvectorLock; }
   
#define MARKED_ALLOCATOR_BIT_ACCESSORS(lowerBitName, capitalBitName)     \
    bool is ## capitalBitName(const AbstractLocker&, size_t index) const { return m_ ## lowerBitName[index]; } \
    bool is ## capitalBitName(const AbstractLocker& locker, MarkedBlock::Handle* block) const { return is ## capitalBitName(locker, block->index()); } \
    void setIs ## capitalBitName(const AbstractLocker&, size_t index, bool value) { m_ ## lowerBitName[index] = value; } \
    void setIs ## capitalBitName(const AbstractLocker& locker, MarkedBlock::Handle* block, bool value) { setIs ## capitalBitName(locker, block->index(), value); }
    FOR_EACH_MARKED_ALLOCATOR_BIT(MARKED_ALLOCATOR_BIT_ACCESSORS)
#undef MARKED_ALLOCATOR_BIT_ACCESSORS

    template<typename Func>
    void forEachBitVector(const AbstractLocker&, const Func& func)
    {
#define MARKED_ALLOCATOR_BIT_CALLBACK(lowerBitName, capitalBitName) \
        func(m_ ## lowerBitName);
        FOR_EACH_MARKED_ALLOCATOR_BIT(MARKED_ALLOCATOR_BIT_CALLBACK);
#undef MARKED_ALLOCATOR_BIT_CALLBACK
    }
    
    template<typename Func>
    void forEachBitVectorWithName(const AbstractLocker&, const Func& func)
    {
#define MARKED_ALLOCATOR_BIT_CALLBACK(lowerBitName, capitalBitName) \
        func(m_ ## lowerBitName, #capitalBitName);
        FOR_EACH_MARKED_ALLOCATOR_BIT(MARKED_ALLOCATOR_BIT_CALLBACK);
#undef MARKED_ALLOCATOR_BIT_CALLBACK
    }
    
    MarkedAllocator* nextAllocator() const { return m_nextAllocator; }
    MarkedAllocator* nextAllocatorInSubspace() const { return m_nextAllocatorInSubspace; }
    
    void setNextAllocator(MarkedAllocator* allocator) { m_nextAllocator = allocator; }
    void setNextAllocatorInSubspace(MarkedAllocator* allocator) { m_nextAllocatorInSubspace = allocator; }
    
    MarkedBlock::Handle* findEmptyBlockToSteal();
    
    MarkedBlock::Handle* findBlockToSweep();
    
    Subspace* subspace() const { return m_subspace; }
    MarkedSpace& markedSpace() const;
    
    void dump(PrintStream&) const;
    void dumpBits(PrintStream& = WTF::dataFile());
    
private:
    friend class MarkedBlock;
    
    bool shouldStealEmptyBlocksFromOtherAllocators() const;
    
    JS_EXPORT_PRIVATE void* allocateSlowCase(GCDeferralContext*);
    JS_EXPORT_PRIVATE void* tryAllocateSlowCase(GCDeferralContext*);
    void* allocateSlowCaseImpl(GCDeferralContext*, bool crashOnFailure);
    void didConsumeFreeList();
    void* tryAllocateWithoutCollecting();
    MarkedBlock::Handle* tryAllocateBlock();
    void* tryAllocateIn(MarkedBlock::Handle*);
    void* allocateIn(MarkedBlock::Handle*);
    ALWAYS_INLINE void doTestCollectionsIfNeeded(GCDeferralContext*);
    
    void setFreeList(const FreeList&);
    
    FreeList m_freeList;
    
    Vector<MarkedBlock::Handle*> m_blocks;
    Vector<unsigned> m_freeBlockIndices;

    // Mutator uses this to guard resizing the bitvectors. Those things in the GC that may run
    // concurrently to the mutator must lock this when accessing the bitvectors.
    Lock m_bitvectorLock;
#define MARKED_ALLOCATOR_BIT_DECLARATION(lowerBitName, capitalBitName) \
    FastBitVector m_ ## lowerBitName;
    FOR_EACH_MARKED_ALLOCATOR_BIT(MARKED_ALLOCATOR_BIT_DECLARATION)
#undef MARKED_ALLOCATOR_BIT_DECLARATION
    
    // After you do something to a block based on one of these cursors, you clear the bit in the
    // corresponding bitvector and leave the cursor where it was.
    size_t m_allocationCursor { 0 }; // Points to the next block that is a candidate for allocation.
    size_t m_emptyCursor { 0 }; // Points to the next block that is a candidate for empty allocation (allocating in empty blocks).
    size_t m_unsweptCursor { 0 }; // Points to the next block that is a candidate for incremental sweeping.
    
    MarkedBlock::Handle* m_currentBlock;
    MarkedBlock::Handle* m_lastActiveBlock;

    Lock m_lock;
    unsigned m_cellSize;
    AllocatorAttributes m_attributes;
    // FIXME: All of these should probably be references.
    // https://bugs.webkit.org/show_bug.cgi?id=166988
    Heap* m_heap;
    Subspace* m_subspace;
    MarkedAllocator* m_nextAllocator { nullptr };
    MarkedAllocator* m_nextAllocatorInSubspace { nullptr };
};

inline ptrdiff_t MarkedAllocator::offsetOfFreeList()
{
    return OBJECT_OFFSETOF(MarkedAllocator, m_freeList);
}

inline ptrdiff_t MarkedAllocator::offsetOfCellSize()
{
    return OBJECT_OFFSETOF(MarkedAllocator, m_cellSize);
}

} // namespace JSC
