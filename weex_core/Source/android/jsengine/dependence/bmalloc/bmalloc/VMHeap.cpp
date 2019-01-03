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
