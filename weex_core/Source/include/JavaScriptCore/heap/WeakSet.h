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
