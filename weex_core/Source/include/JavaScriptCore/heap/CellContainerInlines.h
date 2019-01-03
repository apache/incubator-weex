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
#include "JSCell.h"
#include "LargeAllocation.h"
#include "MarkedBlock.h"
#include "VM.h"

namespace JSC {

inline VM* CellContainer::vm() const
{
    if (isLargeAllocation())
        return largeAllocation().vm();
    return markedBlock().vm();
}

inline Heap* CellContainer::heap() const
{
    return &vm()->heap;
}

inline bool CellContainer::isMarked(HeapCell* cell) const
{
    if (isLargeAllocation())
        return largeAllocation().isMarked();
    return markedBlock().isMarked(cell);
}

inline bool CellContainer::isMarked(HeapVersion markingVersion, HeapCell* cell) const
{
    if (isLargeAllocation())
        return largeAllocation().isMarked();
    return markedBlock().isMarked(markingVersion, cell);
}

inline void CellContainer::noteMarked()
{
    if (!isLargeAllocation())
        markedBlock().noteMarked();
}

inline void CellContainer::assertValidCell(VM& vm, HeapCell* cell) const
{
    if (isLargeAllocation())
        largeAllocation().assertValidCell(vm, cell);
    else
        markedBlock().assertValidCell(vm, cell);
}

inline size_t CellContainer::cellSize() const
{
    if (isLargeAllocation())
        return largeAllocation().cellSize();
    return markedBlock().cellSize();
}

inline WeakSet& CellContainer::weakSet() const
{
    if (isLargeAllocation())
        return largeAllocation().weakSet();
    return markedBlock().weakSet();
}

inline void CellContainer::aboutToMark(HeapVersion markingVersion)
{
    if (!isLargeAllocation())
        markedBlock().aboutToMark(markingVersion);
}

inline bool CellContainer::areMarksStale() const
{
    if (isLargeAllocation())
        return false;
    return markedBlock().areMarksStale();
}

} // namespace JSC

