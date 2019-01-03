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
#ifndef WTF_MetaAllocator_h
#define WTF_MetaAllocator_h

#include <wtf/Assertions.h>
#include <wtf/HashMap.h>
#include <wtf/Lock.h>
#include <wtf/MetaAllocatorHandle.h>
#include <wtf/Noncopyable.h>
#include <wtf/PageBlock.h>
#include <wtf/RedBlackTree.h>
#include <wtf/RefCounted.h>
#include <wtf/RefPtr.h>

namespace WTF {

#define ENABLE_META_ALLOCATOR_PROFILE 0

class MetaAllocatorTracker {
    WTF_MAKE_FAST_ALLOCATED;
public:
    void notify(MetaAllocatorHandle*);
    void release(MetaAllocatorHandle*);

    MetaAllocatorHandle* find(void* address)
    {
        MetaAllocatorHandle* handle = m_allocations.findGreatestLessThanOrEqual(address);
        if (handle && address < handle->end())
            return handle;
        return 0;
    }

    RedBlackTree<MetaAllocatorHandle, void*> m_allocations;
};

class MetaAllocator {
    WTF_MAKE_NONCOPYABLE(MetaAllocator);

public:
    WTF_EXPORT_PRIVATE MetaAllocator(size_t allocationGranule, size_t pageSize = WTF::pageSize());
    
    WTF_EXPORT_PRIVATE virtual ~MetaAllocator();
    
    WTF_EXPORT_PRIVATE RefPtr<MetaAllocatorHandle> allocate(size_t sizeInBytes, void* ownerUID);

    void trackAllocations(MetaAllocatorTracker* tracker)
    {
        m_tracker = tracker;
    }
    
    // Non-atomic methods for getting allocator statistics.
    size_t bytesAllocated() { return m_bytesAllocated; }
    size_t bytesReserved() { return m_bytesReserved; }
    size_t bytesCommitted() { return m_bytesCommitted; }
    
    // Atomic method for getting allocator statistics.
    struct Statistics {
        size_t bytesAllocated;
        size_t bytesReserved;
        size_t bytesCommitted;
    };
    WTF_EXPORT_PRIVATE Statistics currentStatistics();

    // Add more free space to the allocator. Call this directly from
    // the constructor if you wish to operate the allocator within a
    // fixed pool.
    WTF_EXPORT_PRIVATE void addFreshFreeSpace(void* start, size_t sizeInBytes);

    // This is meant only for implementing tests. Never call this in release
    // builds.
    WTF_EXPORT_PRIVATE size_t debugFreeSpaceSize();

    Lock& getLock() { return m_lock; }
    WTF_EXPORT_PRIVATE bool isInAllocatedMemory(const AbstractLocker&, void* address);
    
#if ENABLE(META_ALLOCATOR_PROFILE)
    void dumpProfile();
#else
    void dumpProfile() { }
#endif

protected:
    
    // Allocate new virtual space, but don't commit. This may return more
    // pages than we asked, in which case numPages is changed.
    virtual void* allocateNewSpace(size_t& numPages) = 0;
    
    // Commit a page.
    virtual void notifyNeedPage(void* page) = 0;
    
    // Uncommit a page.
    virtual void notifyPageIsFree(void* page) = 0;
    
    // NOTE: none of the above methods are called during allocator
    // destruction, in part because a MetaAllocator cannot die so long
    // as there are Handles that refer to it.

private:
    
    friend class MetaAllocatorHandle;
    
    class FreeSpaceNode : public RedBlackTree<FreeSpaceNode, size_t>::Node {
    public:
        FreeSpaceNode(void* start, size_t sizeInBytes)
            : m_start(start)
            , m_sizeInBytes(sizeInBytes)
        {
        }

        size_t key()
        {
            return m_sizeInBytes;
        }

        void* m_start;
        size_t m_sizeInBytes;
    };
    typedef RedBlackTree<FreeSpaceNode, size_t> Tree;

    // Release a MetaAllocatorHandle.
    void release(MetaAllocatorHandle*);
    
    // Remove free space from the allocator. This is effectively
    // the allocate() function, except that it does not mark the
    // returned space as being in-use.
    void* findAndRemoveFreeSpace(size_t sizeInBytes);

    // This is called when memory from an allocation is freed.
    void addFreeSpaceFromReleasedHandle(void* start, size_t sizeInBytes);
    
    // This is the low-level implementation of adding free space; it
    // is called from both addFreeSpaceFromReleasedHandle and from
    // addFreshFreeSpace.
    void addFreeSpace(void* start, size_t sizeInBytes);
    
    // Management of used space.
    
    void incrementPageOccupancy(void* address, size_t sizeInBytes);
    void decrementPageOccupancy(void* address, size_t sizeInBytes);

    // Utilities.
    
    size_t roundUp(size_t sizeInBytes);
    
    FreeSpaceNode* allocFreeSpaceNode();
    WTF_EXPORT_PRIVATE void freeFreeSpaceNode(FreeSpaceNode*);
    
    size_t m_allocationGranule;
    unsigned m_logAllocationGranule;
    size_t m_pageSize;
    unsigned m_logPageSize;
    
    Tree m_freeSpaceSizeMap;
    HashMap<void*, FreeSpaceNode*> m_freeSpaceStartAddressMap;
    HashMap<void*, FreeSpaceNode*> m_freeSpaceEndAddressMap;
    HashMap<uintptr_t, size_t> m_pageOccupancyMap;
    
    size_t m_bytesAllocated;
    size_t m_bytesReserved;
    size_t m_bytesCommitted;
    
    Lock m_lock;

    MetaAllocatorTracker* m_tracker;

#ifndef NDEBUG
    size_t m_mallocBalance;
#endif

#if ENABLE(META_ALLOCATOR_PROFILE)
    unsigned m_numAllocations;
    unsigned m_numFrees;
#endif
};

} // namespace WTF

#endif // WTF_MetaAllocator_h

