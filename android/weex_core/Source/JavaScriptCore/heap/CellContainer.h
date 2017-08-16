/*
 * Copyright (C) 2016 Apple Inc. All rights reserved.
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

#pragma once

#include <wtf/StdLibExtras.h>

namespace JSC {

class Heap;
class HeapCell;
class LargeAllocation;
class MarkedBlock;
class WeakSet;
class VM;

typedef uint32_t HeapVersion;

// This is how we abstract over either MarkedBlock& or LargeAllocation&. Put things in here as you
// find need for them.

class CellContainer {
public:
    CellContainer()
        : m_encodedPointer(0)
    {
    }
    
    CellContainer(MarkedBlock& markedBlock)
        : m_encodedPointer(bitwise_cast<uintptr_t>(&markedBlock))
    {
    }
    
    CellContainer(LargeAllocation& largeAllocation)
        : m_encodedPointer(bitwise_cast<uintptr_t>(&largeAllocation) | isLargeAllocationBit)
    {
    }
    
    VM* vm() const;
    Heap* heap() const;
    
    explicit operator bool() const { return !!m_encodedPointer; }
    
    bool isMarkedBlock() const { return m_encodedPointer && !(m_encodedPointer & isLargeAllocationBit); }
    bool isLargeAllocation() const { return m_encodedPointer & isLargeAllocationBit; }
    
    MarkedBlock& markedBlock() const
    {
        ASSERT(isMarkedBlock());
        return *bitwise_cast<MarkedBlock*>(m_encodedPointer);
    }
    
    LargeAllocation& largeAllocation() const
    {
        ASSERT(isLargeAllocation());
        return *bitwise_cast<LargeAllocation*>(m_encodedPointer - isLargeAllocationBit);
    }
    
    void aboutToMark(HeapVersion markingVersion);
    bool areMarksStale() const;
    
    bool isMarked(HeapCell*) const;
    bool isMarked(HeapVersion markingVersion, HeapCell*) const;
    
    bool isNewlyAllocated(HeapCell*) const;
    
    void noteMarked();
    void assertValidCell(VM&, HeapCell*) const;
    
    size_t cellSize() const;
    
    WeakSet& weakSet() const;
    
private:
    static const uintptr_t isLargeAllocationBit = 1;
    uintptr_t m_encodedPointer;
};

} // namespace JSC

