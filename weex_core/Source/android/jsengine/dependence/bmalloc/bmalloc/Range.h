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
#ifndef Range_h
#define Range_h

#include <algorithm>
#include <cstddef>

namespace bmalloc {

class Range {
public:
    Range()
        : m_begin(0)
        , m_size(0)
    {
    }

    Range(void* start, size_t size)
        : m_begin(static_cast<char*>(start))
        , m_size(size)
    {
    }

    char* begin() const { return m_begin; }
    char* end() const { return m_begin + m_size; }
    size_t size() const { return m_size; }
    
    bool operator!() const { return !m_size; }
    explicit operator bool() const { return !!*this; }
    bool operator<(const Range& other) const { return m_begin < other.m_begin; }

private:
    char* m_begin;
    size_t m_size;
};

inline bool canMerge(const Range& a, const Range& b)
{
    return a.begin() == b.end() || a.end() == b.begin();
}

inline Range merge(const Range& a, const Range& b)
{
    return Range(std::min(a.begin(), b.begin()), a.size() + b.size());
}

} // namespace bmalloc

#endif // Range_h
