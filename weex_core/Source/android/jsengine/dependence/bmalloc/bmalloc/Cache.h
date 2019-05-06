/*
 * Copyright (C) 2014, 2016 Apple Inc. All rights reserved.
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
#ifndef Cache_h
#define Cache_h

#include "Allocator.h"
#include "Deallocator.h"
#include "PerThread.h"

namespace bmalloc {

// Per-thread allocation / deallocation cache, backed by a per-process Heap.

class Cache {
public:
    void* operator new(size_t);
    void operator delete(void*, size_t);

    static void* tryAllocate(size_t);
    static void* allocate(size_t);
    static void* tryAllocate(size_t alignment, size_t);
    static void* allocate(size_t alignment, size_t);
    static void deallocate(void*);
    static void* reallocate(void*, size_t);

    static void scavenge();

    Cache();

    Allocator& allocator() { return m_allocator; }
    Deallocator& deallocator() { return m_deallocator; }

private:
    static void* tryAllocateSlowCaseNullCache(size_t);
    static void* allocateSlowCaseNullCache(size_t);
    static void* allocateSlowCaseNullCache(size_t alignment, size_t);
    static void deallocateSlowCaseNullCache(void*);
    static void* reallocateSlowCaseNullCache(void*, size_t);

    Deallocator m_deallocator;
    Allocator m_allocator;
};

inline void* Cache::tryAllocate(size_t size)
{
    Cache* cache = PerThread<Cache>::getFastCase();
    if (!cache)
        return tryAllocateSlowCaseNullCache(size);
    return cache->allocator().tryAllocate(size);
}

inline void* Cache::allocate(size_t size)
{
    Cache* cache = PerThread<Cache>::getFastCase();
    if (!cache)
        return allocateSlowCaseNullCache(size);
    return cache->allocator().allocate(size);
}

inline void* Cache::tryAllocate(size_t alignment, size_t size)
{
    Cache* cache = PerThread<Cache>::getFastCase();
    if (!cache)
        return allocateSlowCaseNullCache(alignment, size);
    return cache->allocator().tryAllocate(alignment, size);
}

inline void* Cache::allocate(size_t alignment, size_t size)
{
    Cache* cache = PerThread<Cache>::getFastCase();
    if (!cache)
        return allocateSlowCaseNullCache(alignment, size);
    return cache->allocator().allocate(alignment, size);
}

inline void Cache::deallocate(void* object)
{
    Cache* cache = PerThread<Cache>::getFastCase();
    if (!cache)
        return deallocateSlowCaseNullCache(object);
    return cache->deallocator().deallocate(object);
}

inline void* Cache::reallocate(void* object, size_t newSize)
{
    Cache* cache = PerThread<Cache>::getFastCase();
    if (!cache)
        return reallocateSlowCaseNullCache(object, newSize);
    return cache->allocator().reallocate(object, newSize);
}

} // namespace bmalloc

#endif // Cache_h
