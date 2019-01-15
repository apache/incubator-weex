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

#include "MarkedBlock.h"
#include "WeakSet.h"

namespace JSC {

class SlotVisitor;

// WebKit has a good malloc that already knows what to do for large allocations. The GC shouldn't
// have to think about such things. That's where LargeAllocation comes in. We will allocate large
// objects directly using malloc, and put the LargeAllocation header just before them. We can detect
// when a HeapCell* is a LargeAllocation because it will have the MarkedBlock::atomSize / 2 bit set.

class LargeAllocation : public BasicRawSentinelNode<LargeAllocation> {
public:
    static LargeAllocation* tryCreate(Heap&, size_t, Subspace*);
    
    ~LargeAllocation();
    
    static LargeAllocation* fromCell(const void* cell)
    {
        return bitwise_cast<LargeAllocation*>(bitwise_cast<char*>(cell) - headerSize());
    }
    
    HeapCell* cell() const
    {
        return bitwise_cast<HeapCell*>(bitwise_cast<char*>(this) + headerSize());
    }
    
    static bool isLargeAllocation(HeapCell* cell)
    {
        return bitwise_cast<uintptr_t>(cell) & halfAlignment;
    }
    
    void lastChanceToFinalize();
    
    Heap* heap() const { return m_weakSet.heap(); }
    VM* vm() const { return m_weakSet.vm(); }
    WeakSet& weakSet() { return m_weakSet; }
    
    void shrink();
    
    void visitWeakSet(SlotVisitor&);
    void reapWeakSet();
    
    void clearNewlyAllocated() { m_isNewlyAllocated = false; }
    void flip();
    
    bool isNewlyAllocated() const { return m_isNewlyAllocated; }
    ALWAYS_INLINE bool isMarked() { return m_isMarked.load(std::memory_order_relaxed); }
    ALWAYS_INLINE bool isMarked(HeapCell*) { return isMarked(); }
    ALWAYS_INLINE bool isMarked(HeapCell*, Dependency) { return isMarked(); }
    ALWAYS_INLINE bool isMarkedConcurrently(HeapVersion, HeapCell*) { return isMarked(); }
    bool isLive() { return isMarked() || isNewlyAllocated(); }
    
    bool hasValidCell() const { return m_hasValidCell; }
    
    bool isEmpty();
    
    size_t cellSize() const { return m_cellSize; }
    
    bool aboveLowerBound(const void* rawPtr)
    {
        char* ptr = bitwise_cast<char*>(rawPtr);
        char* begin = bitwise_cast<char*>(cell());
        return ptr >= begin;
    }
    
    bool belowUpperBound(const void* rawPtr)
    {
        char* ptr = bitwise_cast<char*>(rawPtr);
        char* begin = bitwise_cast<char*>(cell());
        char* end = begin + cellSize();
        // We cannot #include IndexingHeader.h because reasons. The fact that IndexingHeader is 8
        // bytes is wired deep into our engine, so this isn't so bad.
        size_t sizeOfIndexingHeader = 8;
        return ptr <= end + sizeOfIndexingHeader;
    }
    
    bool contains(const void* rawPtr)
    {
        return aboveLowerBound(rawPtr) && belowUpperBound(rawPtr);
    }
    
    const AllocatorAttributes& attributes() const { return m_attributes; }
    
    Dependency aboutToMark(HeapVersion) { return nullDependency(); }
    
    ALWAYS_INLINE bool testAndSetMarked()
    {
        // This method is usually called when the object is already marked. This avoids us
        // having to CAS in that case. It's profitable to reduce the total amount of CAS
        // traffic.
        if (isMarked())
            return true;
        return m_isMarked.compareExchangeStrong(false, true);
    }
    ALWAYS_INLINE bool testAndSetMarked(HeapCell*, Dependency, TransactionAbortLikelihood = TransactionAbortLikelihood::Likely) { return testAndSetMarked(); }
    void clearMarked() { m_isMarked.store(false); }
    
    void noteMarked() { }
    
#if ASSERT_DISABLED
    void assertValidCell(VM&, HeapCell*) const { }
#else
    void assertValidCell(VM&, HeapCell*) const;
#endif
    
    void sweep();
    
    void destroy();
    
    void dump(PrintStream&) const;
    
private:
    LargeAllocation(Heap&, size_t, Subspace*);
    
    static const unsigned alignment = MarkedBlock::atomSize;
    static const unsigned halfAlignment = alignment / 2;

    static unsigned headerSize();
    
    size_t m_cellSize;
    bool m_isNewlyAllocated;
    bool m_hasValidCell;
    Atomic<bool> m_isMarked;
    AllocatorAttributes m_attributes;
    Subspace* m_subspace;
    WeakSet m_weakSet;
};

inline unsigned LargeAllocation::headerSize()
{
    return ((sizeof(LargeAllocation) + halfAlignment - 1) & ~(halfAlignment - 1)) | halfAlignment;
}

} // namespace JSC

