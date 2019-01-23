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
#ifndef BubbleSort_h
#define BubbleSort_h

namespace WTF {

// Why would you want to use bubble sort? When you know that your input is already mostly
// sorted! This sort is guaranteed stable (it won't reorder elements that were equal), it
// doesn't require any scratch memory, and is the fastest available sorting algorithm if your
// input already happens to be sorted. This sort is also likely to have competetive performance
// for small inputs, even if they are very unsorted.

// We use this sorting algorithm for compiler insertion sets. An insertion set is usually very
// nearly sorted. It shouldn't take more than a few bubbles to make it fully sorted. We made
// this decision deliberately. Here's the performance of the testb3 Complex(64, 384) benchmark
// with the Air::InsertionSet doing no sorting, std::stable_sorting, and bubbleSorting:
//
// no sort:          8.8222 +- 0.1911 ms.
// std::stable_sort: 9.0135 +- 0.1418 ms.
// bubbleSort:       8.8457 +- 0.1511 ms.
//
// Clearly, bubble sort is superior.
//
// Note that the critical piece here is that insertion sets tend to be small, they must be
// sorted, the sort must be stable, they are usually already sorted to begin with, and when they
// are unsorted it's usually because of a few out-of-place elements.

template<typename IteratorType, typename LessThan>
void bubbleSort(IteratorType begin, IteratorType end, const LessThan& lessThan)
{
    for (;;) {
        bool changed = false;
        ASSERT(end >= begin);
        size_t limit = end - begin;
        for (size_t i = limit; i-- > 1;) {
            if (lessThan(begin[i], begin[i - 1])) {
                std::swap(begin[i], begin[i - 1]);
                changed = true;
            }
        }
        if (!changed)
            return;
        // After one run, the first element in the list is guaranteed to be the smallest.
        begin++;

        // Now go in the other direction. This eliminates most sorting pathologies.
        changed = false;
        ASSERT(end >= begin);
        limit = end - begin;
        for (size_t i = 1; i < limit; ++i) {
            if (lessThan(begin[i], begin[i - 1])) {
                std::swap(begin[i], begin[i - 1]);
                changed = true;
            }
        }
        if (!changed)
            return;
        // Now the last element is guaranteed to be the largest.
        end--;
    }
}

template<typename IteratorType>
void bubbleSort(IteratorType begin, IteratorType end)
{
    bubbleSort(
        begin, end,
        [](auto& left, auto& right) {
            return left < right;
        });
}

} // namespace WTF

using WTF::bubbleSort;

#endif // BubbleSort_h

