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
#ifndef SegmentedVector_h
#define SegmentedVector_h

#include <wtf/Noncopyable.h>
#include <wtf/Vector.h>

namespace WTF {

    // An iterator for SegmentedVector. It supports only the pre ++ operator
    template <typename T, size_t SegmentSize = 8> class SegmentedVector;
    template <typename T, size_t SegmentSize = 8> class SegmentedVectorIterator {
    private:
        friend class SegmentedVector<T, SegmentSize>;
    public:
        typedef SegmentedVectorIterator<T, SegmentSize> Iterator;

        ~SegmentedVectorIterator() { }

        T& operator*() const { return m_vector.at(m_index); }
        T* operator->() const { return &m_vector.at(m_index); }

        // Only prefix ++ operator supported
        Iterator& operator++()
        {
            m_index++;
            return *this;
        }

        bool operator==(const Iterator& other) const
        {
            return m_index == other.m_index && &m_vector == &other.m_vector;
        }

        bool operator!=(const Iterator& other) const
        {
            return m_index != other.m_index || &m_vector != &other.m_vector;
        }

        SegmentedVectorIterator& operator=(const SegmentedVectorIterator<T, SegmentSize>& other)
        {
            m_vector = other.m_vector;
            m_index = other.m_index;
            return *this;
        }

    private:
        SegmentedVectorIterator(SegmentedVector<T, SegmentSize>& vector, size_t index)
            : m_vector(vector)
            , m_index(index)
        {
        }

        SegmentedVector<T, SegmentSize>& m_vector;
        size_t m_index;
    };

    // SegmentedVector is just like Vector, but it doesn't move the values
    // stored in its buffer when it grows. Therefore, it is safe to keep
    // pointers into a SegmentedVector. The default tuning values are
    // optimized for segmented vectors that get large; you may want to use
    // SegmentedVector<thingy, 1> if you don't expect a lot of entries.
    template <typename T, size_t SegmentSize>
    class SegmentedVector {
        friend class SegmentedVectorIterator<T, SegmentSize>;
        WTF_MAKE_NONCOPYABLE(SegmentedVector);
        WTF_MAKE_FAST_ALLOCATED;

    public:
        typedef SegmentedVectorIterator<T, SegmentSize> Iterator;

        SegmentedVector() = default;

        ~SegmentedVector()
        {
            deleteAllSegments();
        }

        size_t size() const { return m_size; }
        bool isEmpty() const { return !size(); }

        T& at(size_t index)
        {
            ASSERT_WITH_SECURITY_IMPLICATION(index < m_size);
            return segmentFor(index)->entries[subscriptFor(index)];
        }

        const T& at(size_t index) const
        {
            return const_cast<SegmentedVector<T, SegmentSize>*>(this)->at(index);
        }

        T& operator[](size_t index)
        {
            return at(index);
        }

        const T& operator[](size_t index) const
        {
            return at(index);
        }

        T& first()
        {
            ASSERT_WITH_SECURITY_IMPLICATION(!isEmpty());
            return at(0);
        }
        const T& first() const
        {
            ASSERT_WITH_SECURITY_IMPLICATION(!isEmpty());
            return at(0);
        }
        T& last()
        {
            ASSERT_WITH_SECURITY_IMPLICATION(!isEmpty());
            return at(size() - 1);
        }
        const T& last() const
        {
            ASSERT_WITH_SECURITY_IMPLICATION(!isEmpty());
            return at(size() - 1);
        }

        T takeLast()
        {
            ASSERT_WITH_SECURITY_IMPLICATION(!isEmpty());
            T result = WTFMove(last());
            --m_size;
            return result;
        }

        template<typename... Args>
        void append(Args&&... args)
        {
            ++m_size;
            if (!segmentExistsFor(m_size - 1))
                allocateSegment();
            new (NotNull, &last()) T(std::forward<Args>(args)...);
        }

        template<typename... Args>
        T& alloc(Args&&... args)
        {
            append(std::forward<Args>(args)...);
            return last();
        }

        void removeLast()
        {
            last().~T();
            --m_size;
        }

        void grow(size_t size)
        {
            ASSERT(size > m_size);
            ensureSegmentsFor(size);
            size_t oldSize = m_size;
            m_size = size;
            for (size_t i = oldSize; i < m_size; ++i)
                new (NotNull, &at(i)) T();
        }

        void clear()
        {
            deleteAllSegments();
            m_segments.clear();
            m_size = 0;
        }

        Iterator begin()
        {
            return Iterator(*this, 0);
        }

        Iterator end()
        {
            return Iterator(*this, m_size);
        }
        
        void shrinkToFit()
        {
            m_segments.shrinkToFit();
        }

    private:
        struct Segment {
#if COMPILER(MSVC)
#pragma warning(push)
#pragma warning(disable: 4200)
#endif
            T entries[0];
#if COMPILER(MSVC)
#pragma warning(pop)
#endif
        };

        void deleteAllSegments()
        {
            for (size_t i = 0; i < m_size; ++i)
                at(i).~T();
            for (size_t i = 0; i < m_segments.size(); ++i)
                fastFree(m_segments[i]);
        }

        bool segmentExistsFor(size_t index)
        {
            return index / SegmentSize < m_segments.size();
        }

        Segment* segmentFor(size_t index)
        {
            return m_segments[index / SegmentSize];
        }

        size_t subscriptFor(size_t index)
        {
            return index % SegmentSize;
        }

        void ensureSegmentsFor(size_t size)
        {
            size_t segmentCount = (m_size + SegmentSize - 1) / SegmentSize;
            size_t neededSegmentCount = (size + SegmentSize - 1) / SegmentSize;

            for (size_t i = segmentCount ? segmentCount - 1 : 0; i < neededSegmentCount; ++i)
                ensureSegment(i);
        }

        void ensureSegment(size_t segmentIndex)
        {
            ASSERT_WITH_SECURITY_IMPLICATION(segmentIndex <= m_segments.size());
            if (segmentIndex == m_segments.size())
                allocateSegment();
        }

        void allocateSegment()
        {
            m_segments.append(static_cast<Segment*>(fastMalloc(sizeof(T) * SegmentSize)));
        }

        size_t m_size { 0 };
        Vector<Segment*> m_segments;
    };

} // namespace WTF

using WTF::SegmentedVector;

#endif // SegmentedVector_h
