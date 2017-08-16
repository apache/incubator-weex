/*
 * Copyright (C) 2014-2017 Apple Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "EdenGCActivityCallback.h"
#include "HeapInlines.h"

#include "VM.h"

namespace JSC {

#if USE(CF) || USE(GLIB)

EdenGCActivityCallback::EdenGCActivityCallback(Heap* heap)
    : GCActivityCallback(heap)
{
}

void EdenGCActivityCallback::doCollection()
{
    m_vm->heap.collectAsync(CollectionScope::Eden);
}

double EdenGCActivityCallback::lastGCLength()
{
    return m_vm->heap.lastEdenGCLength().seconds();
}

double EdenGCActivityCallback::deathRate()
{
    Heap* heap = &m_vm->heap;
    size_t sizeBefore = heap->sizeBeforeLastEdenCollection();
    size_t sizeAfter = heap->sizeAfterLastEdenCollection();
    if (!sizeBefore)
        return 1.0;
    if (sizeAfter > sizeBefore) {
        // GC caused the heap to grow(!)
        // This could happen if the we visited more extra memory than was reported allocated.
        // We don't return a negative death rate, since that would schedule the next GC in the past.
        return 0;
    }
    return static_cast<double>(sizeBefore - sizeAfter) / static_cast<double>(sizeBefore);
}

double EdenGCActivityCallback::gcTimeSlice(size_t bytes)
{
    return std::min((static_cast<double>(bytes) / MB) * Options::percentCPUPerMBForEdenTimer(), Options::collectionTimerMaxPercentCPU());
}

#else

EdenGCActivityCallback::EdenGCActivityCallback(Heap* heap)
    : GCActivityCallback(heap->vm())
{
}

void EdenGCActivityCallback::doCollection()
{
}

double EdenGCActivityCallback::lastGCLength()
{
    return 0;
}

double EdenGCActivityCallback::deathRate()
{
    return 0;
}

double EdenGCActivityCallback::gcTimeSlice(size_t)
{
    return 0;
}

#endif // USE(CF) || USE(GLIB)

} // namespace JSC
