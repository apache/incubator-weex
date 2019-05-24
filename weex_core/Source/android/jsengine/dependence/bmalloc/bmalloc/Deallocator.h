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
#ifndef Deallocator_h
#define Deallocator_h

#include "FixedVector.h"
#include <mutex>

namespace bmalloc {

class DebugHeap;
class Heap;
class StaticMutex;

// Per-cache object deallocator.

class Deallocator {
public:
    Deallocator(Heap*);
    ~Deallocator();

    void deallocate(void*);
    void scavenge();
    
    void processObjectLog();
    void processObjectLog(std::lock_guard<StaticMutex>&);

private:
    bool deallocateFastCase(void*);
    void deallocateSlowCase(void*);

    FixedVector<void*, deallocatorLogCapacity> m_objectLog;
    DebugHeap* m_debugHeap;
};

inline bool Deallocator::deallocateFastCase(void* object)
{
    BASSERT(mightBeLarge(nullptr));
    if (mightBeLarge(object))
        return false;

    if (m_objectLog.size() == m_objectLog.capacity())
        return false;

    m_objectLog.push(object);
    return true;
}

inline void Deallocator::deallocate(void* object)
{
    if (!deallocateFastCase(object))
        deallocateSlowCase(object);
}

} // namespace bmalloc

#endif // Deallocator_h
