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

