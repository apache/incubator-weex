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

#include "MarkedBlockInlines.h"
#include "MarkedSpace.h"

namespace JSC {

template<typename Functor> inline void MarkedSpace::forEachLiveCell(HeapIterationScope&, const Functor& functor)
{
    ASSERT(isIterating());
    forEachLiveCell(functor);
}

template<typename Functor> inline void MarkedSpace::forEachLiveCell(const Functor& functor)
{
    BlockIterator end = m_blocks.set().end();
    for (BlockIterator it = m_blocks.set().begin(); it != end; ++it) {
        if ((*it)->handle().forEachLiveCell(functor) == IterationStatus::Done)
            return;
    }
    for (LargeAllocation* allocation : m_largeAllocations) {
        if (allocation->isLive()) {
            if (functor(allocation->cell(), allocation->attributes().cellKind) == IterationStatus::Done)
                return;
        }
    }
}

template<typename Functor> inline void MarkedSpace::forEachDeadCell(HeapIterationScope&, const Functor& functor)
{
    ASSERT(isIterating());
    BlockIterator end = m_blocks.set().end();
    for (BlockIterator it = m_blocks.set().begin(); it != end; ++it) {
        if ((*it)->handle().forEachDeadCell(functor) == IterationStatus::Done)
            return;
    }
    for (LargeAllocation* allocation : m_largeAllocations) {
        if (!allocation->isLive()) {
            if (functor(allocation->cell(), allocation->attributes().cellKind) == IterationStatus::Done)
                return;
        }
    }
}

} // namespace JSC

