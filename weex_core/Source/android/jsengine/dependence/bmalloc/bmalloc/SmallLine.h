/*
 * Copyright (C) 2014-2016 Apple Inc. All rights reserved.
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
#ifndef SmallLine_h
#define SmallLine_h

#include "BAssert.h"
#include "Mutex.h"
#include "ObjectType.h"
#include <mutex>

namespace bmalloc {

class SmallLine {
public:
    void ref(std::lock_guard<StaticMutex>&, unsigned char = 1);
    bool deref(std::lock_guard<StaticMutex>&);
    unsigned refCount(std::lock_guard<StaticMutex>&) { return m_refCount; }
    
    char* begin();
    char* end();

private:
    unsigned char m_refCount;

static_assert(
    smallLineSize / alignment <= std::numeric_limits<decltype(m_refCount)>::max(),
    "maximum object count must fit in SmallLine::m_refCount");

};

inline void SmallLine::ref(std::lock_guard<StaticMutex>&, unsigned char refCount)
{
    BASSERT(!m_refCount);
    m_refCount = refCount;
}

inline bool SmallLine::deref(std::lock_guard<StaticMutex>&)
{
    BASSERT(m_refCount);
    --m_refCount;
    return !m_refCount;
}

} // namespace bmalloc

#endif // SmallLine_h
