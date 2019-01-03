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

#include "CellContainer.h"
#include "HeapCell.h"
#include "LargeAllocation.h"
#include "MarkedBlock.h"
#include "VM.h"

namespace JSC {

ALWAYS_INLINE bool HeapCell::isLargeAllocation() const
{
    return LargeAllocation::isLargeAllocation(const_cast<HeapCell*>(this));
}

ALWAYS_INLINE CellContainer HeapCell::cellContainer() const
{
    if (isLargeAllocation())
        return largeAllocation();
    return markedBlock();
}

ALWAYS_INLINE MarkedBlock& HeapCell::markedBlock() const
{
    return *MarkedBlock::blockFor(this);
}

ALWAYS_INLINE LargeAllocation& HeapCell::largeAllocation() const
{
    return *LargeAllocation::fromCell(const_cast<HeapCell*>(this));
}

ALWAYS_INLINE Heap* HeapCell::heap() const
{
    return &vm()->heap;
}

ALWAYS_INLINE VM* HeapCell::vm() const
{
    if (isLargeAllocation())
        return largeAllocation().vm();
    return markedBlock().vm();
}
    
ALWAYS_INLINE size_t HeapCell::cellSize() const
{
    if (isLargeAllocation())
        return largeAllocation().cellSize();
    return markedBlock().cellSize();
}

ALWAYS_INLINE AllocatorAttributes HeapCell::allocatorAttributes() const
{
    if (isLargeAllocation())
        return largeAllocation().attributes();
    return markedBlock().attributes();
}

ALWAYS_INLINE DestructionMode HeapCell::destructionMode() const
{
    return allocatorAttributes().destruction;
}

ALWAYS_INLINE HeapCell::Kind HeapCell::cellKind() const
{
    return allocatorAttributes().cellKind;
}

} // namespace JSC

