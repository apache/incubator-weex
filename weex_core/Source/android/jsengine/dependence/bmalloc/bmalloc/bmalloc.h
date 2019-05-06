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
#include "AvailableMemory.h"
#include "Cache.h"
#include "Heap.h"
#include "PerProcess.h"
#include "StaticMutex.h"

namespace bmalloc {
namespace api {

// Returns null on failure.
inline void* tryMalloc(size_t size)
{
    return Cache::tryAllocate(size);
}

// Crashes on failure.
inline void* malloc(size_t size)
{
    return Cache::allocate(size);
}

// Returns null on failure.
inline void* tryMemalign(size_t alignment, size_t size)
{
    return Cache::tryAllocate(alignment, size);
}

// Crashes on failure.
inline void* memalign(size_t alignment, size_t size)
{
    return Cache::allocate(alignment, size);
}

// Crashes on failure.
inline void* realloc(void* object, size_t newSize)
{
    return Cache::reallocate(object, newSize);
}

inline void free(void* object)
{
    Cache::deallocate(object);
}

inline void scavengeThisThread()
{
    Cache::scavenge();
}

inline void scavenge()
{
    scavengeThisThread();

    std::unique_lock<StaticMutex> lock(PerProcess<Heap>::mutex());
    PerProcess<Heap>::get()->scavenge(lock);
}

inline bool isEnabled()
{
    std::unique_lock<StaticMutex> lock(PerProcess<Heap>::mutex());
    return !PerProcess<Heap>::getFastCase()->debugHeap();
}
    
inline size_t availableMemory()
{
    return bmalloc::availableMemory();
}
    
#if BPLATFORM(IOS)
inline size_t memoryFootprint()
{
    return PerProcess<Heap>::get()->memoryFootprint();
}

inline double percentAvailableMemoryInUse()
{
    return PerProcess<Heap>::get()->percentAvailableMemoryInUse();
}
#endif

#if BOS(DARWIN)
inline void setScavengerThreadQOSClass(qos_class_t overrideClass)
{
    std::unique_lock<StaticMutex> lock(PerProcess<Heap>::mutex());
    PerProcess<Heap>::getFastCase()->setScavengerThreadQOSClass(overrideClass);
}
#endif

} // namespace api
} // namespace bmalloc
