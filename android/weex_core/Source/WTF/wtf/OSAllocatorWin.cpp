/*
 * Copyright (C) 2010, 2014 Apple Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "OSAllocator.h"

#if OS(WINDOWS)

#include "windows.h"
#include <wtf/Assertions.h>

namespace WTF {

static inline DWORD protection(bool writable, bool executable)
{
    return executable ?
        (writable ? PAGE_EXECUTE_READWRITE : PAGE_EXECUTE_READ) :
        (writable ? PAGE_READWRITE : PAGE_READONLY);
}

void* OSAllocator::reserveUncommitted(size_t bytes, Usage, bool writable, bool executable, bool)
{
    void* result = VirtualAlloc(0, bytes, MEM_RESERVE, protection(writable, executable));
    if (!result)
        CRASH();
    return result;
}

void* OSAllocator::reserveAndCommit(size_t bytes, Usage, bool writable, bool executable, bool)
{
    void* result = VirtualAlloc(0, bytes, MEM_RESERVE | MEM_COMMIT, protection(writable, executable));
    if (!result)
        CRASH();
    return result;
}

void OSAllocator::commit(void* address, size_t bytes, bool writable, bool executable)
{
    void* result = VirtualAlloc(address, bytes, MEM_COMMIT, protection(writable, executable));
    if (!result)
        CRASH();
}

void OSAllocator::decommit(void* address, size_t bytes)
{
    // According to http://msdn.microsoft.com/en-us/library/aa366892(VS.85).aspx,
    // bytes (i.e. dwSize) being 0 when dwFreeType is MEM_DECOMMIT means that we'll
    // decommit the entire region allocated by VirtualAlloc() instead of decommitting
    // nothing as we would expect. Hence, we should check if bytes is 0 and handle it
    // appropriately before calling VirtualFree().
    // See: https://bugs.webkit.org/show_bug.cgi?id=121972.
    if (!bytes)
        return;
    // Silence warning about using MEM_DECOMMIT instead of MEM_RELEASE:
#pragma warning(suppress: 6250)
    bool result = VirtualFree(address, bytes, MEM_DECOMMIT);
    if (!result)
        CRASH();
}

void OSAllocator::releaseDecommitted(void* address, size_t bytes)
{
    // See comment in OSAllocator::decommit(). Similarly, when bytes is 0, we
    // don't want to release anything. So, don't call VirtualFree() below.
    if (!bytes)
        return;
    // According to http://msdn.microsoft.com/en-us/library/aa366892(VS.85).aspx,
    // dwSize must be 0 if dwFreeType is MEM_RELEASE.
    bool result = VirtualFree(address, 0, MEM_RELEASE);
    if (!result)
        CRASH();
}

void OSAllocator::hintMemoryNotNeededSoon(void*, size_t)
{
}

} // namespace WTF

#endif // OS(WINDOWS)
