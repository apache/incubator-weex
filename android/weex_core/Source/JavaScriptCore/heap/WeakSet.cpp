/*
 * Copyright (C) 2012, 2016 Apple Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "WeakSet.h"

#include "Heap.h"
#include "JSCInlines.h"
#include "VM.h"

namespace JSC {

WeakSet::~WeakSet()
{
    if (isOnList())
        remove();
    
    Heap& heap = *this->heap();
    WeakBlock* next = 0;
    for (WeakBlock* block = m_blocks.head(); block; block = next) {
        next = block->next();
        WeakBlock::destroy(heap, block);
    }
    m_blocks.clear();
}

void WeakSet::sweep()
{
    for (WeakBlock* block = m_blocks.head(); block;) {
        heap()->sweepNextLogicallyEmptyWeakBlock();

        WeakBlock* nextBlock = block->next();
        block->sweep();
        if (block->isLogicallyEmptyButNotFree()) {
            // If this WeakBlock is logically empty, but still has Weaks pointing into it,
            // we can't destroy it just yet. Detach it from the WeakSet and hand ownership
            // to the Heap so we don't pin down the entire MarkedBlock or LargeAllocation.
            m_blocks.remove(block);
            heap()->addLogicallyEmptyWeakBlock(block);
            block->disconnectContainer();
        }
        block = nextBlock;
    }

    resetAllocator();
}

void WeakSet::shrink()
{
    WeakBlock* next;
    for (WeakBlock* block = m_blocks.head(); block; block = next) {
        next = block->next();

        if (block->isEmpty())
            removeAllocator(block);
    }

    resetAllocator();
    
    if (m_blocks.isEmpty() && isOnList())
        remove();
}

WeakBlock::FreeCell* WeakSet::findAllocator()
{
    if (WeakBlock::FreeCell* allocator = tryFindAllocator())
        return allocator;

    return addAllocator();
}

WeakBlock::FreeCell* WeakSet::tryFindAllocator()
{
    while (m_nextAllocator) {
        WeakBlock* block = m_nextAllocator;
        m_nextAllocator = m_nextAllocator->next();

        WeakBlock::SweepResult sweepResult = block->takeSweepResult();
        if (sweepResult.freeList)
            return sweepResult.freeList;
    }

    return 0;
}

WeakBlock::FreeCell* WeakSet::addAllocator()
{
    if (!isOnList())
        heap()->objectSpace().addActiveWeakSet(this);
    
    WeakBlock* block = WeakBlock::create(*heap(), m_container);
    heap()->didAllocate(WeakBlock::blockSize);
    m_blocks.append(block);
    WeakBlock::SweepResult sweepResult = block->takeSweepResult();
    ASSERT(!sweepResult.isNull() && sweepResult.freeList);
    return sweepResult.freeList;
}

void WeakSet::removeAllocator(WeakBlock* block)
{
    m_blocks.remove(block);
    WeakBlock::destroy(*heap(), block);
}

} // namespace JSC
