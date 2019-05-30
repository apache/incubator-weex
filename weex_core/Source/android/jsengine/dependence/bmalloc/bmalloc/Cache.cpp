/*
 * Copyright (C) 2014, 2015 Apple Inc. All rights reserved.
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
