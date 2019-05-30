/*
 * Copyright (C) 2014, 2015 Apple Inc. All rights reserved.
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
#include "PerProcess.h"
#include "VMHeap.h"
#include <thread>

namespace bmalloc {

LargeRange VMHeap::tryAllocateLargeChunk(std::lock_guard<StaticMutex>&, size_t alignment, size_t size)
{
    // We allocate VM in aligned multiples to increase the chances that
    // the OS will provide contiguous ranges that we can merge.
    size_t roundedAlignment = roundUpToMultipleOf<chunkSize>(alignment);
    if (roundedAlignment < alignment) // Check for overflow
        return LargeRange();
    alignment = roundedAlignment;

    size_t roundedSize = roundUpToMultipleOf<chunkSize>(size);
    if (roundedSize < size) // Check for overflow
        return LargeRange();
    size = roundedSize;

    void* memory = tryVMAllocate(alignment, size);
    if (!memory)
        return LargeRange();

    Chunk* chunk = static_cast<Chunk*>(memory);
    
#if BOS(DARWIN)
    m_zone.addRange(Range(chunk->bytes(), size));
#endif

    return LargeRange(chunk->bytes(), size, 0);
}

void VMHeap::allocateSmallChunk(std::lock_guard<StaticMutex>& lock, size_t pageClass)
{
    size_t pageSize = bmalloc::pageSize(pageClass);
    size_t smallPageCount = pageSize / smallPageSize;

    void* memory = vmAllocate(chunkSize, chunkSize);
    Chunk* chunk = static_cast<Chunk*>(memory);

    // We align to our page size in order to honor OS APIs and in order to
    // guarantee that we can service aligned allocation requests at equal
    // and smaller powers of two.
    size_t vmPageSize = roundUpToMultipleOf(bmalloc::vmPageSize(), pageSize);
    size_t metadataSize = roundUpToMultipleOfNonPowerOfTwo(vmPageSize, sizeof(Chunk));

    Object begin(chunk, metadataSize);
    Object end(chunk, chunkSize);

    // Establish guard pages before writing to Chunk memory to work around
    // an edge case in the Darwin VM system (<rdar://problem/25910098>).
    vmRevokePermissions(begin.address(), vmPageSize);
    vmRevokePermissions(end.address() - vmPageSize, vmPageSize);
    
    begin = begin + vmPageSize;
    end = end - vmPageSize;
    BASSERT(begin <= end && end.offset() - begin.offset() >= pageSize);

    new (chunk) Chunk(lock);

#if BOS(DARWIN)
    m_zone.addRange(Range(begin.address(), end.address() - begin.address()));
#endif

    for (Object it = begin; it + pageSize <= end; it = it + pageSize) {
        SmallPage* page = it.page();

        for (size_t i = 0; i < smallPageCount; ++i)
            page[i].setSlide(i);

        m_smallPages[pageClass].push(page);
    }
}

} // namespace bmalloc
