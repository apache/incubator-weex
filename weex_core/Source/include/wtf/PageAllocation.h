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
#ifndef PageAllocation_h
#define PageAllocation_h

#include <wtf/Assertions.h>
#include <wtf/OSAllocator.h>
#include <wtf/PageBlock.h>
#include <wtf/VMTags.h>
#include <algorithm>

#if OS(DARWIN)
#include <mach/mach_init.h>
#include <mach/vm_map.h>
#endif

#if OS(WINDOWS)
#include <malloc.h>
#include <windows.h>
#endif

#if HAVE(ERRNO_H)
#include <errno.h>
#endif

#if HAVE(MMAP)
#include <sys/mman.h>
#include <unistd.h>
#endif

namespace WTF {

/*
    PageAllocation

    The PageAllocation class provides a cross-platform memory allocation interface
    with similar capabilities to posix mmap/munmap.  Memory is allocated by calling
    PageAllocation::allocate, and deallocated by calling deallocate on the
    PageAllocation object.  The PageAllocation holds the allocation's base pointer
    and size.

    The allocate method is passed the size required (which must be a multiple of
    the system page size, which can be accessed using PageAllocation::pageSize).
    Callers may also optinally provide a flag indicating the usage (for use by
    system memory usage tracking tools, where implemented), and boolean values
    specifying the required protection (defaulting to writable, non-executable).
*/

class PageAllocation : private PageBlock {
public:
    PageAllocation()
    {
    }

    using PageBlock::size;
    using PageBlock::base;

#ifndef __clang__
    using PageBlock::operator bool;
#else
    // FIXME: This is a workaround for <rdar://problem/8876150>, wherein Clang incorrectly emits an access
    // control warning when a client tries to use operator bool exposed above via "using PageBlock::operator bool".
    operator bool() const { return PageBlock::operator bool(); }
#endif

    static PageAllocation allocate(size_t size, OSAllocator::Usage usage = OSAllocator::UnknownUsage, bool writable = true, bool executable = false)
    {
        ASSERT(isPageAligned(size));
        return PageAllocation(OSAllocator::reserveAndCommit(size, usage, writable, executable), size);
    }

    void deallocate()
    {
        // Clear base & size before calling release; if this is *inside* allocation
        // then we won't be able to clear then after deallocating the memory.
        PageAllocation tmp;
        std::swap(tmp, *this);

        ASSERT(tmp);
        ASSERT(!*this);

        OSAllocator::decommitAndRelease(tmp.base(), tmp.size());
    }

private:
    PageAllocation(void* base, size_t size)
        : PageBlock(base, size, false)
    {
    }
};

} // namespace WTF

using WTF::PageAllocation;

#endif // PageAllocation_h
