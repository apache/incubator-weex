/*
 * Copyright (C) 2016 Apple Inc. All rights reserved.
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
#include "LargeMap.h"
#include <utility>

namespace bmalloc {

LargeRange LargeMap::remove(size_t alignment, size_t size)
{
    size_t alignmentMask = alignment - 1;

    LargeRange* candidate = m_free.end();
    for (LargeRange* it = m_free.begin(); it != m_free.end(); ++it) {
        if (it->size() < size)
            continue;

        if (candidate != m_free.end() && candidate->begin() < it->begin())
            continue;

        if (test(it->begin(), alignmentMask)) {
            char* aligned = roundUpToMultipleOf(alignment, it->begin());
            if (aligned < it->begin()) // Check for overflow.
                continue;

            char* alignedEnd = aligned + size;
            if (alignedEnd < aligned) // Check for overflow.
                continue;

            if (alignedEnd > it->end())
                continue;
        }

        candidate = it;
    }
    
    if (candidate == m_free.end())
        return LargeRange();

    return m_free.pop(candidate);
}

void LargeMap::add(const LargeRange& range)
{
    LargeRange merged = range;

    for (size_t i = 0; i < m_free.size(); ++i) {
        if (!canMerge(merged, m_free[i]))
            continue;

        merged = merge(merged, m_free.pop(i--));
    }
    
    m_free.push(merged);
}

} // namespace bmalloc
