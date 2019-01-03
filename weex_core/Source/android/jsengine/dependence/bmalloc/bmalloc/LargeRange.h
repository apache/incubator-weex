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
#ifndef LargeRange_h
#define LargeRange_h

#include "BAssert.h"
#include "Range.h"

namespace bmalloc {

class LargeRange : public Range {
public:
    LargeRange()
        : Range()
        , m_physicalSize(0)
    {
    }

    LargeRange(const Range& other, size_t physicalSize)
        : Range(other)
        , m_physicalSize(physicalSize)
    {
    }

    LargeRange(void* begin, size_t size, size_t physicalSize)
        : Range(begin, size)
        , m_physicalSize(physicalSize)
    {
    }

    size_t physicalSize() const { return m_physicalSize; }
    void setPhysicalSize(size_t physicalSize) { m_physicalSize = physicalSize; }

    std::pair<LargeRange, LargeRange> split(size_t) const;

    bool operator<(const void* other) const { return begin() < other; }
    bool operator<(const LargeRange& other) const { return begin() < other.begin(); }

private:
    size_t m_physicalSize;
};

inline bool canMerge(const LargeRange& a, const LargeRange& b)
{
    if (a.end() == b.begin())
        return true;
    
    if (b.end() == a.begin())
        return true;
    
    return false;
}

inline LargeRange merge(const LargeRange& a, const LargeRange& b)
{
    const LargeRange& left = std::min(a, b);
    if (left.size() == left.physicalSize()) {
        return LargeRange(
            left.begin(),
            a.size() + b.size(),
            a.physicalSize() + b.physicalSize());
    }

    return LargeRange(
        left.begin(),
        a.size() + b.size(),
        left.physicalSize());
}

inline std::pair<LargeRange, LargeRange> LargeRange::split(size_t size) const
{
    BASSERT(size <= this->size());
    
    if (size <= physicalSize()) {
        LargeRange left(begin(), size, size);
        LargeRange right(left.end(), this->size() - size, physicalSize() - size);
        return std::make_pair(left, right);
    }

    LargeRange left(begin(), size, physicalSize());
    LargeRange right(left.end(), this->size() - size, 0);
    return std::make_pair(left, right);
}

} // namespace bmalloc

#endif // LargeRange_h
