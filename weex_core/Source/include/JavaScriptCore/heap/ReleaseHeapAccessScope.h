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

