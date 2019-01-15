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

