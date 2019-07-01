/*
 * Copyright (C) 2009, 2016 Apple Inc. All rights reserved.
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

#pragma once

#include "Heap.h"

namespace JSC {

class CodeBlockSet;
class HeapCell;
class JITStubRoutineSet;

class ConservativeRoots {
public:
    ConservativeRoots(Heap&);
    ~ConservativeRoots();

    void add(void* begin, void* end);
    void add(void* begin, void* end, JITStubRoutineSet&);
    void add(void* begin, void* end, JITStubRoutineSet&, CodeBlockSet&);
    
    size_t size();
    HeapCell** roots();

private:
    static const size_t inlineCapacity = 128;
    static const size_t nonInlineCapacity = 8192 / sizeof(HeapCell*);
    
    template<typename MarkHook>
    void genericAddPointer(void*, HeapVersion, TinyBloomFilter, MarkHook&);

    template<typename MarkHook>
    void genericAddSpan(void*, void* end, MarkHook&);
    
    void grow();

    HeapCell** m_roots;
    size_t m_size;
    size_t m_capacity;
    Heap& m_heap;
    HeapCell* m_inlineRoots[inlineCapacity];
};

inline size_t ConservativeRoots::size()
{
    return m_size;
}

inline HeapCell** ConservativeRoots::roots()
{
    return m_roots;
}

} // namespace JSC
