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
#ifndef Heap_h
#define Heap_h

#include "AsyncTask.h"
#include "BumpRange.h"
#include "Environment.h"
#include "LargeMap.h"
#include "LineMetadata.h"
#include "List.h"
#include "Map.h"
#include "Mutex.h"
#include "Object.h"
#include "SmallLine.h"
#include "SmallPage.h"
#include "VMHeap.h"
#include "Vector.h"
#include <array>
#include <mutex>

#if BOS(DARWIN)
#include <dispatch/dispatch.h>
#endif

namespace bmalloc {

class BeginTag;
class BumpAllocator;
class DebugHeap;
class EndTag;

class Heap {
public:
    Heap(std::lock_guard<StaticMutex>&);
    
    DebugHeap* debugHeap() { return m_debugHeap; }

    void allocateSmallBumpRanges(std::lock_guard<StaticMutex>&, size_t sizeClass, BumpAllocator&, BumpRangeCache&);
    void derefSmallLine(std::lock_guard<StaticMutex>&, Object);

    void* allocateLarge(std::lock_guard<StaticMutex>&, size_t alignment, size_t);
    void* tryAllocateLarge(std::lock_guard<StaticMutex>&, size_t alignment, size_t);
    void deallocateLarge(std::lock_guard<StaticMutex>&, void*);

    bool isLarge(std::lock_guard<StaticMutex>&, void*);
    size_t largeSize(std::lock_guard<StaticMutex>&, void*);
    void shrinkLarge(std::lock_guard<StaticMutex>&, const Range&, size_t);

    void scavenge(std::unique_lock<StaticMutex>&);

    size_t memoryFootprint();
    double percentAvailableMemoryInUse();
    bool isUnderMemoryPressure();
    
#if BOS(DARWIN)
    void setScavengerThreadQOSClass(qos_class_t overrideClass) { m_requestedScavengerThreadQOSClass = overrideClass; }
#endif

private:
    struct LargeObjectHash {
        static unsigned hash(void* key)
        {
            return static_cast<unsigned>(
                reinterpret_cast<uintptr_t>(key) / smallMax);
        }
    };

    ~Heap() = delete;
    
    void initializeLineMetadata();
    void initializePageMetadata();

    void allocateSmallBumpRangesByMetadata(std::lock_guard<StaticMutex>&,
        size_t sizeClass, BumpAllocator&, BumpRangeCache&);
    void allocateSmallBumpRangesByObject(std::lock_guard<StaticMutex>&,
        size_t sizeClass, BumpAllocator&, BumpRangeCache&);

    SmallPage* allocateSmallPage(std::lock_guard<StaticMutex>&, size_t sizeClass);

    void deallocateSmallLine(std::lock_guard<StaticMutex>&, Object);

    void mergeLarge(BeginTag*&, EndTag*&, Range&);
    void mergeLargeLeft(EndTag*&, BeginTag*&, Range&, bool& inVMHeap);
    void mergeLargeRight(EndTag*&, BeginTag*&, Range&, bool& inVMHeap);

    LargeRange splitAndAllocate(LargeRange&, size_t alignment, size_t);

    void scheduleScavenger(size_t);
    void scheduleScavengerIfUnderMemoryPressure(size_t);
    
    void concurrentScavenge();
    void scavengeSmallPages(std::unique_lock<StaticMutex>&);
    void scavengeLargeObjects(std::unique_lock<StaticMutex>&);
    
#if BPLATFORM(IOS)
    void updateMemoryInUseParameters();
#endif

    size_t m_vmPageSizePhysical;
    Vector<LineMetadata> m_smallLineMetadata;
    std::array<size_t, sizeClassCount> m_pageClasses;

    std::array<List<SmallPage>, sizeClassCount> m_smallPagesWithFreeLines;
    std::array<List<SmallPage>, pageClassCount> m_smallPages;

    Map<void*, size_t, LargeObjectHash> m_largeAllocated;
    LargeMap m_largeFree;

    Map<Chunk*, ObjectType, ChunkHash> m_objectTypes;

    size_t m_scavengerBytes { 0 };
    bool m_isGrowing { false };
    
    AsyncTask<Heap, decltype(&Heap::concurrentScavenge)> m_scavenger;

    Environment m_environment;
    DebugHeap* m_debugHeap;

#if BPLATFORM(IOS)
    size_t m_maxAvailableMemory;
    size_t m_memoryFootprint;
    double m_percentAvailableMemoryInUse;
#endif

    VMHeap m_vmHeap;

#if BOS(DARWIN)
    dispatch_source_t m_pressureHandlerDispatchSource;
    qos_class_t m_requestedScavengerThreadQOSClass { QOS_CLASS_USER_INITIATED };
#endif
};

inline void Heap::allocateSmallBumpRanges(
    std::lock_guard<StaticMutex>& lock, size_t sizeClass,
    BumpAllocator& allocator, BumpRangeCache& rangeCache)
{
    if (sizeClass < bmalloc::sizeClass(smallLineSize))
        return allocateSmallBumpRangesByMetadata(lock, sizeClass, allocator, rangeCache);
    return allocateSmallBumpRangesByObject(lock, sizeClass, allocator, rangeCache);
}

inline void Heap::derefSmallLine(std::lock_guard<StaticMutex>& lock, Object object)
{
    if (!object.line()->deref(lock))
        return;
    deallocateSmallLine(lock, object);
}

inline bool Heap::isUnderMemoryPressure()
{
#if BPLATFORM(IOS)
    return percentAvailableMemoryInUse() > memoryPressureThreshold;
#else
    return false;
#endif
}
    
#if BPLATFORM(IOS)
inline size_t Heap::memoryFootprint()
{
    updateMemoryInUseParameters();

    return m_memoryFootprint;
}

inline double Heap::percentAvailableMemoryInUse()
{
    updateMemoryInUseParameters();

    return m_percentAvailableMemoryInUse;
}
#endif

} // namespace bmalloc

#endif // Heap_h
