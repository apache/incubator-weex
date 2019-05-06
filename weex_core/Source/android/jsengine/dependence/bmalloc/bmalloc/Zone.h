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
#ifndef Zone_h
#define Zone_h

#include "FixedVector.h"
#include "Range.h"
#include <malloc/malloc.h>

namespace bmalloc {

class Chunk;

class Zone : public malloc_zone_t {
public:
    // Enough capacity to track a 64GB heap, so probably enough for anything.
    static const size_t capacity = 2048;

    Zone();
    Zone(task_t, memory_reader_t, vm_address_t);

    void addRange(Range);
    FixedVector<Range, capacity>& ranges() { return m_ranges; }
    
private:
    // This vector has two purposes:
    //     (1) It stores the list of Chunks so that we can enumerate
    //         each Chunk and request that it be scanned if reachable.
    //     (2) It roots a pointer to each Chunk in a global non-malloc
    //         VM region, making each Chunk appear reachable, and therefore
    //         ensuring that the leaks tool will scan it. (The leaks tool
    //         conservatively scans all writeable VM regions that are not malloc
    //         regions, and then scans malloc regions using the introspection API.)
    // This prevents the leaks tool from reporting false positive leaks for
    // objects pointed to from bmalloc memory -- though it also prevents the
    // leaks tool from finding any leaks in bmalloc memory.
    FixedVector<Range, capacity> m_ranges;
};

inline void Zone::addRange(Range range)
{
    if (m_ranges.size() == m_ranges.capacity())
        return;
    
    m_ranges.push(range);
}

} // namespace bmalloc

#endif // Zone_h
