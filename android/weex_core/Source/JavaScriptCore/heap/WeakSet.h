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

#pragma once

#include "CellContainer.h"
#include "WeakBlock.h"
#include <wtf/SentinelLinkedList.h>

namespace JSC {

class Heap;
class WeakImpl;

class WeakSet : public BasicRawSentinelNode<WeakSet> {
    friend class LLIntOffsetsExtractor;

public:
    static WeakImpl* allocate(JSValue, WeakHandleOwner* = 0, void* context = 0);
    static void deallocate(WeakImpl*);

    WeakSet(VM*, CellContainer);
    ~WeakSet();
    void lastChanceToFinalize();
    
    CellContainer container() const { return m_container; }
    void setContainer(CellContainer container) { m_container = container; }

    Heap* heap() const;
    VM* vm() const;

    bool isEmpty() const;

    void visit(SlotVisitor&);

    void reap();
    void sweep();
    void shrink();
    void resetAllocator();

private:
    JS_EXPORT_PRIVATE WeakBlock::FreeCell* findAllocator();
    WeakBlock::FreeCell* tryFindAllocator();
    WeakBlock::FreeCell* addAllocator();
    void removeAllocator(WeakBlock*);

    WeakBlock::FreeCell* m_allocator;
    WeakBlock* m_nextAllocator;
    DoublyLinkedList<WeakBlock> m_blocks;
    VM* m_vm;
    CellContainer m_container;
};

inline WeakSet::WeakSet(VM* vm, CellContainer container)
    : m_allocator(0)
    , m_nextAllocator(0)
    , m_vm(vm)
    , m_container(container)
{
}

inline VM* WeakSet::vm() const
{
    return m_vm;
}

inline bool WeakSet::isEmpty() const
{
    for (WeakBlock* block = m_blocks.head(); block; block = block->next()) {
        if (!block->isEmpty())
            return false;
    }

    return true;
}

inline void WeakSet::deallocate(WeakImpl* weakImpl)
{
    weakImpl->setState(WeakImpl::Deallocated);
}

inline void WeakSet::lastChanceToFinalize()
{
    for (WeakBlock* block = m_blocks.head(); block; block = block->next())
        block->lastChanceToFinalize();
}

inline void WeakSet::visit(SlotVisitor& visitor)
{
    for (WeakBlock* block = m_blocks.head(); block; block = block->next())
        block->visit(visitor);
}

inline void WeakSet::reap()
{
    for (WeakBlock* block = m_blocks.head(); block; block = block->next())
        block->reap();
}

inline void WeakSet::resetAllocator()
{
    m_allocator = 0;
    m_nextAllocator = m_blocks.head();
}

} // namespace JSC
