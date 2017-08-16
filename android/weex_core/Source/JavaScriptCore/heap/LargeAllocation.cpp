/*
 * Copyright (C) 2016-2017 Apple Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 */

#include "config.h"
#include "LargeAllocation.h"

#include "Heap.h"
#include "JSCInlines.h"
#include "Operations.h"

namespace JSC {

LargeAllocation* LargeAllocation::tryCreate(Heap& heap, size_t size, Subspace* subspace)
{
    void* space = tryFastAlignedMalloc(alignment, headerSize() + size);
    if (!space)
        return nullptr;
    if (scribbleFreeCells())
        scribble(space, size);
    return new (NotNull, space) LargeAllocation(heap, size, subspace);
}

LargeAllocation::LargeAllocation(Heap& heap, size_t size, Subspace* subspace)
    : m_cellSize(size)
    , m_isNewlyAllocated(true)
    , m_hasValidCell(true)
    , m_attributes(subspace->attributes())
    , m_subspace(subspace)
    , m_weakSet(heap.vm(), *this)
{
    m_isMarked.store(0);
}

LargeAllocation::~LargeAllocation()
{
    if (isOnList())
        remove();
}

void LargeAllocation::lastChanceToFinalize()
{
    m_weakSet.lastChanceToFinalize();
    clearMarked();
    clearNewlyAllocated();
    sweep();
}

void LargeAllocation::shrink()
{
    m_weakSet.shrink();
}

void LargeAllocation::visitWeakSet(SlotVisitor& visitor)
{
    m_weakSet.visit(visitor);
}

void LargeAllocation::reapWeakSet()
{
    return m_weakSet.reap();
}

void LargeAllocation::flip()
{
    ASSERT(heap()->collectionScope() == CollectionScope::Full);
    clearMarked();
}

bool LargeAllocation::isEmpty()
{
    return !isMarked() && m_weakSet.isEmpty() && !isNewlyAllocated();
}

void LargeAllocation::sweep()
{
    m_weakSet.sweep();
    
    if (m_hasValidCell && !isLive()) {
        if (m_attributes.destruction == NeedsDestruction)
            m_subspace->destroy(*vm(), static_cast<JSCell*>(cell()));
        m_hasValidCell = false;
    }
}

void LargeAllocation::destroy()
{
    this->~LargeAllocation();
    fastAlignedFree(this);
}

void LargeAllocation::dump(PrintStream& out) const
{
    out.print(RawPointer(this), ":(cell at ", RawPointer(cell()), " with size ", m_cellSize, " and attributes ", m_attributes, ")");
}

#if !ASSERT_DISABLED
void LargeAllocation::assertValidCell(VM& vm, HeapCell* cell) const
{
    ASSERT(&vm == this->vm());
    ASSERT(cell == this->cell());
    ASSERT(m_hasValidCell);
}
#endif

} // namespace JSC

