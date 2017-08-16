/*
 * Copyright (C) 2014 Apple Inc. All rights reserved.
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
