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
#pragma once

#include "MarkedBlock.h"
#include "TinyBloomFilter.h"
#include <wtf/HashSet.h>

namespace JSC {

class MarkedBlock;

class MarkedBlockSet {
public:
    void add(MarkedBlock*);
    void remove(MarkedBlock*);

    TinyBloomFilter filter() const;
    const HashSet<MarkedBlock*>& set() const;

private:
    void recomputeFilter();

    TinyBloomFilter m_filter;
    HashSet<MarkedBlock*> m_set;
};

inline void MarkedBlockSet::add(MarkedBlock* block)
{
    m_filter.add(reinterpret_cast<Bits>(block));
    m_set.add(block);
}

inline void MarkedBlockSet::remove(MarkedBlock* block)
{
    unsigned oldCapacity = m_set.capacity();
    m_set.remove(block);
    if (m_set.capacity() != oldCapacity) // Indicates we've removed a lot of blocks.
        recomputeFilter();
}

inline void MarkedBlockSet::recomputeFilter()
{
    TinyBloomFilter filter;
    for (HashSet<MarkedBlock*>::iterator it = m_set.begin(); it != m_set.end(); ++it)
        filter.add(reinterpret_cast<Bits>(*it));
    m_filter = filter;
}

inline TinyBloomFilter MarkedBlockSet::filter() const
{
    return m_filter;
}

inline const HashSet<MarkedBlock*>& MarkedBlockSet::set() const
{
    return m_set;
}

} // namespace JSC
