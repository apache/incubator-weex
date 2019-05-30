/*
 * Copyright (C) 2015 Apple Inc. All Rights Reserved.
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

