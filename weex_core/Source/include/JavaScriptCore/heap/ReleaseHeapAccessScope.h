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

#pragma once

#include "Heap.h"

namespace JSC {

// Almost all of the VM's code runs with "heap access". This means that the GC thread believes that
// the VM is messing with the heap in a way that would be unsafe for certain phases of the collector,
// like the weak reference fixpoint, stack scanning, and changing barrier modes. However, many long
// running operations inside the VM don't require heap access. For example, memcpying a typed array
// if a reference to it is on the stack is totally fine without heap access. Blocking on a futex is
// also fine without heap access. Releasing heap access for long-running code (in the case of futex
// wait, possibly infinitely long-running) ensures that the GC can finish a collection cycle while
// you are waiting.
class ReleaseHeapAccessScope {
public:
    ReleaseHeapAccessScope(Heap& heap)
        : m_heap(heap)
    {
        m_heap.releaseAccess();
    }
    
    ~ReleaseHeapAccessScope()
    {
        m_heap.acquireAccess();
    }

private:
    Heap& m_heap;
};

} // namespace JSC

