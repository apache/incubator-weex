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
