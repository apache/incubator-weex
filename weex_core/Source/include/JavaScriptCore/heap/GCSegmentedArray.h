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

#include <wtf/DoublyLinkedList.h>
#include <wtf/ListDump.h>
#include <wtf/PrintStream.h>
#include <wtf/Vector.h>

namespace JSC {

template <typename T>
class GCArraySegment : public DoublyLinkedListNode<GCArraySegment<T>> {
    friend class WTF::DoublyLinkedListNode<GCArraySegment<T>>;
public:
    GCArraySegment()
        : DoublyLinkedListNode<GCArraySegment<T>>()
#if !ASSERT_DISABLED
        , m_top(0)
#endif
    {
    }

    static GCArraySegment* create();
    static void destroy(GCArraySegment*);

    T* data()
    {
        return bitwise_cast<T*>(this + 1);
    }

    static const size_t blockSize = 4 * KB;

    GCArraySegment* m_prev;
    GCArraySegment* m_next;
#if !ASSERT_DISABLED
    size_t m_top;
#endif
};

template <typename T> class GCSegmentedArrayIterator;

template <typename T>
class GCSegmentedArray {
    friend class GCSegmentedArrayIterator<T>;
    friend class GCSegmentedArrayIterator<const T>;
public:
    GCSegmentedArray();
    ~GCSegmentedArray();

    void append(T);

    bool canRemoveLast();
    const T removeLast();
    bool refill();
    
    size_t size();
    bool isEmpty();

    void fillVector(Vector<T>&);
    void clear();

    typedef GCSegmentedArrayIterator<T> iterator;
    iterator begin() const { return GCSegmentedArrayIterator<T>(m_segments.head(), m_top); }
    iterator end() const { return GCSegmentedArrayIterator<T>(); }

protected:
    template <size_t size> struct CapacityFromSize {
        static const size_t value = (size - sizeof(GCArraySegment<T>)) / sizeof(T);
    };

    void expand();
    
    size_t postIncTop();
    size_t preDecTop();
    void setTopForFullSegment();
    void setTopForEmptySegment();
    size_t top();
    
    void validatePrevious();

    DoublyLinkedList<GCArraySegment<T>> m_segments;

    JS_EXPORT_PRIVATE static const size_t s_segmentCapacity = CapacityFromSize<GCArraySegment<T>::blockSize>::value;
    size_t m_top;
    size_t m_numberOfSegments;
};

template <typename T>
class GCSegmentedArrayIterator {
    friend class GCSegmentedArray<T>;
public:
    GCSegmentedArrayIterator()
        : m_currentSegment(0)
        , m_currentOffset(0)
    {
    }

    T& get() { return m_currentSegment->data()[m_currentOffset]; }
    T& operator*() { return get(); }
    T& operator->() { return get(); }

    bool operator==(const GCSegmentedArrayIterator& other)
    {
        return m_currentSegment == other.m_currentSegment && m_currentOffset == other.m_currentOffset;
    }

    bool operator!=(const GCSegmentedArrayIterator& other)
    {
        return !(*this == other);
    }

    GCSegmentedArrayIterator& operator++()
    {
        ASSERT(m_currentSegment);

        m_currentOffset++;

        if (m_currentOffset >= m_offsetLimit) {
            m_currentSegment = m_currentSegment->next();
            m_currentOffset = 0;
            m_offsetLimit = GCSegmentedArray<T>::s_segmentCapacity;
        }

        return *this;
    }

private:
    GCSegmentedArrayIterator(GCArraySegment<T>* start, size_t top)
        : m_currentSegment(start)
        , m_currentOffset(0)
        , m_offsetLimit(top)
    {
        if (!m_offsetLimit)
            m_currentSegment = nullptr;
    }

    GCArraySegment<T>* m_currentSegment;
    size_t m_currentOffset;
    size_t m_offsetLimit;
};

} // namespace JSC

