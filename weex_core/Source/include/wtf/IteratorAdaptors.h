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
#ifndef WTF_IteratorAdaptors_h
#define WTF_IteratorAdaptors_h

#include <type_traits>

namespace WTF {

template<typename Predicate, typename Iterator>
class FilterIterator {
public:
    FilterIterator(Predicate pred, Iterator begin, Iterator end)
        : m_pred(WTFMove(pred))
        , m_iter(WTFMove(begin))
        , m_end(WTFMove(end))
    {
        while (m_iter != m_end && !m_pred(*m_iter))
            ++m_iter;
    }

    FilterIterator& operator++()
    {
        while (m_iter != m_end) {
            ++m_iter;
            if (m_iter == m_end || m_pred(*m_iter))
                break;
        }
        return *this;
    }

    const typename std::remove_const<decltype(*std::declval<Iterator>())>::type operator*() const
    {
        ASSERT(m_iter != m_end);
        ASSERT(m_pred(*m_iter));
        return *m_iter;
    }

    inline bool operator==(FilterIterator& other) const { return m_iter == other.m_iter; }
    inline bool operator!=(FilterIterator& other) const { return m_iter != other.m_iter; }

private:
    const Predicate m_pred;
    Iterator m_iter;
    Iterator m_end;
};

template<typename Predicate, typename Iterator>
inline FilterIterator<Predicate, Iterator> makeFilterIterator(Predicate&& pred, Iterator&& begin, Iterator&& end)
{
    return FilterIterator<Predicate, Iterator>(std::forward<Predicate>(pred), std::forward<Iterator>(begin), std::forward<Iterator>(end));
}

template<typename Transform, typename Iterator>
class TransformIterator {
public:
    TransformIterator(Transform&& transform, Iterator&& iter)
        : m_transform(WTFMove(transform))
        , m_iter(WTFMove(iter))
    {
    }

    TransformIterator& operator++()
    {
        ++m_iter;
        return *this;
    }

    const typename std::remove_const<decltype(std::declval<Transform>()(*std::declval<Iterator>()))>::type operator*() const
    {
        return m_transform(*m_iter);
    }

    inline bool operator==(TransformIterator& other) const { return m_iter == other.m_iter; }
    inline bool operator!=(TransformIterator& other) const { return m_iter != other.m_iter; }

private:
    const Transform m_transform;
    Iterator m_iter;
};

template<typename Transform, typename Iterator>
inline TransformIterator<Transform, Iterator> makeTransformIterator(Transform&& transform, Iterator&& iter)
{
    return TransformIterator<Transform, Iterator>(WTFMove(transform), WTFMove(iter));
}

} // namespace WTF

#endif // WTF_IteratorAdaptors_h
