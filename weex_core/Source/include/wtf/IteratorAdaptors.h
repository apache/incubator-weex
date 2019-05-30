/*
 * Copyright (C) 2013 Apple Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
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
