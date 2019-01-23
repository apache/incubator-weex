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
