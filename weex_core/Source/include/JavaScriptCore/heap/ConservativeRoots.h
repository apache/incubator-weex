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
