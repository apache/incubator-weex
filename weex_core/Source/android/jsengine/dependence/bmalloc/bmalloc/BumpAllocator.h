/*
 * Copyright (C) 2014 Apple Inc. All rights reserved.
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
#ifndef BumpAllocator_h
#define BumpAllocator_h

#include "BAssert.h"
#include "BumpRange.h"
#include "ObjectType.h"

namespace bmalloc {

// Helper object for allocating small objects.

class BumpAllocator {
public:
    BumpAllocator();
    void init(size_t);
    
    size_t size() { return m_size; }
    
    bool isNull() { return !m_ptr; }
    void clear();

    bool canAllocate() { return !!m_remaining; }
    void* allocate();

    void refill(const BumpRange&);

private:
    char* m_ptr;
    unsigned m_size;
    unsigned m_remaining;
};

inline BumpAllocator::BumpAllocator()
    : m_ptr()
    , m_size()
    , m_remaining()
{
}

inline void BumpAllocator::init(size_t size)
{
    m_ptr = nullptr;
    m_size = size;
    m_remaining = 0;
}

inline void* BumpAllocator::allocate()
{
    BASSERT(m_remaining);

    --m_remaining;
    char* result = m_ptr;
    m_ptr += m_size;
    return result;
}

inline void BumpAllocator::refill(const BumpRange& bumpRange)
{
    BASSERT(!canAllocate());
    m_ptr = bumpRange.begin;
    m_remaining = bumpRange.objectCount;
    BASSERT(canAllocate());
}

inline void BumpAllocator::clear()
{
    m_ptr = nullptr;
    m_remaining = 0;
}

} // namespace bmalloc

#endif // BumpAllocator_h
