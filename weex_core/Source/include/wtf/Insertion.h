/*
 * Copyright (C) 2013, 2015-2016 Apple Inc. All rights reserved.
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

#ifndef Insertion_h
#define Insertion_h

namespace WTF {

template<typename T>
class Insertion {
public:
    Insertion() { }

    template<typename U>
    Insertion(size_t index, U&& element)
        : m_index(index)
        , m_element(std::forward<U>(element))
    {
    }
    
    size_t index() const { return m_index; }
    const T& element() const { return m_element; }
    T& element() { return m_element; }
    
    bool operator<(const Insertion& other) const
    {
        return m_index < other.m_index;
    }
    
private:
    size_t m_index;
    T m_element;
};

template<typename TargetVectorType, typename InsertionVectorType>
size_t executeInsertions(TargetVectorType& target, InsertionVectorType& insertions)
{
    size_t numInsertions = insertions.size();
    if (!numInsertions)
        return 0;
    size_t originalTargetSize = target.size();
    target.grow(target.size() + numInsertions);
    size_t lastIndex = target.size();
    for (size_t indexInInsertions = numInsertions; indexInInsertions--;) {
        ASSERT(!indexInInsertions || insertions[indexInInsertions].index() >= insertions[indexInInsertions - 1].index());
        ASSERT_UNUSED(originalTargetSize, insertions[indexInInsertions].index() <= originalTargetSize);
        size_t firstIndex = insertions[indexInInsertions].index() + indexInInsertions;
        size_t indexOffset = indexInInsertions + 1;
        for (size_t i = lastIndex; --i > firstIndex;)
            target[i] = WTFMove(target[i - indexOffset]);
        target[firstIndex] = WTFMove(insertions[indexInInsertions].element());
        lastIndex = firstIndex;
    }
    insertions.resize(0);
    return numInsertions;
}

} // namespace WTF

using WTF::Insertion;
using WTF::executeInsertions;

#endif // Insertion_h
