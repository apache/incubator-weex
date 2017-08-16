/*
 * Copyright (C) 2011 Apple Inc. All rights reserved.
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
#include "BitVector.h"

#include <algorithm>
#include <string.h>
#include <wtf/Assertions.h>
#include <wtf/FastMalloc.h>
#include <wtf/StdLibExtras.h>

namespace WTF {

void BitVector::setSlow(const BitVector& other)
{
    uintptr_t newBitsOrPointer;
    if (other.isInline() || other.isEmptyOrDeletedValue())
        newBitsOrPointer = other.m_bitsOrPointer;
    else {
        OutOfLineBits* newOutOfLineBits = OutOfLineBits::create(other.size());
        memcpy(newOutOfLineBits->bits(), other.bits(), byteCount(other.size()));
        newBitsOrPointer = bitwise_cast<uintptr_t>(newOutOfLineBits) >> 1;
    }
    if (!isInline() && !isEmptyOrDeletedValue())
        OutOfLineBits::destroy(outOfLineBits());
    m_bitsOrPointer = newBitsOrPointer;
}

void BitVector::resize(size_t numBits)
{
    if (numBits <= maxInlineBits()) {
        if (isInline())
            return;
    
        OutOfLineBits* myOutOfLineBits = outOfLineBits();
        m_bitsOrPointer = makeInlineBits(*myOutOfLineBits->bits());
        OutOfLineBits::destroy(myOutOfLineBits);
        return;
    }
    
    resizeOutOfLine(numBits);
}

void BitVector::clearAll()
{
    if (isInline())
        m_bitsOrPointer = makeInlineBits(0);
    else
        memset(outOfLineBits()->bits(), 0, byteCount(size()));
}

BitVector::OutOfLineBits* BitVector::OutOfLineBits::create(size_t numBits)
{
    numBits = (numBits + bitsInPointer() - 1) & ~(bitsInPointer() - 1);
    size_t size = sizeof(OutOfLineBits) + sizeof(uintptr_t) * (numBits / bitsInPointer());
    OutOfLineBits* result = new (NotNull, fastMalloc(size)) OutOfLineBits(numBits);
    return result;
}

void BitVector::OutOfLineBits::destroy(OutOfLineBits* outOfLineBits)
{
    fastFree(outOfLineBits);
}

void BitVector::resizeOutOfLine(size_t numBits)
{
    ASSERT(numBits > maxInlineBits());
    OutOfLineBits* newOutOfLineBits = OutOfLineBits::create(numBits);
    size_t newNumWords = newOutOfLineBits->numWords();
    if (isInline()) {
        // Make sure that all of the bits are zero in case we do a no-op resize.
        *newOutOfLineBits->bits() = m_bitsOrPointer & ~(static_cast<uintptr_t>(1) << maxInlineBits());
        memset(newOutOfLineBits->bits() + 1, 0, (newNumWords - 1) * sizeof(void*));
    } else {
        if (numBits > size()) {
            size_t oldNumWords = outOfLineBits()->numWords();
            memcpy(newOutOfLineBits->bits(), outOfLineBits()->bits(), oldNumWords * sizeof(void*));
            memset(newOutOfLineBits->bits() + oldNumWords, 0, (newNumWords - oldNumWords) * sizeof(void*));
        } else
            memcpy(newOutOfLineBits->bits(), outOfLineBits()->bits(), newOutOfLineBits->numWords() * sizeof(void*));
        OutOfLineBits::destroy(outOfLineBits());
    }
    m_bitsOrPointer = bitwise_cast<uintptr_t>(newOutOfLineBits) >> 1;
}

void BitVector::mergeSlow(const BitVector& other)
{
    if (other.isInline()) {
        ASSERT(!isInline());
        *bits() |= cleanseInlineBits(other.m_bitsOrPointer);
        return;
    }
    
    ensureSize(other.size());
    ASSERT(!isInline());
    ASSERT(!other.isInline());
    
    OutOfLineBits* a = outOfLineBits();
    const OutOfLineBits* b = other.outOfLineBits();
    for (unsigned i = a->numWords(); i--;)
        a->bits()[i] |= b->bits()[i];
}

void BitVector::filterSlow(const BitVector& other)
{
    if (other.isInline()) {
        ASSERT(!isInline());
        *bits() &= cleanseInlineBits(other.m_bitsOrPointer);
        return;
    }
    
    if (isInline()) {
        ASSERT(!other.isInline());
        m_bitsOrPointer &= *other.outOfLineBits()->bits();
        m_bitsOrPointer |= (static_cast<uintptr_t>(1) << maxInlineBits());
        ASSERT(isInline());
        return;
    }
    
    OutOfLineBits* a = outOfLineBits();
    const OutOfLineBits* b = other.outOfLineBits();
    for (unsigned i = std::min(a->numWords(), b->numWords()); i--;)
        a->bits()[i] &= b->bits()[i];
    
    for (unsigned i = b->numWords(); i < a->numWords(); ++i)
        a->bits()[i] = 0;
}

void BitVector::excludeSlow(const BitVector& other)
{
    if (other.isInline()) {
        ASSERT(!isInline());
        *bits() &= ~cleanseInlineBits(other.m_bitsOrPointer);
        return;
    }
    
    if (isInline()) {
        ASSERT(!other.isInline());
        m_bitsOrPointer &= ~*other.outOfLineBits()->bits();
        m_bitsOrPointer |= (static_cast<uintptr_t>(1) << maxInlineBits());
        ASSERT(isInline());
        return;
    }
    
    OutOfLineBits* a = outOfLineBits();
    const OutOfLineBits* b = other.outOfLineBits();
    for (unsigned i = std::min(a->numWords(), b->numWords()); i--;)
        a->bits()[i] &= ~b->bits()[i];
}

size_t BitVector::bitCountSlow() const
{
    ASSERT(!isInline());
    const OutOfLineBits* bits = outOfLineBits();
    size_t result = 0;
    for (unsigned i = bits->numWords(); i--;)
        result += bitCount(bits->bits()[i]);
    return result;
}

bool BitVector::equalsSlowCase(const BitVector& other) const
{
    bool result = equalsSlowCaseFast(other);
    ASSERT(result == equalsSlowCaseSimple(other));
    return result;
}

bool BitVector::equalsSlowCaseFast(const BitVector& other) const
{
    if (isInline() != other.isInline())
        return equalsSlowCaseSimple(other);
    
    const OutOfLineBits* myBits = outOfLineBits();
    const OutOfLineBits* otherBits = other.outOfLineBits();
    
    size_t myNumWords = myBits->numWords();
    size_t otherNumWords = otherBits->numWords();
    size_t minNumWords;
    size_t maxNumWords;
    
    const OutOfLineBits* longerBits;
    if (myNumWords < otherNumWords) {
        minNumWords = myNumWords;
        maxNumWords = otherNumWords;
        longerBits = otherBits;
    } else {
        minNumWords = otherNumWords;
        maxNumWords = myNumWords;
        longerBits = myBits;
    }
    
    for (size_t i = minNumWords; i < maxNumWords; ++i) {
        if (longerBits->bits()[i])
            return false;
    }
    
    for (size_t i = minNumWords; i--;) {
        if (myBits->bits()[i] != otherBits->bits()[i])
            return false;
    }

    return true;
}

bool BitVector::equalsSlowCaseSimple(const BitVector& other) const
{
    // This is really cheesy, but probably good enough for now.
    for (unsigned i = std::max(size(), other.size()); i--;) {
        if (get(i) != other.get(i))
            return false;
    }
    return true;
}

uintptr_t BitVector::hashSlowCase() const
{
    ASSERT(!isInline());
    const OutOfLineBits* bits = outOfLineBits();
    uintptr_t result = 0;
    for (unsigned i = bits->numWords(); i--;)
        result ^= bits->bits()[i];
    return result;
}

void BitVector::dump(PrintStream& out) const
{
    for (size_t i = 0; i < size(); ++i) {
        if (get(i))
            out.printf("1");
        else
            out.printf("-");
    }
}

} // namespace WTF
