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
