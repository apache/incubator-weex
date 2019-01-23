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
#include "WeakImpl.h"
#include <wtf/DoublyLinkedList.h>
#include <wtf/StdLibExtras.h>

namespace JSC {

class Heap;
class SlotVisitor;

class WeakBlock : public DoublyLinkedListNode<WeakBlock> {
public:
    friend class WTF::DoublyLinkedListNode<WeakBlock>;
    static const size_t blockSize = 256; // 1/16 of MarkedBlock size

    struct FreeCell {
        FreeCell* next;
    };

    struct SweepResult {
        bool isNull() const;

        bool blockIsFree { true };
        bool blockIsLogicallyEmpty { true };
        FreeCell* freeList { nullptr };
    };

    static WeakBlock* create(Heap&, CellContainer);
    static void destroy(Heap&, WeakBlock*);

    static WeakImpl* asWeakImpl(FreeCell*);

    bool isEmpty();
    bool isLogicallyEmptyButNotFree() const;

    void sweep();
    SweepResult takeSweepResult();

    void visit(SlotVisitor&);

    void reap();

    void lastChanceToFinalize();
    void disconnectContainer() { m_container = CellContainer(); }

private:
    static FreeCell* asFreeCell(WeakImpl*);
    
    template<typename ContainerType>
    void specializedVisit(ContainerType&, SlotVisitor&);

    explicit WeakBlock(CellContainer);
    void finalize(WeakImpl*);
    WeakImpl* weakImpls();
    size_t weakImplCount();
    void addToFreeList(FreeCell**, WeakImpl*);

    CellContainer m_container;
    WeakBlock* m_prev;
    WeakBlock* m_next;
    SweepResult m_sweepResult;
};

inline bool WeakBlock::SweepResult::isNull() const
{
    return blockIsFree && !freeList; // This state is impossible, so we can use it to mean null.
}

inline WeakImpl* WeakBlock::asWeakImpl(FreeCell* freeCell)
{
    return reinterpret_cast_ptr<WeakImpl*>(freeCell);
}

inline WeakBlock::SweepResult WeakBlock::takeSweepResult()
{
    SweepResult tmp;
    std::swap(tmp, m_sweepResult);
    ASSERT(m_sweepResult.isNull());
    return tmp;
}

inline WeakBlock::FreeCell* WeakBlock::asFreeCell(WeakImpl* weakImpl)
{
    return reinterpret_cast_ptr<FreeCell*>(weakImpl);
}

inline WeakImpl* WeakBlock::weakImpls()
{
    return reinterpret_cast_ptr<WeakImpl*>(this) + ((sizeof(WeakBlock) + sizeof(WeakImpl) - 1) / sizeof(WeakImpl));
}

inline size_t WeakBlock::weakImplCount()
{
    return (blockSize / sizeof(WeakImpl)) - ((sizeof(WeakBlock) + sizeof(WeakImpl) - 1) / sizeof(WeakImpl));
}

inline void WeakBlock::addToFreeList(FreeCell** freeList, WeakImpl* weakImpl)
{
    ASSERT(weakImpl->state() == WeakImpl::Deallocated);
    FreeCell* freeCell = asFreeCell(weakImpl);
    ASSERT(!*freeList || ((char*)*freeList > (char*)this && (char*)*freeList < (char*)this + blockSize));
    ASSERT((char*)freeCell > (char*)this && (char*)freeCell < (char*)this + blockSize);
    freeCell->next = *freeList;
    *freeList = freeCell;
}

inline bool WeakBlock::isEmpty()
{
    return !m_sweepResult.isNull() && m_sweepResult.blockIsFree;
}

inline bool WeakBlock::isLogicallyEmptyButNotFree() const
{
    return !m_sweepResult.isNull() && !m_sweepResult.blockIsFree && m_sweepResult.blockIsLogicallyEmpty;
}

} // namespace JSC
