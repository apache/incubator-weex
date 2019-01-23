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

#if ENABLE(DFG_JIT)

#include "DFGCommon.h"
#include <wtf/StdLibExtras.h>

namespace JSC { namespace DFG {

// Custom pool allocator for exactly one type (type T). It has fast (O(1), only a few
// instructions) allocator, and a similarly fast free(). Recycling works if either of
// the following is true:
// - T has a trivial destructor. In that case you don't have to ever call free() on
//   anything. You can just call freeAll() instead.
// - You call free() on all T's that you allocated, and never use freeAll().

template<typename T>
class Allocator {
public:
    Allocator();
    ~Allocator();
    
    void* allocate(); // Use placement new to allocate, and avoid using this method.
    void free(T*); // Call this method to delete; never use 'delete' directly.
    
    void freeAll(); // Only call this if you've either freed everything or if T has a trivial destructor.
    void reset(); // Like freeAll(), but also returns all memory to the OS.
    
    unsigned indexOf(const T*);
    
    static Allocator* allocatorOf(const T*);
    
private:
    void* bumpAllocate();
    void* freeListAllocate();
    void* allocateSlow();

    struct Region {
        static size_t size() { return 64 * KB; }
        static size_t headerSize() { return std::max(sizeof(Region), sizeof(T)); }
        static unsigned numberOfThingsPerRegion() { return (size() - headerSize()) / sizeof(T); }
        T* data() { return bitwise_cast<T*>(bitwise_cast<char*>(this) + headerSize()); }
        bool isInThisRegion(const T* pointer) { return static_cast<unsigned>(pointer - data()) < numberOfThingsPerRegion(); }
        static Region* regionFor(const T* pointer) { return bitwise_cast<Region*>(bitwise_cast<uintptr_t>(pointer) & ~(size() - 1)); }
        
        void* m_allocation;
        Allocator* m_allocator;
        Region* m_next;
    };
    
    void freeRegionsStartingAt(Region*);
    void startBumpingIn(Region*);
    
    Region* m_regionHead;
    void** m_freeListHead;
    T* m_bumpEnd;
    unsigned m_bumpRemaining;
};

template<typename T>
inline Allocator<T>::Allocator()
    : m_regionHead(0)
    , m_freeListHead(0)
    , m_bumpRemaining(0)
{
}

template<typename T>
inline Allocator<T>::~Allocator()
{
    reset();
}

template<typename T>
ALWAYS_INLINE void* Allocator<T>::allocate()
{
    void* result = bumpAllocate();
    if (LIKELY(!!result))
        return result;
    return freeListAllocate();
}

template<typename T>
void Allocator<T>::free(T* object)
{
    object->~T();
    
    void** cell = bitwise_cast<void**>(object);
    *cell = m_freeListHead;
    m_freeListHead = cell;
}

template<typename T>
void Allocator<T>::freeAll()
{
    if (!m_regionHead) {
        ASSERT(!m_bumpRemaining);
        ASSERT(!m_freeListHead);
        return;
    }
    
    // Since the caller is opting out of calling the destructor for any allocated thing,
    // we have two choices, plus a continuum between: we can either just delete all regions
    // (i.e. call reset()), or we can make all regions available for reuse. We do something
    // that optimizes for (a) speed of freeAll(), (b) the assumption that if the user calls
    // freeAll() then they will probably be calling allocate() in the near future. Namely,
    // we free all but one region, and make the remaining region a bump allocation region.
    
    freeRegionsStartingAt(m_regionHead->m_next);
    
    m_regionHead->m_next = 0;
    m_freeListHead = 0;
    startBumpingIn(m_regionHead);
}

template<typename T>
void Allocator<T>::reset()
{
    freeRegionsStartingAt(m_regionHead);
    
    m_regionHead = 0;
    m_freeListHead = 0;
    m_bumpRemaining = 0;
}

template<typename T>
unsigned Allocator<T>::indexOf(const T* object)
{
    unsigned numRegions = 0;
    for (Region* region = m_regionHead; region; region = region->m_next)
        numRegions++;
    unsigned regionIndex = 0;
    for (Region* region = m_regionHead; region; region = region->m_next) {
        if (region->isInThisRegion(object))
            return (numRegions - 1 - regionIndex) * Region::numberOfThingsPerRegion() + (object - region->data());
        regionIndex++;
    }
    CRASH();
    return 0;
}

template<typename T>
Allocator<T>* Allocator<T>::allocatorOf(const T* object)
{
    return Region::regionFor(object)->m_allocator;
}

template<typename T>
ALWAYS_INLINE void* Allocator<T>::bumpAllocate()
{
    if (unsigned remaining = m_bumpRemaining) {
        remaining--;
        m_bumpRemaining = remaining;
        return m_bumpEnd - (remaining + 1);
    }
    return 0;
}

template<typename T>
void* Allocator<T>::freeListAllocate()
{
    void** result = m_freeListHead;
    if (UNLIKELY(!result))
        return allocateSlow();
    m_freeListHead = bitwise_cast<void**>(*result);
    return result;
}

template<typename T>
void* Allocator<T>::allocateSlow()
{
    ASSERT(!m_freeListHead);
    ASSERT(!m_bumpRemaining);
    
    if (logCompilationChanges())
        dataLog("Allocating another allocator region.\n");

    void* allocation = fastAlignedMalloc(Region::size(), Region::size());
    Region* region = static_cast<Region*>(allocation);
    region->m_allocation = allocation;
    region->m_allocator = this;
    startBumpingIn(region);
    region->m_next = m_regionHead;
    m_regionHead = region;
    
    void* result = bumpAllocate();
    ASSERT(result);
    return result;
}

template<typename T>
void Allocator<T>::freeRegionsStartingAt(typename Allocator<T>::Region* region)
{
    while (region) {
        Region* nextRegion = region->m_next;
        fastAlignedFree(region->m_allocation);
        region = nextRegion;
    }
}

template<typename T>
void Allocator<T>::startBumpingIn(typename Allocator<T>::Region* region)
{
    m_bumpEnd = region->data() + Region::numberOfThingsPerRegion();
    m_bumpRemaining = Region::numberOfThingsPerRegion();
}

} } // namespace JSC::DFG

#endif // ENABLE(DFG_JIT)
