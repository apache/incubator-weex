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
