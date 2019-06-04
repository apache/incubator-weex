/*
 * Copyright (C) 2015 Apple Inc. All Rights Reserved.
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

#ifndef IndexSparseSet_h
#define IndexSparseSet_h

#include <wtf/Vector.h>

namespace WTF {

// IndexSparseSet is an efficient set of integers that can only be valued
// between zero and size() - 1.
//
// The implementation is using Briggs Sparse Set representation. We allocate
// memory from 0 to size() - 1 to do mapping in O(1), but we never initialize
// that memory. When adding/removing values to the set, they are added in a list
// and the corresponding bucket is initialized to the position in the list.
//
// The assumption here is that we only need a sparse subset of number live at any
// time.

template<typename OverflowHandler = CrashOnOverflow>
class IndexSparseSet {
    typedef Vector<unsigned, 0, OverflowHandler> ValueList;
public:
    explicit IndexSparseSet(unsigned size);

    bool add(unsigned);
    bool remove(unsigned);
    void clear();

    unsigned size() const;
    bool isEmpty() const;
    bool contains(unsigned) const;

    typedef typename ValueList::const_iterator const_iterator;
    const_iterator begin() const;
    const_iterator end() const;

private:
    Vector<unsigned, 0, OverflowHandler, 1> m_map;
    ValueList m_values;
};

template<typename OverflowHandler>
inline IndexSparseSet<OverflowHandler>::IndexSparseSet(unsigned size)
{
    m_map.resize(size);
}

template<typename OverflowHandler>
inline bool IndexSparseSet<OverflowHandler>::add(unsigned value)
{
    if (contains(value))
        return false;

    unsigned newPosition = m_values.size();
    m_values.append(value);
    m_map[value] = newPosition;
    return true;
}

template<typename OverflowHandler>
inline bool IndexSparseSet<OverflowHandler>::remove(unsigned value)
{
    unsigned position = m_map[value];
    if (position >= m_values.size())
        return false;

    if (m_values[position] == value) {
        unsigned lastValue = m_values.last();
        m_values[position] = lastValue;
        m_map[lastValue] = position;
        m_values.removeLast();
        return true;
    }

    return false;
}

template<typename OverflowHandler>
void IndexSparseSet<OverflowHandler>::clear()
{
    m_values.resize(0);
}

template<typename OverflowHandler>
unsigned IndexSparseSet<OverflowHandler>::size() const
{
    return m_values.size();
}

template<typename OverflowHandler>
bool IndexSparseSet<OverflowHandler>::isEmpty() const
{
    return !size();
}

template<typename OverflowHandler>
bool IndexSparseSet<OverflowHandler>::contains(unsigned value) const
{
    unsigned position = m_map[value];
    if (position >= m_values.size())
        return false;

    return m_values[position] == value;
}

template<typename OverflowHandler>
auto IndexSparseSet<OverflowHandler>::begin() const -> const_iterator
{
    return m_values.begin();
}

template<typename OverflowHandler>
auto IndexSparseSet<OverflowHandler>::end() const -> const_iterator
{
    return m_values.end();
}

} // namespace WTF

using WTF::IndexSparseSet;

#endif // IndexSparseSet_h
