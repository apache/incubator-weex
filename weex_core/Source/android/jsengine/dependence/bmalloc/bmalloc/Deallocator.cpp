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
#include "BAssert.h"
#include "Chunk.h"
#include "Deallocator.h"
#include "DebugHeap.h"
#include "Heap.h"
#include "Inline.h"
#include "Object.h"
#include "PerProcess.h"
#include <algorithm>
#include <cstdlib>
#include <sys/mman.h>

using namespace std;

namespace bmalloc {

Deallocator::Deallocator(Heap* heap)
    : m_debugHeap(heap->debugHeap())
{
    if (m_debugHeap) {
        // Fill the object log in order to disable the fast path.
        while (m_objectLog.size() != m_objectLog.capacity())
            m_objectLog.push(nullptr);
    }
}

Deallocator::~Deallocator()
{
    scavenge();
}
    
void Deallocator::scavenge()
{
    if (m_debugHeap)
        return;

    processObjectLog();
}

void Deallocator::processObjectLog(std::lock_guard<StaticMutex>& lock)
{
    Heap* heap = PerProcess<Heap>::getFastCase();
    
    for (Object object : m_objectLog)
        heap->derefSmallLine(lock, object);

    m_objectLog.clear();
}

void Deallocator::processObjectLog()
{
    std::lock_guard<StaticMutex> lock(PerProcess<Heap>::mutex());
    processObjectLog(lock);
}

void Deallocator::deallocateSlowCase(void* object)
{
    if (m_debugHeap)
        return m_debugHeap->free(object);

    if (!object)
        return;

    std::lock_guard<StaticMutex> lock(PerProcess<Heap>::mutex());
    if (PerProcess<Heap>::getFastCase()->isLarge(lock, object)) {
        PerProcess<Heap>::getFastCase()->deallocateLarge(lock, object);
        return;
    }

    if (m_objectLog.size() == m_objectLog.capacity())
        processObjectLog(lock);

    m_objectLog.push(object);
}

} // namespace bmalloc
