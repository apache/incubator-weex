/*
 * Copyright (C) 2016 Apple Inc. All rights reserved.
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

#include "config.h"
#include "FastBitVector.h"

namespace WTF {

void FastBitVectorWordOwner::setEqualsSlow(const FastBitVectorWordOwner& other)
{
    uint32_t* newArray = static_cast<uint32_t*>(
        fastCalloc(other.arrayLength(), sizeof(uint32_t)));
    memcpy(newArray, other.m_words, other.arrayLength() * sizeof(uint32_t));
    if (m_words)
        fastFree(m_words);
    m_words = newArray;
    m_numBits = other.m_numBits;
}

void FastBitVectorWordOwner::resizeSlow(size_t numBits)
{
    size_t newLength = fastBitVectorArrayLength(numBits);
    
    // Use fastCalloc instead of fastRealloc because we expect the common
    // use case for this method to be initializing the size of the bitvector.
    
    uint32_t* newArray = static_cast<uint32_t*>(fastCalloc(newLength, sizeof(uint32_t)));
    memcpy(newArray, m_words, arrayLength() * sizeof(uint32_t));
    if (m_words)
        fastFree(m_words);
    m_words = newArray;
}

void FastBitVector::clearRange(size_t begin, size_t end)
{
    if (end - begin < 32) {
        for (size_t i = begin; i < end; ++i)
            at(i) = false;
        return;
    }
    
    size_t endBeginSlop = (begin + 31) & ~31;
    size_t beginEndSlop = end & ~31;
    
    for (size_t i = begin; i < endBeginSlop; ++i)
        at(i) = false;
    for (size_t i = beginEndSlop; i < end; ++i)
        at(i) = false;
    for (size_t i = endBeginSlop / 32; i < beginEndSlop / 32; ++i)
        m_words.word(i) = 0;
}

} // namespace WTF

