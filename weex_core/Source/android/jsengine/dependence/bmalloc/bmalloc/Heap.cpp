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
#include "Heap.h"

#if BPLATFORM(IOS)
#include "AvailableMemory.h"
#endif
#include "BumpAllocator.h"
#include "Chunk.h"
#include "DebugHeap.h"
#include "PerProcess.h"
#include "SmallLine.h"
#include "SmallPage.h"
#include <thread>

#if BOS(DARWIN)
#include "bmalloc.h"
#if BPLATFORM(IOS)
#import <mach/host_info.h>
#import <mach/mach.h>
#import <mach/mach_error.h>
#endif
#endif

namespace bmalloc {

Heap::Heap(std::lock_guard<StaticMutex>&)
    : m_vmPageSizePhysical(vmPageSizePhysical())
    , m_scavenger(*this, &Heap::concurrentScavenge)
    , m_debugHeap(nullptr)
#if BPLATFORM(IOS)
    , m_maxAvailableMemory(availableMemory())
#endif
{
    RELEASE_BASSERT(vmPageSizePhysical() >= smallPageSize);
    RELEASE_BASSERT(vmPageSize() >= vmPageSizePhysical());

    initializeLineMetadata();
    initializePageMetadata();
    
    if (m_environment.isDebugHeapEnabled())
        m_debugHeap = PerProcess<DebugHeap>::get();

#if BOS(DARWIN)
    auto queue = dispatch_queue_create("WebKit Malloc Memory Pressure Handler", DISPATCH_QUEUE_SERIAL);
    m_pressureHandlerDispatchSource = dispatch_source_create(DISPATCH_SOURCE_TYPE_MEMORYPRESSURE, 0, DISPATCH_MEMORYPRESSURE_CRITICAL, queue);
    dispatch_source_set_event_handler(m_pressureHandlerDispatchSource, ^{
        std::unique_lock<StaticMutex> lock(PerProcess<Heap>::mutex());
        scavenge(lock);
    });
    dispatch_resume(m_pressureHandlerDispatchSource);
    dispatch_release(queue);
#endif
}

void Heap::initializeLineMetadata()
{
    size_t sizeClassCount = bmalloc::sizeClass(smallLineSize);
    size_t smallLineCount = m_vmPageSizePhysical / smallLineSize;
    m_smallLineMetadata.grow(sizeClassCount * smallLineCount);

    for (size_t sizeClass = 0; sizeClass < sizeClassCount; ++sizeClass) {
        size_t size = objectSize(sizeClass);
        LineMetadata* pageMetadata = &m_smallLineMetadata[sizeClass * smallLineCount];

        size_t object = 0;
        size_t line = 0;
        while (object < m_vmPageSizePhysical) {
            line = object / smallLineSize;
            size_t leftover = object % smallLineSize;

            size_t objectCount;
            size_t remainder;
            divideRoundingUp(smallLineSize - leftover, size, objectCount, remainder);

            pageMetadata[line] = { static_cast<unsigned char>(leftover), static_cast<unsigned char>(objectCount) };

            object += objectCount * size;
        }

        // Don't allow the last object in a page to escape the page.
        if (object > m_vmPageSizePhysical) {
            BASSERT(pageMetadata[line].objectCount);
            --pageMetadata[line].objectCount;
        }
    }
}

void Heap::initializePageMetadata()
{
    auto computePageSize = [&](size_t sizeClass) {
        size_t size = objectSize(sizeClass);
        if (sizeClass < bmalloc::sizeClass(smallLineSize))
            return m_vmPageSizePhysical;

        for (size_t pageSize = m_vmPageSizePhysical;
            pageSize < pageSizeMax;
            pageSize += m_vmPageSizePhysical) {
            RELEASE_BASSERT(pageSize <= chunkSize / 2);
            size_t waste = pageSize % size;
            if (waste <= pageSize / pageSizeWasteFactor)
                return pageSize;
        }
        
        return pageSizeMax;
    };

    for (size_t i = 0; i < sizeClassCount; ++i)
        m_pageClasses[i] = (computePageSize(i) - 1) / smallPageSize;
}

#if BPLATFORM(IOS)
void Heap::updateMemoryInUseParameters()
{
    task_vm_info_data_t vmInfo;
    mach_msg_type_number_t vmSize = TASK_VM_INFO_COUNT;
    
    if (KERN_SUCCESS != task_info(mach_task_self(), TASK_VM_INFO, (task_info_t)(&vmInfo), &vmSize))
        m_memoryFootprint = 0;
    else
        m_memoryFootprint = static_cast<size_t>(vmInfo.phys_footprint);

    double percentInUse = static_cast<double>(m_memoryFootprint) / static_cast<double>(m_maxAvailableMemory);
    m_percentAvailableMemoryInUse = std::min(percentInUse, 1.0);
}
#endif

void Heap::concurrentScavenge()
{
    std::unique_lock<StaticMutex> lock(PerProcess<Heap>::mutex());

#if BOS(DARWIN)
    pthread_set_qos_class_self_np(m_requestedScavengerThreadQOSClass, 0);
#endif

    if (m_isGrowing && !isUnderMemoryPressure()) {
        m_isGrowing = false;
        m_scavenger.runSoon();
        return;
    }
    
    scavenge(lock);
}

void Heap::scavenge(std::unique_lock<StaticMutex>& lock)
{
    scavengeSmallPages(lock);
    scavengeLargeObjects(lock);
}
    
void Heap::scavengeSmallPages(std::unique_lock<StaticMutex>& lock)
{
    for (size_t pageClass = 0; pageClass < pageClassCount; pageClass++) {
        auto& smallPages = m_smallPages[pageClass];

        while (!smallPages.isEmpty()) {
            SmallPage* page = smallPages.pop();
            m_vmHeap.deallocateSmallPage(lock, pageClass, page);
        }
    }
}

void Heap::scavengeLargeObjects(std::unique_lock<StaticMutex>& lock)
{
    auto& ranges = m_largeFree.ranges();
    for (size_t i = ranges.size(); i-- > 0; i = std::min(i, ranges.size())) {
        auto range = ranges.pop(i);
        
        lock.unlock();
        vmDeallocatePhysicalPagesSloppy(range.begin(), range.size());
        lock.lock();

        range.setPhysicalSize(0);
        ranges.push(range);
    }
}

void Heap::scheduleScavengerIfUnderMemoryPressure(size_t bytes)
{
    m_scavengerBytes += bytes;
    if (m_scavengerBytes < scavengerBytesPerMemoryPressureCheck)
        return;

    m_scavengerBytes = 0;

    if (m_scavenger.willRun())
        return;

    if (!isUnderMemoryPressure())
        return;

    m_isGrowing = false;
    m_scavenger.run();
}

void Heap::scheduleScavenger(size_t bytes)
{
    scheduleScavengerIfUnderMemoryPressure(bytes);

    if (m_scavenger.willRunSoon())
        return;

    m_isGrowing = false;
    m_scavenger.runSoon();
}

SmallPage* Heap::allocateSmallPage(std::lock_guard<StaticMutex>& lock, size_t sizeClass)
{
    if (!m_smallPagesWithFreeLines[sizeClass].isEmpty())
        return m_smallPagesWithFreeLines[sizeClass].popFront();

    m_isGrowing = true;
    
    SmallPage* page = [&]() {
        size_t pageClass = m_pageClasses[sizeClass];
        if (!m_smallPages[pageClass].isEmpty())
            return m_smallPages[pageClass].pop();

        scheduleScavengerIfUnderMemoryPressure(pageSize(pageClass));
        
        SmallPage* page = m_vmHeap.allocateSmallPage(lock, pageClass);
        m_objectTypes.set(Chunk::get(page), ObjectType::Small);
        return page;
    }();

    page->setSizeClass(sizeClass);
    return page;
}

void Heap::deallocateSmallLine(std::lock_guard<StaticMutex>& lock, Object object)
{
    BASSERT(!object.line()->refCount(lock));
    SmallPage* page = object.page();
    page->deref(lock);

    if (!page->hasFreeLines(lock)) {
        page->setHasFreeLines(lock, true);
        m_smallPagesWithFreeLines[page->sizeClass()].push(page);
    }

    if (page->refCount(lock))
        return;

    size_t sizeClass = page->sizeClass();
    size_t pageClass = m_pageClasses[sizeClass];

    m_smallPagesWithFreeLines[sizeClass].remove(page);
    m_smallPages[pageClass].push(page);
    
    scheduleScavenger(pageSize(pageClass));
}

void Heap::allocateSmallBumpRangesByMetadata(
    std::lock_guard<StaticMutex>& lock, size_t sizeClass,
    BumpAllocator& allocator, BumpRangeCache& rangeCache)
{
    SmallPage* page = allocateSmallPage(lock, sizeClass);
    SmallLine* lines = page->begin();
    BASSERT(page->hasFreeLines(lock));
    size_t smallLineCount = m_vmPageSizePhysical / smallLineSize;
    LineMetadata* pageMetadata = &m_smallLineMetadata[sizeClass * smallLineCount];
    
    auto findSmallBumpRange = [&](size_t& lineNumber) {
        for ( ; lineNumber < smallLineCount; ++lineNumber) {
            if (!lines[lineNumber].refCount(lock)) {
                if (pageMetadata[lineNumber].objectCount)
                    return true;
            }
        }
        return false;
    };

    auto allocateSmallBumpRange = [&](size_t& lineNumber) -> BumpRange {
        char* begin = lines[lineNumber].begin() + pageMetadata[lineNumber].startOffset;
        unsigned short objectCount = 0;
        
        for ( ; lineNumber < smallLineCount; ++lineNumber) {
            if (lines[lineNumber].refCount(lock))
                break;

            if (!pageMetadata[lineNumber].objectCount)
                continue;

            objectCount += pageMetadata[lineNumber].objectCount;
            lines[lineNumber].ref(lock, pageMetadata[lineNumber].objectCount);
            page->ref(lock);
        }
        return { begin, objectCount };
    };

    size_t lineNumber = 0;
    for (;;) {
        if (!findSmallBumpRange(lineNumber)) {
            page->setHasFreeLines(lock, false);
            BASSERT(allocator.canAllocate());
            return;
        }

        // In a fragmented page, some free ranges might not fit in the cache.
        if (rangeCache.size() == rangeCache.capacity()) {
            m_smallPagesWithFreeLines[sizeClass].push(page);
            BASSERT(allocator.canAllocate());
            return;
        }

        BumpRange bumpRange = allocateSmallBumpRange(lineNumber);
        if (allocator.canAllocate())
            rangeCache.push(bumpRange);
        else
            allocator.refill(bumpRange);
    }
}

void Heap::allocateSmallBumpRangesByObject(
    std::lock_guard<StaticMutex>& lock, size_t sizeClass,
    BumpAllocator& allocator, BumpRangeCache& rangeCache)
{
    size_t size = allocator.size();
    SmallPage* page = allocateSmallPage(lock, sizeClass);
    BASSERT(page->hasFreeLines(lock));

    auto findSmallBumpRange = [&](Object& it, Object& end) {
        for ( ; it + size <= end; it = it + size) {
            if (!it.line()->refCount(lock))
                return true;
        }
        return false;
    };

    auto allocateSmallBumpRange = [&](Object& it, Object& end) -> BumpRange {
        char* begin = it.address();
        unsigned short objectCount = 0;
        for ( ; it + size <= end; it = it + size) {
            if (it.line()->refCount(lock))
                break;

            ++objectCount;
            it.line()->ref(lock);
            it.page()->ref(lock);
        }
        return { begin, objectCount };
    };

    Object it(page->begin()->begin());
    Object end(it + pageSize(m_pageClasses[sizeClass]));
    for (;;) {
        if (!findSmallBumpRange(it, end)) {
            page->setHasFreeLines(lock, false);
            BASSERT(allocator.canAllocate());
            return;
        }

        // In a fragmented page, some free ranges might not fit in the cache.
        if (rangeCache.size() == rangeCache.capacity()) {
            m_smallPagesWithFreeLines[sizeClass].push(page);
            BASSERT(allocator.canAllocate());
            return;
        }

        BumpRange bumpRange = allocateSmallBumpRange(it, end);
        if (allocator.canAllocate())
            rangeCache.push(bumpRange);
        else
            allocator.refill(bumpRange);
    }
}

LargeRange Heap::splitAndAllocate(LargeRange& range, size_t alignment, size_t size)
{
    LargeRange prev;
    LargeRange next;

    size_t alignmentMask = alignment - 1;
    if (test(range.begin(), alignmentMask)) {
        size_t prefixSize = roundUpToMultipleOf(alignment, range.begin()) - range.begin();
        std::pair<LargeRange, LargeRange> pair = range.split(prefixSize);
        prev = pair.first;
        range = pair.second;
    }

    if (range.size() - size > size / pageSizeWasteFactor) {
        std::pair<LargeRange, LargeRange> pair = range.split(size);
        range = pair.first;
        next = pair.second;
    }
    
    if (range.physicalSize() < range.size()) {
        scheduleScavengerIfUnderMemoryPressure(range.size());
        
        vmAllocatePhysicalPagesSloppy(range.begin() + range.physicalSize(), range.size() - range.physicalSize());
        range.setPhysicalSize(range.size());
    }
    
    if (prev)
        m_largeFree.add(prev);

    if (next)
        m_largeFree.add(next);

    m_objectTypes.set(Chunk::get(range.begin()), ObjectType::Large);

    m_largeAllocated.set(range.begin(), range.size());
    return range;
}

void* Heap::tryAllocateLarge(std::lock_guard<StaticMutex>& lock, size_t alignment, size_t size)
{
    BASSERT(isPowerOfTwo(alignment));

    m_isGrowing = true;
    
    size_t roundedSize = size ? roundUpToMultipleOf(largeAlignment, size) : largeAlignment;
    if (roundedSize < size) // Check for overflow
        return nullptr;
    size = roundedSize;

    size_t roundedAlignment = roundUpToMultipleOf<largeAlignment>(alignment);
    if (roundedAlignment < alignment) // Check for overflow
        return nullptr;
    alignment = roundedAlignment;

    LargeRange range = m_largeFree.remove(alignment, size);
    if (!range) {
        range = m_vmHeap.tryAllocateLargeChunk(lock, alignment, size);
        if (!range)
            return nullptr;

        m_largeFree.add(range);
        range = m_largeFree.remove(alignment, size);
    }

    return splitAndAllocate(range, alignment, size).begin();
}

void* Heap::allocateLarge(std::lock_guard<StaticMutex>& lock, size_t alignment, size_t size)
{
    void* result = tryAllocateLarge(lock, alignment, size);
    RELEASE_BASSERT(result);
    return result;
}

bool Heap::isLarge(std::lock_guard<StaticMutex>&, void* object)
{
    return m_objectTypes.get(Object(object).chunk()) == ObjectType::Large;
}

size_t Heap::largeSize(std::lock_guard<StaticMutex>&, void* object)
{
    return m_largeAllocated.get(object);
}

void Heap::shrinkLarge(std::lock_guard<StaticMutex>&, const Range& object, size_t newSize)
{
    BASSERT(object.size() > newSize);

    size_t size = m_largeAllocated.remove(object.begin());
    LargeRange range = LargeRange(object, size);
    splitAndAllocate(range, alignment, newSize);

    scheduleScavenger(size);
}

void Heap::deallocateLarge(std::lock_guard<StaticMutex>&, void* object)
{
    size_t size = m_largeAllocated.remove(object);
    m_largeFree.add(LargeRange(object, size, size));
    
    scheduleScavenger(size);
}

} // namespace bmalloc
