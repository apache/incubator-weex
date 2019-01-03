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

#include "JSCell.h"
#include "MarkedAllocator.h"
#include "MarkedBlock.h"
#include "MarkedSpace.h"
#include "Subspace.h"

namespace JSC {

template<typename Func>
void Subspace::forEachMarkedBlock(const Func& func)
{
    for (MarkedAllocator* allocator = m_firstAllocator; allocator; allocator = allocator->nextAllocatorInSubspace())
        allocator->forEachBlock(func);
}

template<typename Func>
void Subspace::forEachNotEmptyMarkedBlock(const Func& func)
{
    for (MarkedAllocator* allocator = m_firstAllocator; allocator; allocator = allocator->nextAllocatorInSubspace())
        allocator->forEachNotEmptyBlock(func);
}

template<typename Func>
void Subspace::forEachLargeAllocation(const Func& func)
{
    for (LargeAllocation* allocation = m_largeAllocations.begin(); allocation != m_largeAllocations.end(); allocation = allocation->next())
        func(allocation);
}

template<typename Func>
void Subspace::forEachMarkedCell(const Func& func)
{
    forEachNotEmptyMarkedBlock(
        [&] (MarkedBlock::Handle* handle) {
            handle->forEachMarkedCell(
                [&] (HeapCell* cell, HeapCell::Kind kind) -> IterationStatus { 
                    func(cell, kind);
                    return IterationStatus::Continue;
                });
        });
    forEachLargeAllocation(
        [&] (LargeAllocation* allocation) {
            if (allocation->isMarked())
                func(allocation->cell(), m_attributes.cellKind);
        });
}

} // namespace JSC

