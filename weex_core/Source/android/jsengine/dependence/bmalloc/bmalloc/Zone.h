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
