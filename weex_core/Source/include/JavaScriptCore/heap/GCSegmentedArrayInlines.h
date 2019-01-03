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

#include "GCSegmentedArray.h"

namespace JSC {

template <typename T>
GCSegmentedArray<T>::GCSegmentedArray()
    : m_top(0)
    , m_numberOfSegments(0)
{
    m_segments.push(GCArraySegment<T>::create());
    m_numberOfSegments++;
}

template <typename T>
GCSegmentedArray<T>::~GCSegmentedArray()
{
    ASSERT(m_numberOfSegments == 1);
    ASSERT(m_segments.size() == 1);
    GCArraySegment<T>::destroy(m_segments.removeHead());
    m_numberOfSegments--;
    ASSERT(!m_numberOfSegments);
    ASSERT(!m_segments.size());
}

template <typename T>
void GCSegmentedArray<T>::clear()
{
    if (!m_segments.head())
        return;
    GCArraySegment<T>* next;
    for (GCArraySegment<T>* current = m_segments.head(); current->next(); current = next) {
        next = current->next();
        m_segments.remove(current);
        GCArraySegment<T>::destroy(current);
    }
    m_top = 0;
    m_numberOfSegments = 1;
#if !ASSERT_DISABLED
    m_segments.head()->m_top = 0;
#endif
}

template <typename T>
void GCSegmentedArray<T>::expand()
{
    ASSERT(m_segments.head()->m_top == s_segmentCapacity);
    
    GCArraySegment<T>* nextSegment = GCArraySegment<T>::create();
    m_numberOfSegments++;
    
#if !ASSERT_DISABLED
    nextSegment->m_top = 0;
#endif

    m_segments.push(nextSegment);
    setTopForEmptySegment();
    validatePrevious();
}

template <typename T>
bool GCSegmentedArray<T>::refill()
{
    validatePrevious();
    if (top())
        return true;
    GCArraySegment<T>::destroy(m_segments.removeHead());
    ASSERT(m_numberOfSegments > 1);
    m_numberOfSegments--;
    setTopForFullSegment();
    validatePrevious();
    return true;
}

template <typename T>
void GCSegmentedArray<T>::fillVector(Vector<T>& vector)
{
    ASSERT(vector.size() == size());

    GCArraySegment<T>* currentSegment = m_segments.head();
    if (!currentSegment)
        return;

    unsigned count = 0;
    for (unsigned i = 0; i < m_top; ++i) {
        ASSERT(currentSegment->data()[i]);
        vector[count++] = currentSegment->data()[i];
    }

    currentSegment = currentSegment->next();
    while (currentSegment) {
        for (unsigned i = 0; i < s_segmentCapacity; ++i) {
            ASSERT(currentSegment->data()[i]);
            vector[count++] = currentSegment->data()[i];
        }
        currentSegment = currentSegment->next();
    }
}

template <typename T>
inline GCArraySegment<T>* GCArraySegment<T>::create()
{
    return new (NotNull, fastMalloc(blockSize)) GCArraySegment<T>();
}

template <typename T>
inline void GCArraySegment<T>::destroy(GCArraySegment* segment)
{
    segment->~GCArraySegment();
    fastFree(segment);
}

template <typename T>
inline size_t GCSegmentedArray<T>::postIncTop()
{
    size_t result = m_top++;
    ASSERT(result == m_segments.head()->m_top++);
    return result;
}

template <typename T>
inline size_t GCSegmentedArray<T>::preDecTop()
{
    size_t result = --m_top;
    ASSERT(result == --m_segments.head()->m_top);
    return result;
}

template <typename T>
inline void GCSegmentedArray<T>::setTopForFullSegment()
{
    ASSERT(m_segments.head()->m_top == s_segmentCapacity);
    m_top = s_segmentCapacity;
}

template <typename T>
inline void GCSegmentedArray<T>::setTopForEmptySegment()
{
    ASSERT(!m_segments.head()->m_top);
    m_top = 0;
}

template <typename T>
inline size_t GCSegmentedArray<T>::top()
{
    ASSERT(m_top == m_segments.head()->m_top);
    return m_top;
}

template <typename T>
#if ASSERT_DISABLED
inline void GCSegmentedArray<T>::validatePrevious() { }
#else
inline void GCSegmentedArray<T>::validatePrevious()
{
    unsigned count = 0;
    for (GCArraySegment<T>* current = m_segments.head(); current; current = current->next())
        count++;
    ASSERT(m_segments.size() == m_numberOfSegments);
}
#endif

template <typename T>
inline void GCSegmentedArray<T>::append(T value)
{
    if (m_top == s_segmentCapacity)
        expand();
    m_segments.head()->data()[postIncTop()] = value;
}

template <typename T>
inline bool GCSegmentedArray<T>::canRemoveLast()
{
    return !!m_top;
}

template <typename T>
inline const T GCSegmentedArray<T>::removeLast()
{
    return m_segments.head()->data()[preDecTop()];
}

template <typename T>
inline bool GCSegmentedArray<T>::isEmpty()
{
    if (m_top)
        return false;
    if (m_segments.head()->next()) {
        ASSERT(m_segments.head()->next()->m_top == s_segmentCapacity);
        return false;
    }
    return true;
}

template <typename T>
inline size_t GCSegmentedArray<T>::size()
{
    return m_top + s_segmentCapacity * (m_numberOfSegments - 1);
}

} // namespace JSC
