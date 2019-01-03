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

#include "MarkedAllocator.h"

namespace JSC {

ALWAYS_INLINE void* MarkedAllocator::tryAllocate(GCDeferralContext* deferralContext)
{
    unsigned remaining = m_freeList.remaining;
    if (remaining) {
        unsigned cellSize = m_cellSize;
        remaining -= cellSize;
        m_freeList.remaining = remaining;
        return m_freeList.payloadEnd - remaining - cellSize;
    }
    
    FreeCell* head = m_freeList.head;
    if (UNLIKELY(!head))
        return tryAllocateSlowCase(deferralContext);
    
    m_freeList.head = head->next;
    return head;
}

ALWAYS_INLINE void* MarkedAllocator::allocate(GCDeferralContext* deferralContext)
{
    unsigned remaining = m_freeList.remaining;
    if (remaining) {
        unsigned cellSize = m_cellSize;
        remaining -= cellSize;
        m_freeList.remaining = remaining;
        return m_freeList.payloadEnd - remaining - cellSize;
    }
    
    FreeCell* head = m_freeList.head;
    if (UNLIKELY(!head))
        return allocateSlowCase(deferralContext);
    
    m_freeList.head = head->next;
    return head;
}

template <typename Functor> inline void MarkedAllocator::forEachBlock(const Functor& functor)
{
    m_live.forEachSetBit(
        [&] (size_t index) {
            functor(m_blocks[index]);
        });
}

template <typename Functor> inline void MarkedAllocator::forEachNotEmptyBlock(const Functor& functor)
{
    m_markingNotEmpty.forEachSetBit(
        [&] (size_t index) {
            functor(m_blocks[index]);
        });
}

} // namespace JSC

