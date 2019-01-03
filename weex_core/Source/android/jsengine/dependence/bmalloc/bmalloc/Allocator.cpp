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
#include "Allocator.h"
#include "BAssert.h"
#include "Chunk.h"
#include "Deallocator.h"
#include "DebugHeap.h"
#include "Heap.h"
#include "PerProcess.h"
#include "Sizes.h"
#include <algorithm>
#include <cstdlib>

using namespace std;

namespace bmalloc {

Allocator::Allocator(Heap* heap, Deallocator& deallocator)
    : m_debugHeap(heap->debugHeap())
    , m_deallocator(deallocator)
{
    for (size_t sizeClass = 0; sizeClass < sizeClassCount; ++sizeClass)
        m_bumpAllocators[sizeClass].init(objectSize(sizeClass));
}

Allocator::~Allocator()
{
    scavenge();
}

void* Allocator::tryAllocate(size_t size)
{
    if (m_debugHeap)
        return m_debugHeap->malloc(size);

    if (size <= smallMax)
        return allocate(size);

    std::lock_guard<StaticMutex> lock(PerProcess<Heap>::mutex());
    return PerProcess<Heap>::getFastCase()->tryAllocateLarge(lock, alignment, size);
}

void* Allocator::allocate(size_t alignment, size_t size)
{
    bool crashOnFailure = true;
    return allocateImpl(alignment, size, crashOnFailure);
}

void* Allocator::tryAllocate(size_t alignment, size_t size)
{
    bool crashOnFailure = false;
    return allocateImpl(alignment, size, crashOnFailure);
}

void* Allocator::allocateImpl(size_t alignment, size_t size, bool crashOnFailure)
{
    BASSERT(isPowerOfTwo(alignment));

    if (m_debugHeap)
        return m_debugHeap->memalign(alignment, size, crashOnFailure);

    if (!size)
        size = alignment;

    if (size <= smallMax && alignment <= smallMax)
        return allocate(roundUpToMultipleOf(alignment, size));

    std::lock_guard<StaticMutex> lock(PerProcess<Heap>::mutex());
    Heap* heap = PerProcess<Heap>::getFastCase();
    if (crashOnFailure)
        return heap->allocateLarge(lock, alignment, size);
    return heap->tryAllocateLarge(lock, alignment, size);
}

void* Allocator::reallocate(void* object, size_t newSize)
{
    if (m_debugHeap)
        return m_debugHeap->realloc(object, newSize);

    size_t oldSize = 0;
    switch (objectType(object)) {
    case ObjectType::Small: {
        BASSERT(objectType(nullptr) == ObjectType::Small);
        if (!object)
            break;

        size_t sizeClass = Object(object).page()->sizeClass();
        oldSize = objectSize(sizeClass);
        break;
    }
    case ObjectType::Large: {
        std::lock_guard<StaticMutex> lock(PerProcess<Heap>::mutex());
        oldSize = PerProcess<Heap>::getFastCase()->largeSize(lock, object);

        if (newSize < oldSize && newSize > smallMax) {
            PerProcess<Heap>::getFastCase()->shrinkLarge(lock, Range(object, oldSize), newSize);
            return object;
        }
        break;
    }
    }

    void* result = allocate(newSize);
    size_t copySize = std::min(oldSize, newSize);
    memcpy(result, object, copySize);
    m_deallocator.deallocate(object);
    return result;
}

void Allocator::scavenge()
{
    for (size_t sizeClass = 0; sizeClass < sizeClassCount; ++sizeClass) {
        BumpAllocator& allocator = m_bumpAllocators[sizeClass];
        BumpRangeCache& bumpRangeCache = m_bumpRangeCaches[sizeClass];

        while (allocator.canAllocate())
            m_deallocator.deallocate(allocator.allocate());

        while (bumpRangeCache.size()) {
            allocator.refill(bumpRangeCache.pop());
            while (allocator.canAllocate())
                m_deallocator.deallocate(allocator.allocate());
        }

        allocator.clear();
    }
}

NO_INLINE void Allocator::refillAllocatorSlowCase(BumpAllocator& allocator, size_t sizeClass)
{
    BumpRangeCache& bumpRangeCache = m_bumpRangeCaches[sizeClass];

    std::lock_guard<StaticMutex> lock(PerProcess<Heap>::mutex());
    m_deallocator.processObjectLog(lock);
    PerProcess<Heap>::getFastCase()->allocateSmallBumpRanges(lock, sizeClass, allocator, bumpRangeCache);
}

INLINE void Allocator::refillAllocator(BumpAllocator& allocator, size_t sizeClass)
{
    BumpRangeCache& bumpRangeCache = m_bumpRangeCaches[sizeClass];
    if (!bumpRangeCache.size())
        return refillAllocatorSlowCase(allocator, sizeClass);
    return allocator.refill(bumpRangeCache.pop());
}

NO_INLINE void* Allocator::allocateLarge(size_t size)
{
    std::lock_guard<StaticMutex> lock(PerProcess<Heap>::mutex());
    return PerProcess<Heap>::getFastCase()->allocateLarge(lock, alignment, size);
}

NO_INLINE void* Allocator::allocateLogSizeClass(size_t size)
{
    size_t sizeClass = bmalloc::sizeClass(size);
    BumpAllocator& allocator = m_bumpAllocators[sizeClass];
    if (!allocator.canAllocate())
        refillAllocator(allocator, sizeClass);
    return allocator.allocate();
}

void* Allocator::allocateSlowCase(size_t size)
{
    if (m_debugHeap)
        return m_debugHeap->malloc(size);

    if (size <= maskSizeClassMax) {
        size_t sizeClass = bmalloc::maskSizeClass(size);
        BumpAllocator& allocator = m_bumpAllocators[sizeClass];
        refillAllocator(allocator, sizeClass);
        return allocator.allocate();
    }

    if (size <= smallMax)
        return allocateLogSizeClass(size);

    return allocateLarge(size);
}

} // namespace bmalloc
