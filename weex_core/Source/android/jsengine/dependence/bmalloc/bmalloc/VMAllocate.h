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
#ifndef VMAllocate_h
#define VMAllocate_h

#include "BAssert.h"
#include "Logging.h"
#include "Range.h"
#include "Sizes.h"
#include "Syscall.h"
#include <algorithm>
#include <sys/mman.h>
#include <unistd.h>

#if BOS(DARWIN)
#include <mach/vm_page_size.h>
#include <mach/vm_statistics.h>
#endif

namespace bmalloc {

#if BOS(DARWIN)
#define BMALLOC_VM_TAG VM_MAKE_TAG(VM_MEMORY_TCMALLOC)
#else
#define BMALLOC_VM_TAG -1
#endif

inline size_t vmPageSize()
{
    static size_t cached;
    if (!cached)
        cached = sysconf(_SC_PAGESIZE);
    return cached;
}

inline size_t vmPageShift()
{
    static size_t cached;
    if (!cached)
        cached = log2(vmPageSize());
    return cached;
}

inline size_t vmSize(size_t size)
{
    return roundUpToMultipleOf(vmPageSize(), size);
}

inline void vmValidate(size_t vmSize)
{
    UNUSED(vmSize);
    BASSERT(vmSize);
    BASSERT(vmSize == roundUpToMultipleOf(vmPageSize(), vmSize));
}

inline void vmValidate(void* p, size_t vmSize)
{
    vmValidate(vmSize);
    
    UNUSED(p);
    BASSERT(p);
    BASSERT(p == mask(p, ~(vmPageSize() - 1)));
}

inline size_t vmPageSizePhysical()
{
#if (BPLATFORM(IOS) && __IPHONE_OS_VERSION_MIN_REQUIRED >= 100000)
    return vm_kernel_page_size;
#else
    static size_t cached;
    if (!cached)
        cached = sysconf(_SC_PAGESIZE);
    return cached;
#endif
}

inline void vmValidatePhysical(size_t vmSize)
{
    UNUSED(vmSize);
    BASSERT(vmSize);
    BASSERT(vmSize == roundUpToMultipleOf(vmPageSizePhysical(), vmSize));
}

inline void vmValidatePhysical(void* p, size_t vmSize)
{
    vmValidatePhysical(vmSize);
    
    UNUSED(p);
    BASSERT(p);
    BASSERT(p == mask(p, ~(vmPageSizePhysical() - 1)));
}

inline void* tryVMAllocate(size_t vmSize)
{
    vmValidate(vmSize);
    void* result = mmap(0, vmSize, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON | MAP_POPULATE, BMALLOC_VM_TAG, 0);
    if (result == MAP_FAILED) {
        logVMFailure();
        return nullptr;
    }
    return result;
}

inline void* vmAllocate(size_t vmSize)
{
    void* result = tryVMAllocate(vmSize);
    RELEASE_BASSERT(result);
    return result;
}

inline void vmDeallocate(void* p, size_t vmSize)
{
    vmValidate(p, vmSize);
    munmap(p, vmSize);
}

inline void vmRevokePermissions(void* p, size_t vmSize)
{
    vmValidate(p, vmSize);
    mprotect(p, vmSize, PROT_NONE);
}

// Allocates vmSize bytes at a specified power-of-two alignment.
// Use this function to create maskable memory regions.

inline void* tryVMAllocate(size_t vmAlignment, size_t vmSize)
{
    vmValidate(vmSize);
    vmValidate(vmAlignment);

    size_t mappedSize = vmAlignment + vmSize;
    if (mappedSize < vmAlignment || mappedSize < vmSize) // Check for overflow
        return nullptr;

    char* mapped = static_cast<char*>(tryVMAllocate(mappedSize));
    if (!mapped)
        return nullptr;
    char* mappedEnd = mapped + mappedSize;

    char* aligned = roundUpToMultipleOf(vmAlignment, mapped);
    char* alignedEnd = aligned + vmSize;
    
    RELEASE_BASSERT(alignedEnd <= mappedEnd);
    
    if (size_t leftExtra = aligned - mapped)
        vmDeallocate(mapped, leftExtra);
    
    if (size_t rightExtra = mappedEnd - alignedEnd)
        vmDeallocate(alignedEnd, rightExtra);

    return aligned;
}

inline void* vmAllocate(size_t vmAlignment, size_t vmSize)
{
    void* result = tryVMAllocate(vmAlignment, vmSize);
    RELEASE_BASSERT(result);
    return result;
}

inline void vmDeallocatePhysicalPages(void* p, size_t vmSize)
{
    vmValidatePhysical(p, vmSize);
#if BOS(DARWIN)
    SYSCALL(madvise(p, vmSize, MADV_FREE_REUSABLE));
#else
    SYSCALL(madvise(p, vmSize, MADV_DONTNEED));
#endif
}

inline void vmAllocatePhysicalPages(void* p, size_t vmSize)
{
    vmValidatePhysical(p, vmSize);
#if BOS(DARWIN)
    SYSCALL(madvise(p, vmSize, MADV_FREE_REUSE));
#else
    SYSCALL(madvise(p, vmSize, MADV_NORMAL));
#endif
}

// Trims requests that are un-page-aligned.
inline void vmDeallocatePhysicalPagesSloppy(void* p, size_t size)
{
    char* begin = roundUpToMultipleOf(vmPageSizePhysical(), static_cast<char*>(p));
    char* end = roundDownToMultipleOf(vmPageSizePhysical(), static_cast<char*>(p) + size);

    if (begin >= end)
        return;

    vmDeallocatePhysicalPages(begin, end - begin);
}

// Expands requests that are un-page-aligned.
inline void vmAllocatePhysicalPagesSloppy(void* p, size_t size)
{
    char* begin = roundDownToMultipleOf(vmPageSizePhysical(), static_cast<char*>(p));
    char* end = roundUpToMultipleOf(vmPageSizePhysical(), static_cast<char*>(p) + size);

    if (begin >= end)
        return;

    vmAllocatePhysicalPages(begin, end - begin);
}

} // namespace bmalloc

#endif // VMAllocate_h
