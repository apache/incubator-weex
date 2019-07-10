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

#ifndef WTF_RangeSet_h
#define WTF_RangeSet_h

#include <wtf/ListDump.h>
#include <wtf/MathExtras.h>
#include <wtf/StdLibExtras.h>
#include <wtf/Vector.h>

namespace WTF {

// A RangeSet is a set of numerical ranges. A value belongs to the set if it is within any of the
// ranges. A range belongs to the set if every value in the range belongs to the set. A range overlaps
// the set if any value in the range belongs to the set. You can add ranges and query range
// membership. The internal representation is a list of ranges that gets periodically compacted. This
// representation is optimal so long as the number of distinct ranges tends to be small, and the
// number of range sets tends to be small as well. This works reasonably well in a bunch of compiler
// algorithms, where the top range ends up being used a lot.
//
// The initial user of this is JSC::B3::HeapRange, which is used to perform alias analysis. You can
// model new users on that class. Basically, you need to define:
//
// T::Type - the type of the members of the range. HeapRange uses unsigned.
// T(T::Type begin, T::Type end) - construct a new range.
// T::Type T::begin() const - instance method giving the inclusive beginning of the range.
// T::Type T::end() const - instance method giving the exclusive end of the range.
// void T::dump(PrintStream&) const - some kind of dumping.

template<typename RangeType>
class RangeSet {
public:
    typedef RangeType Range;
    typedef typename Range::Type Type;

    RangeSet()
    {
    }

    ~RangeSet()
    {
    }

    void add(const Range& range)
    {
        if (range.begin() == range.end())
            return;
        
        // We expect the range set to become top in a lot of cases. We also expect the same range to
        // be added repeatedly. That's why this is here.
        if (!m_ranges.isEmpty() && subsumesNonEmpty(m_ranges.last(), range))
            return;

        m_isCompact = false;

        // We append without compacting only if doing so is guaranteed not to resize the vector.
        if (m_ranges.size() + 1 < m_ranges.capacity()) {
            m_ranges.append(range);
            return;
        }

        m_ranges.append(range);
        compact();
    }

    bool contains(const Range& range) const
    {
        if (range.begin() == range.end())
            return false;
        
        unsigned index = findRange(range);
        if (index != UINT_MAX)
            return subsumesNonEmpty(m_ranges[index], range);
        return false;
    }

    bool overlaps(const Range& range) const
    {
        if (range.begin() == range.end())
            return false;
        
        return findRange(range) != UINT_MAX;
    }

    void clear()
    {
        m_ranges.clear();
        m_isCompact = true;
    }

    void dump(PrintStream& out) const
    {
        const_cast<RangeSet*>(this)->compact();
        out.print(listDump(m_ranges));
    }

    void dumpRaw(PrintStream& out) const
    {
        out.print("{", listDump(m_ranges), ", isCompact = ", m_isCompact, "}");
    }

private:
    void compact()
    {
        if (m_isCompact)
            return;

        if (m_ranges.isEmpty()) {
            m_isCompact = true;
            return;
        }

        std::sort(
            m_ranges.begin(), m_ranges.end(),
            [&] (const Range& a, const Range& b) -> bool {
                return a.begin() < b.begin();
            });

        unsigned srcIndex = 1;
        unsigned dstIndex = 1;
        Range* lastRange = &m_ranges[0];
        while (srcIndex < m_ranges.size()) {
            Range range = m_ranges[srcIndex++];
            ASSERT(range.begin() >= lastRange->begin());
            if (range.end() <= lastRange->end())
                continue;
            if (range.begin() <= lastRange->end()) {
                *lastRange = Range(lastRange->begin(), range.end());
                continue;
            }
            ASSERT(!overlapsNonEmpty(*lastRange, range));
            lastRange = &m_ranges[dstIndex++];
            *lastRange = range;
        }
        m_ranges.resize(dstIndex);

        m_isCompact = true;
    }
    
    static bool overlapsNonEmpty(const Range& a, const Range& b)
    {
        return nonEmptyRangesOverlap(a.begin(), a.end(), b.begin(), b.end());
    }

    static bool subsumesNonEmpty(const Range& a, const Range& b)
    {
        return a.begin() <= b.begin() && a.end() >= b.end();
    }

    unsigned findRange(const Range& range) const
    {
        const_cast<RangeSet*>(this)->compact();

        // FIXME: Once we start using this in anger, we will want this to be a binary search.
        for (unsigned i = 0; i < m_ranges.size(); ++i) {
            if (overlapsNonEmpty(m_ranges[i], range))
                return i;
        }
        
        return UINT_MAX;
    }
    
    Vector<Range, 8> m_ranges;
    bool m_isCompact { true };
};

} // namespace WTF

using WTF::RangeSet;

#endif // WTF_RangeSet_h

