/*
 * Copyright (C) 2015 Apple Inc. All rights reserved.
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

#ifndef ParallelVectorIterator_h
#define ParallelVectorIterator_h

#include <wtf/FastMalloc.h>
#include <wtf/Lock.h>
#include <wtf/Noncopyable.h>

namespace WTF {

template<typename VectorType>
class ParallelVectorIterator {
    WTF_MAKE_NONCOPYABLE(ParallelVectorIterator);
    WTF_MAKE_FAST_ALLOCATED;
public:
    ParallelVectorIterator(VectorType& vector, size_t shardSize)
        : m_vector(vector)
        , m_shardSize(shardSize)
    {
    }

    template<typename Functor>
    ALWAYS_INLINE void iterate(const Functor& functor)
    {
        for (;;) {
            size_t begin;
            size_t end;
            {
                LockHolder locker(m_lock);
                begin = m_next;
                if (begin == m_vector.size())
                    return;
                if (m_vector.size() - begin < m_shardSize)
                    end = m_vector.size();
                else
                    end = begin + m_shardSize;
                RELEASE_ASSERT(end <= m_vector.size());
                RELEASE_ASSERT(begin < end);
                m_next = end;
            }

            for (size_t i = begin; i < end; ++i)
                functor(m_vector[i]);
        }
    }
private:
    VectorType& m_vector;
    Lock m_lock;
    size_t m_shardSize;
    size_t m_next { 0 };
};

} // namespace WTF

using WTF::ParallelVectorIterator;

#endif // ParallelVectorIterator_h

