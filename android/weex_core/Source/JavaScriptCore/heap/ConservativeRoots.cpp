/*
 * Copyright (C) 2011, 2016 Apple Inc. All rights reserved.
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
#include "ConservativeRoots.h"

#include "CodeBlock.h"
#include "CodeBlockSetInlines.h"
#include "HeapInlines.h"
#include "HeapUtil.h"
#include "JITStubRoutineSet.h"
#include "JSCell.h"
#include "JSObject.h"
#include "JSCInlines.h"
#include "MarkedBlockInlines.h"
#include "Structure.h"
#include <wtf/OSAllocator.h>

namespace JSC {

ConservativeRoots::ConservativeRoots(Heap& heap)
    : m_roots(m_inlineRoots)
    , m_size(0)
    , m_capacity(inlineCapacity)
    , m_heap(heap)
{
}

ConservativeRoots::~ConservativeRoots()
{
    if (m_roots != m_inlineRoots)
        OSAllocator::decommitAndRelease(m_roots, m_capacity * sizeof(HeapCell*));
}

void ConservativeRoots::grow()
{
    size_t newCapacity = m_capacity == inlineCapacity ? nonInlineCapacity : m_capacity * 2;
    HeapCell** newRoots = static_cast<HeapCell**>(OSAllocator::reserveAndCommit(newCapacity * sizeof(HeapCell*)));
    memcpy(newRoots, m_roots, m_size * sizeof(HeapCell*));
    if (m_roots != m_inlineRoots)
        OSAllocator::decommitAndRelease(m_roots, m_capacity * sizeof(HeapCell*));
    m_capacity = newCapacity;
    m_roots = newRoots;
}

template<typename MarkHook>
inline void ConservativeRoots::genericAddPointer(void* p, HeapVersion markingVersion, TinyBloomFilter filter, MarkHook& markHook)
{
    markHook.mark(p);

    HeapUtil::findGCObjectPointersForMarking(
        m_heap, markingVersion, filter, p,
        [&] (void* p) {
            if (m_size == m_capacity)
                grow();
            
            m_roots[m_size++] = bitwise_cast<HeapCell*>(p);
        });
}

template<typename MarkHook>
SUPPRESS_ASAN
void ConservativeRoots::genericAddSpan(void* begin, void* end, MarkHook& markHook)
{
    if (begin > end) {
        void* swapTemp = begin;
        begin = end;
        end = swapTemp;
    }

    RELEASE_ASSERT(isPointerAligned(begin));
    RELEASE_ASSERT(isPointerAligned(end));

    TinyBloomFilter filter = m_heap.objectSpace().blocks().filter(); // Make a local copy of filter to show the compiler it won't alias, and can be register-allocated.
    HeapVersion markingVersion = m_heap.objectSpace().markingVersion();
    for (char** it = static_cast<char**>(begin); it != static_cast<char**>(end); ++it)
        genericAddPointer(*it, markingVersion, filter, markHook);
}

class DummyMarkHook {
public:
    void mark(void*) { }
};

void ConservativeRoots::add(void* begin, void* end)
{
    DummyMarkHook dummy;
    genericAddSpan(begin, end, dummy);
}

void ConservativeRoots::add(void* begin, void* end, JITStubRoutineSet& jitStubRoutines)
{
    genericAddSpan(begin, end, jitStubRoutines);
}

class CompositeMarkHook {
public:
    CompositeMarkHook(JITStubRoutineSet& stubRoutines, CodeBlockSet& codeBlocks, const AbstractLocker& locker)
        : m_stubRoutines(stubRoutines)
        , m_codeBlocks(codeBlocks)
        , m_codeBlocksLocker(locker)
    {
    }
    
    void mark(void* address)
    {
        m_stubRoutines.mark(address);
        m_codeBlocks.mark(m_codeBlocksLocker, address);
    }

private:
    JITStubRoutineSet& m_stubRoutines;
    CodeBlockSet& m_codeBlocks;
    const AbstractLocker& m_codeBlocksLocker;
};

void ConservativeRoots::add(
    void* begin, void* end, JITStubRoutineSet& jitStubRoutines, CodeBlockSet& codeBlocks)
{
    LockHolder locker(codeBlocks.getLock());
    CompositeMarkHook markHook(jitStubRoutines, codeBlocks, locker);
    genericAddSpan(begin, end, markHook);
}

} // namespace JSC
