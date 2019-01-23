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
