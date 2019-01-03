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
#include "Cache.h"
#include "Heap.h"
#include "Inline.h"
#include "PerProcess.h"

namespace bmalloc {

void* Cache::operator new(size_t size)
{
    return vmAllocate(vmSize(size));
}

void Cache::operator delete(void* p, size_t size)
{
    vmDeallocate(p, vmSize(size));
}

void Cache::scavenge()
{
    Cache* cache = PerThread<Cache>::getFastCase();
    if (!cache)
        return;

    cache->allocator().scavenge();
    cache->deallocator().scavenge();
}

Cache::Cache()
    : m_deallocator(PerProcess<Heap>::get())
    , m_allocator(PerProcess<Heap>::get(), m_deallocator)
{
}

NO_INLINE void* Cache::tryAllocateSlowCaseNullCache(size_t size)
{
    return PerThread<Cache>::getSlowCase()->allocator().tryAllocate(size);
}

NO_INLINE void* Cache::allocateSlowCaseNullCache(size_t size)
{
    return PerThread<Cache>::getSlowCase()->allocator().allocate(size);
}

NO_INLINE void* Cache::allocateSlowCaseNullCache(size_t alignment, size_t size)
{
    return PerThread<Cache>::getSlowCase()->allocator().allocate(alignment, size);
}

NO_INLINE void Cache::deallocateSlowCaseNullCache(void* object)
{
    PerThread<Cache>::getSlowCase()->deallocator().deallocate(object);
}

NO_INLINE void* Cache::reallocateSlowCaseNullCache(void* object, size_t newSize)
{
    return PerThread<Cache>::getSlowCase()->allocator().reallocate(object, newSize);
}

} // namespace bmalloc
