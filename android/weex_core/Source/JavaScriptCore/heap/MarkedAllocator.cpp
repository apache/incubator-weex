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
#include "MarkedAllocator.h"

#include "AllocatingScope.h"
#include "GCActivityCallback.h"
#include "Heap.h"
#include "IncrementalSweeper.h"
#include "JSCInlines.h"
#include "MarkedAllocatorInlines.h"
#include "MarkedBlockInlines.h"
#include "SuperSampler.h"
#include "VM.h"
#include <wtf/CurrentTime.h>

namespace JSC {

MarkedAllocator::MarkedAllocator(Heap* heap, Subspace* subspace, size_t cellSize)
    : m_currentBlock(0)
    , m_lastActiveBlock(0)
    , m_cellSize(static_cast<unsigned>(cellSize))
    , m_attributes(subspace->attributes())
    , m_heap(heap)
    , m_subspace(subspace)
{
}

bool MarkedAllocator::isPagedOut(double deadline)
{
    unsigned itersSinceLastTimeCheck = 0;
    for (auto* block : m_blocks) {
        if (block)
            block->block().updateNeedsDestruction();
        ++itersSinceLastTimeCheck;
        if (itersSinceLastTimeCheck >= Heap::s_timeCheckResolution) {
            double currentTime = WTF::monotonicallyIncreasingTime();
            if (currentTime > deadline)
                return true;
            itersSinceLastTimeCheck = 0;
        }
    }
    return false;
}

bool MarkedAllocator::shouldStealEmptyBlocksFromOtherAllocators() const
{
    return !needsDestruction();
}

MarkedBlock::Handle* MarkedAllocator::findEmptyBlockToSteal()
{
    // Don't allow others to steal from us, if we wouldn't steal from others.
    if (!shouldStealEmptyBlocksFromOtherAllocators())
        return nullptr;
    
    m_emptyCursor = m_empty.findBit(m_emptyCursor, true);
    if (m_emptyCursor >= m_blocks.size())
        return nullptr;
    return m_blocks[m_emptyCursor];
}

void MarkedAllocator::didConsumeFreeList()
{
    if (m_currentBlock)
        m_currentBlock->didConsumeFreeList();
    
    setFreeList(FreeList());
    m_currentBlock = nullptr;
}

void* MarkedAllocator::tryAllocateWithoutCollecting()
{
    SuperSamplerScope superSamplerScope(false);
    
    ASSERT(!m_currentBlock);
    ASSERT(!m_freeList);
    
    for (;;) {
        m_allocationCursor = (m_canAllocateButNotEmpty | m_empty).findBit(m_allocationCursor, true);
        if (m_allocationCursor >= m_blocks.size())
            break;
        
        setIsCanAllocateButNotEmpty(NoLockingNecessary, m_allocationCursor, false);

        if (void* result = tryAllocateIn(m_blocks[m_allocationCursor]))
            return result;
    }
    
    if (Options::stealEmptyBlocksFromOtherAllocators()
        && shouldStealEmptyBlocksFromOtherAllocators()) {
        if (MarkedBlock::Handle* block = markedSpace().findEmptyBlockToSteal()) {
            block->sweep();
            
            // It's good that this clears canAllocateButNotEmpty as well as all other bits,
            // because there is a remote chance that a block may have both canAllocateButNotEmpty
            // and empty set at the same time.
            block->removeFromAllocator();
            addBlock(block);
            return allocateIn(block);
        }
    }
    
    return nullptr;
}

void* MarkedAllocator::allocateIn(MarkedBlock::Handle* block)
{
    void* result = tryAllocateIn(block);
    RELEASE_ASSERT(result);
    return result;
}

void* MarkedAllocator::tryAllocateIn(MarkedBlock::Handle* block)
{
    ASSERT(block);
    ASSERT(!block->isFreeListed());
    
    FreeList freeList = block->sweep(MarkedBlock::Handle::SweepToFreeList);
    
    // It's possible to stumble on a completely full block. Marking tries to retire these, but
    // that algorithm is racy and may forget to do it sometimes.
    if (freeList.allocationWillFail()) {
        ASSERT(block->isFreeListed());
        block->unsweepWithNoNewlyAllocated();
        ASSERT(!block->isFreeListed());
        ASSERT(!isEmpty(NoLockingNecessary, block));
        ASSERT(!isCanAllocateButNotEmpty(NoLockingNecessary, block));
        return nullptr;
    }
    
    m_currentBlock = block;
    setFreeList(freeList);
    
    void* result;
    if (m_freeList.remaining) {
        unsigned cellSize = m_cellSize;
        m_freeList.remaining -= cellSize;
        result = m_freeList.payloadEnd - m_freeList.remaining - cellSize;
    } else {
        FreeCell* head = m_freeList.head;
        m_freeList.head = head->next;
        result = head;
    }
    RELEASE_ASSERT(result);
    setIsEden(NoLockingNecessary, m_currentBlock, true);
    markedSpace().didAllocateInBlock(m_currentBlock);
    return result;
}

ALWAYS_INLINE void MarkedAllocator::doTestCollectionsIfNeeded(GCDeferralContext* deferralContext)
{
    if (!Options::slowPathAllocsBetweenGCs())
        return;

    static unsigned allocationCount = 0;
    if (!allocationCount) {
        if (!m_heap->isDeferred()) {
            if (deferralContext)
                deferralContext->m_shouldGC = true;
            else
                m_heap->collectAllGarbage();
        }
    }
    if (++allocationCount >= Options::slowPathAllocsBetweenGCs())
        allocationCount = 0;
}

void* MarkedAllocator::allocateSlowCase(GCDeferralContext* deferralContext)
{
    bool crashOnFailure = true;
    return allocateSlowCaseImpl(deferralContext, crashOnFailure);
}

void* MarkedAllocator::tryAllocateSlowCase(GCDeferralContext* deferralContext)
{
    bool crashOnFailure = false;
    return allocateSlowCaseImpl(deferralContext, crashOnFailure);
}

void* MarkedAllocator::allocateSlowCaseImpl(GCDeferralContext* deferralContext, bool crashOnFailure)
{
    SuperSamplerScope superSamplerScope(false);
    ASSERT(m_heap->vm()->currentThreadIsHoldingAPILock());
    doTestCollectionsIfNeeded(deferralContext);

    ASSERT(!markedSpace().isIterating());
    m_heap->didAllocate(m_freeList.originalSize);
    
    didConsumeFreeList();
    
    AllocatingScope helpingHeap(*m_heap);

    m_heap->collectIfNecessaryOrDefer(deferralContext);
    
    // Goofy corner case: the GC called a callback and now this allocator has a currentBlock. This only
    // happens when running WebKit tests, which inject a callback into the GC's finalization.
    if (UNLIKELY(m_currentBlock)) {
        if (crashOnFailure)
            return allocate(deferralContext);
        return tryAllocate(deferralContext);
    }
    
    void* result = tryAllocateWithoutCollecting();
    
    if (LIKELY(result != 0))
        return result;
    
    MarkedBlock::Handle* block = tryAllocateBlock();
    if (!block) {
        if (crashOnFailure)
            RELEASE_ASSERT_NOT_REACHED();
        else
            return nullptr;
    }
    addBlock(block);
    result = allocateIn(block);
    ASSERT(result);
    return result;
}

static size_t blockHeaderSize()
{
    return WTF::roundUpToMultipleOf<MarkedBlock::atomSize>(sizeof(MarkedBlock));
}

size_t MarkedAllocator::blockSizeForBytes(size_t bytes)
{
    size_t minBlockSize = MarkedBlock::blockSize;
    size_t minAllocationSize = blockHeaderSize() + WTF::roundUpToMultipleOf<MarkedBlock::atomSize>(bytes);
    minAllocationSize = WTF::roundUpToMultipleOf(WTF::pageSize(), minAllocationSize);
    return std::max(minBlockSize, minAllocationSize);
}

MarkedBlock::Handle* MarkedAllocator::tryAllocateBlock()
{
    SuperSamplerScope superSamplerScope(false);
    
    MarkedBlock::Handle* handle = MarkedBlock::tryCreate(*m_heap);
    if (!handle)
        return nullptr;
    
    markedSpace().didAddBlock(handle);
    
    return handle;
}

void MarkedAllocator::addBlock(MarkedBlock::Handle* block)
{
    size_t index;
    if (m_freeBlockIndices.isEmpty()) {
        index = m_blocks.size();

        size_t oldCapacity = m_blocks.capacity();
        m_blocks.append(block);
        if (m_blocks.capacity() != oldCapacity) {
            forEachBitVector(
                NoLockingNecessary,
                [&] (FastBitVector& vector) {
                    ASSERT_UNUSED(vector, vector.numBits() == oldCapacity);
                });
            
            ASSERT(m_blocks.capacity() > oldCapacity);
            
            LockHolder locker(m_bitvectorLock);
            forEachBitVector(
                locker,
                [&] (FastBitVector& vector) {
                    vector.resize(m_blocks.capacity());
                });
        }
    } else {
        index = m_freeBlockIndices.takeLast();
        ASSERT(!m_blocks[index]);
        m_blocks[index] = block;
    }
    
    forEachBitVector(
        NoLockingNecessary,
        [&] (FastBitVector& vector) {
            ASSERT_UNUSED(vector, !vector[index]);
        });

    // This is the point at which the block learns of its cellSize() and attributes().
    block->didAddToAllocator(this, index);
    
    setIsLive(NoLockingNecessary, index, true);
    setIsEmpty(NoLockingNecessary, index, true);
}

void MarkedAllocator::removeBlock(MarkedBlock::Handle* block)
{
    ASSERT(block->allocator() == this);
    ASSERT(m_blocks[block->index()] == block);

    m_blocks[block->index()] = nullptr;
    m_freeBlockIndices.append(block->index());
    
    forEachBitVector(
        holdLock(m_bitvectorLock),
        [&] (FastBitVector& vector) {
            vector[block->index()] = false;
        });
    
    block->didRemoveFromAllocator();
}

void MarkedAllocator::stopAllocating()
{
    if (false)
        dataLog(RawPointer(this), ": MarkedAllocator::stopAllocating!\n");
    ASSERT(!m_lastActiveBlock);
    if (!m_currentBlock) {
        ASSERT(!m_freeList);
        return;
    }
    
    m_currentBlock->stopAllocating(m_freeList);
    m_lastActiveBlock = m_currentBlock;
    m_currentBlock = 0;
    m_freeList = FreeList();
}

void MarkedAllocator::prepareForAllocation()
{
    m_lastActiveBlock = nullptr;
    m_currentBlock = nullptr;
    setFreeList(FreeList());

    m_allocationCursor = 0;
    m_emptyCursor = 0;
    m_unsweptCursor = 0;
    
    m_eden.clearAll();

    if (UNLIKELY(Options::useImmortalObjects())) {
        // FIXME: Make this work again.
        // https://bugs.webkit.org/show_bug.cgi?id=162296
        RELEASE_ASSERT_NOT_REACHED();
    }
}

void MarkedAllocator::lastChanceToFinalize()
{
    forEachBlock(
        [&] (MarkedBlock::Handle* block) {
            block->lastChanceToFinalize();
        });
}

void MarkedAllocator::setFreeList(const FreeList& freeList)
{
    m_freeList = freeList;
}

void MarkedAllocator::resumeAllocating()
{
    if (!m_lastActiveBlock)
        return;

    m_freeList = m_lastActiveBlock->resumeAllocating();
    m_currentBlock = m_lastActiveBlock;
    m_lastActiveBlock = nullptr;
}

void MarkedAllocator::beginMarkingForFullCollection()
{
    // Mark bits are sticky and so is our summary of mark bits. We only clear these during full
    // collections, so if you survived the last collection you will survive the next one so long
    // as the next one is eden.
    m_markingNotEmpty.clearAll();
    m_markingRetired.clearAll();
}

void MarkedAllocator::endMarking()
{
    m_allocated.clearAll();
    
    // It's surprising and frustrating to comprehend, but the end-of-marking flip does not need to
    // know what kind of collection it is. That knowledge is already encoded in the m_markingXYZ
    // vectors.
    
    if (needsDestruction()) {
        // If blocks need destruction then nothing is empty! This is a correct assertion but may
        // become wrong once we go full concurrent: when we create a new block, it will flicker
        // into the empty set for a tiny moment. On the other hand, this code is likely to be run
        // in stopTheWorld.
        ASSERT(m_empty.isEmpty());
        m_canAllocateButNotEmpty = m_live & ~m_markingRetired;
        return;
    }
    
    m_empty = m_live & ~m_markingNotEmpty;
    m_canAllocateButNotEmpty = m_live & m_markingNotEmpty & ~m_markingRetired;
    
    if (false) {
        dataLog("Bits for ", m_cellSize, ", ", m_attributes, " after endMarking:\n");
        dumpBits(WTF::dataFile());
    }
}

void MarkedAllocator::snapshotUnsweptForEdenCollection()
{
    m_unswept |= m_eden;
}

void MarkedAllocator::snapshotUnsweptForFullCollection()
{
    m_unswept = m_live;
}

MarkedBlock::Handle* MarkedAllocator::findBlockToSweep()
{
    m_unsweptCursor = m_unswept.findBit(m_unsweptCursor, true);
    if (m_unsweptCursor >= m_blocks.size())
        return nullptr;
    return m_blocks[m_unsweptCursor];
}

void MarkedAllocator::sweep()
{
    m_unswept.forEachSetBit(
        [&] (size_t index) {
            MarkedBlock::Handle* block = m_blocks[index];
            block->sweep();
        });
}

void MarkedAllocator::shrink()
{
    m_empty.forEachSetBit(
        [&] (size_t index) {
            markedSpace().freeBlock(m_blocks[index]);
        });
}

void MarkedAllocator::assertNoUnswept()
{
    if (ASSERT_DISABLED)
        return;
    
    if (m_unswept.isEmpty())
        return;
    
    dataLog("Assertion failed: unswept not empty in ", *this, ".\n");
    dumpBits();
    ASSERT_NOT_REACHED();
}

void MarkedAllocator::dump(PrintStream& out) const
{
    out.print(RawPointer(this), ":", m_cellSize, "/", m_attributes);
}

void MarkedAllocator::dumpBits(PrintStream& out)
{
    unsigned maxNameLength = 0;
    forEachBitVectorWithName(
        NoLockingNecessary,
        [&] (FastBitVector&, const char* name) {
            unsigned length = strlen(name);
            maxNameLength = std::max(maxNameLength, length);
        });
    
    forEachBitVectorWithName(
        NoLockingNecessary,
        [&] (FastBitVector& vector, const char* name) {
            out.print("    ", name, ": ");
            for (unsigned i = maxNameLength - strlen(name); i--;)
                out.print(" ");
            out.print(vector, "\n");
        });
}

MarkedSpace& MarkedAllocator::markedSpace() const
{
    return m_subspace->space();
}

} // namespace JSC

