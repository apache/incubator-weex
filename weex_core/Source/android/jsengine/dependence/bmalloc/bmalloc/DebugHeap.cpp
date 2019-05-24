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
#include "DebugHeap.h"
#include "BAssert.h"
#include "BPlatform.h"
#include <cstdlib>
#include <thread>

namespace bmalloc {
    
#if BOS(DARWIN)

DebugHeap::DebugHeap(std::lock_guard<StaticMutex>&)
    : m_zone(malloc_create_zone(0, 0))
{
    malloc_set_zone_name(m_zone, "WebKit Using System Malloc");
}

void* DebugHeap::malloc(size_t size)
{
    void* result = malloc_zone_malloc(m_zone, size);
    if (!result)
        BCRASH();
    return result;
}

void* DebugHeap::memalign(size_t alignment, size_t size, bool crashOnFailure)
{
    void* result = malloc_zone_memalign(m_zone, alignment, size);
    if (!result && crashOnFailure)
        BCRASH();
    return result;
}

void* DebugHeap::realloc(void* object, size_t size)
{
    void* result = malloc_zone_realloc(m_zone, object, size);
    if (!result)
        BCRASH();
    return result;
}

void DebugHeap::free(void* object)
{
    malloc_zone_free(m_zone, object);
}

#else

DebugHeap::DebugHeap(std::lock_guard<StaticMutex>&)
{
}

void* DebugHeap::malloc(size_t size)
{
    void* result = ::malloc(size);
    if (!result)
        BCRASH();
    return result;
}

void* DebugHeap::memalign(size_t alignment, size_t size, bool crashOnFailure)
{
    void* result;
    if (posix_memalign(&result, alignment, size)) {
        if (crashOnFailure)
            BCRASH();
        return nullptr;
    }
    return result;
}

void* DebugHeap::realloc(void* object, size_t size)
{
    void* result = ::realloc(object, size);
    if (!result)
        BCRASH();
    return result;
}

void DebugHeap::free(void* object)
{
    ::free(object);
}
    
#endif

} // namespace bmalloc
