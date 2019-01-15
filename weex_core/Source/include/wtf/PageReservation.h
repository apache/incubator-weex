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
#ifndef PageReservation_h
#define PageReservation_h

#include <wtf/PageAllocation.h>

namespace WTF {

/*
    PageReservation

    Like PageAllocation, the PageReservation class provides a cross-platform memory
    allocation interface, but with a set of capabilities more similar to that of
    VirtualAlloc than posix mmap.  PageReservation can be used to allocate virtual
    memory without committing physical memory pages using PageReservation::reserve.
    Following a call to reserve all memory in the region is in a decommited state,
    in which the memory should not be used (accessing the memory may cause a fault).

    Before using memory it must be committed by calling commit, which is passed start
    and size values (both of which require system page size granularity).  One the
    committed memory is no longer needed 'decommit' may be called to return the
    memory to its devommitted state.  Commit should only be called on memory that is
    currently decommitted, and decommit should only be called on memory regions that
    are currently committed.  All memory should be decommited before the reservation
    is deallocated.  Values in memory may not be retained accross a pair of calls if
    the region of memory is decommitted and then committed again.

    Memory protection should not be changed on decommitted memory, and if protection
    is changed on memory while it is committed it should be returned to the orignal
    protection before decommit is called.
*/

class PageReservation : private PageBlock {
public:
    PageReservation()
        : m_committed(0)
        , m_writable(false)
        , m_executable(false)
    {
    }

    using PageBlock::base;
    using PageBlock::size;
    using PageBlock::operator bool;

    void commit(void* start, size_t size)
    {
        ASSERT(*this);
        ASSERT(isPageAligned(start));
        ASSERT(isPageAligned(size));
        ASSERT(contains(start, size));

        m_committed += size;
        OSAllocator::commit(start, size, m_writable, m_executable);
    }

    void decommit(void* start, size_t size)
    {
        ASSERT(*this);
        ASSERT(isPageAligned(start));
        ASSERT(isPageAligned(size));
        ASSERT(contains(start, size));

        m_committed -= size;
        OSAllocator::decommit(start, size);
    }

    size_t committed()
    {
        return m_committed;
    }

    static PageReservation reserve(size_t size, OSAllocator::Usage usage = OSAllocator::UnknownUsage, bool writable = true, bool executable = false)
    {
        ASSERT(isPageAligned(size));
        return PageReservation(OSAllocator::reserveUncommitted(size, usage, writable, executable), size, writable, executable, false);
    }
    
    static PageReservation reserveWithGuardPages(size_t size, OSAllocator::Usage usage = OSAllocator::UnknownUsage, bool writable = true, bool executable = false)
    {
        ASSERT(isPageAligned(size));
        return PageReservation(OSAllocator::reserveUncommitted(size + pageSize() * 2, usage, writable, executable, true), size, writable, executable, true);
    }

    void deallocate()
    {
        ASSERT(!m_committed);

        // Clear base & size before calling release; if this is *inside* allocation
        // then we won't be able to clear then after deallocating the memory.
        PageReservation tmp;
        std::swap(tmp, *this);

        ASSERT(tmp);
        ASSERT(!*this);

        OSAllocator::releaseDecommitted(tmp.base(), tmp.size());
    }

private:
    PageReservation(void* base, size_t size, bool writable, bool executable, bool hasGuardPages)
        : PageBlock(base, size, hasGuardPages)
        , m_committed(0)
        , m_writable(writable)
        , m_executable(executable)
    {
    }

    size_t m_committed;
    bool m_writable;
    bool m_executable;
};

}

using WTF::PageReservation;

#endif // PageReservation_h
