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
