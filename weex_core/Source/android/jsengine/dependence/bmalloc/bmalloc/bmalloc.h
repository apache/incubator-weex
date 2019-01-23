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
