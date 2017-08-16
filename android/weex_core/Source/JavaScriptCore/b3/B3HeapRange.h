/*
 * Copyright (C) 2015-2017 Apple Inc. All rights reserved.
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

#pragma once

#if ENABLE(B3_JIT)

#include <limits.h>
#include <wtf/MathExtras.h>
#include <wtf/PrintStream.h>

namespace JSC { namespace B3 {

// Alias analysis in B3 is done by checking if two integer ranges overlap. This is powerful enough
// to be used for TBAA-style alias analysis used by the DFG, FTL, and LLVM: you just turn each node
// in the tree of abstract heaps into a pre/post range.
//
// Note that the 'begin' is inclusive, while the 'end' is exclusive. These two ranges are non-
// overlapping:
//
//     rangeA = 0...8
//     rangeB = 8...16

class HeapRange {
public:
    typedef unsigned Type;
    
    HeapRange()
        : m_begin(0)
        , m_end(0)
    {
    }

    explicit HeapRange(unsigned value)
        : m_begin(value)
        , m_end(value + 1)
    {
        ASSERT(m_end >= m_begin);
    }

    HeapRange(unsigned begin, unsigned end)
        : m_begin(begin)
        , m_end(end)
    {
        ASSERT(m_end >= m_begin);
        if (m_begin == m_end) {
            // Canonicalize empty ranges.
            m_begin = 0;
            m_end = 0;
        }
    }

    static HeapRange top()
    {
        return HeapRange(0, UINT_MAX);
    }

    bool operator==(const HeapRange& other) const
    {
        return m_begin == other.m_begin
            && m_end == other.m_end;
    }

    bool operator!=(const HeapRange& other) const
    {
        return !(*this == other);
    }
    
    HeapRange operator|(const HeapRange& other) const
    {
        return HeapRange(
            std::min(m_begin, other.m_begin),
            std::max(m_end, other.m_end));
    }
    
    explicit operator bool() const { return m_begin != m_end; }

    unsigned begin() const { return m_begin; }
    unsigned end() const { return m_end; }

    bool overlaps(const HeapRange& other) const
    {
        return WTF::rangesOverlap(m_begin, m_end, other.m_begin, other.m_end);
    }

    JS_EXPORT_PRIVATE void dump(PrintStream& out) const;

private:
    unsigned m_begin;
    unsigned m_end;
};

} } // namespace JSC::B3

#endif // ENABLE(B3_JIT)
