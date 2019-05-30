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
