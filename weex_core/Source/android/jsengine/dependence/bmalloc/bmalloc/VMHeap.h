/*
 * Copyright (C) 2014-2016 Apple Inc. All rights reserved.
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
#ifndef VMHeap_h
#define VMHeap_h

#include "Chunk.h"
#include "FixedVector.h"
#include "LargeRange.h"
#include "Map.h"
#include "Vector.h"
#if BOS(DARWIN)
#include "Zone.h"
#endif

namespace bmalloc {

class BeginTag;
class EndTag;
class Heap;

typedef enum { Sync, Async } ScavengeMode;

class VMHeap {
public:
    SmallPage* allocateSmallPage(std::lock_guard<StaticMutex>&, size_t);
    void deallocateSmallPage(std::unique_lock<StaticMutex>&, size_t, SmallPage*);

    LargeRange tryAllocateLargeChunk(std::lock_guard<StaticMutex>&, size_t alignment, size_t);
    
private:
    void allocateSmallChunk(std::lock_guard<StaticMutex>&, size_t);

    std::array<List<SmallPage>, pageClassCount> m_smallPages;
    
#if BOS(DARWIN)
    Zone m_zone;
#endif
};

inline SmallPage* VMHeap::allocateSmallPage(std::lock_guard<StaticMutex>& lock, size_t pageClass)
{
    if (m_smallPages[pageClass].isEmpty())
        allocateSmallChunk(lock, pageClass);

    SmallPage* page = m_smallPages[pageClass].pop();
    vmAllocatePhysicalPagesSloppy(page->begin()->begin(), pageSize(pageClass));
    return page;
}

inline void VMHeap::deallocateSmallPage(std::unique_lock<StaticMutex>& lock, size_t pageClass, SmallPage* page)
{
    lock.unlock();
    vmDeallocatePhysicalPagesSloppy(page->begin()->begin(), pageSize(pageClass));
    lock.lock();
    
    m_smallPages[pageClass].push(page);
}

} // namespace bmalloc

#endif // VMHeap_h
