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

#include "JITCompilationEffort.h"
#include <stddef.h> // for ptrdiff_t
#include <limits>
#include <wtf/Assertions.h>
#include <wtf/Lock.h>
#include <wtf/MetaAllocatorHandle.h>
#include <wtf/MetaAllocator.h>
#include <wtf/PageAllocation.h>

#if OS(IOS)
#include <libkern/OSCacheControl.h>
#endif

#if OS(IOS)
#include <sys/mman.h>
#endif

#if CPU(MIPS) && OS(LINUX)
#include <sys/cachectl.h>
#endif

#if ENABLE(FAST_JIT_PERMISSIONS)
#include <os/thread_self_restrict.h> 
#endif
#define JIT_ALLOCATOR_LARGE_ALLOC_SIZE (pageSize() * 4)

#define EXECUTABLE_POOL_WRITABLE true

namespace JSC {

class VM;

static const unsigned jitAllocationGranule = 32;

typedef WTF::MetaAllocatorHandle ExecutableMemoryHandle;

#if ENABLE(ASSEMBLER)

#if defined(FIXED_EXECUTABLE_MEMORY_POOL_SIZE_IN_MB) && FIXED_EXECUTABLE_MEMORY_POOL_SIZE_IN_MB > 0
static const size_t fixedExecutableMemoryPoolSize = FIXED_EXECUTABLE_MEMORY_POOL_SIZE_IN_MB * 1024 * 1024;
#elif CPU(ARM)
static const size_t fixedExecutableMemoryPoolSize = 16 * 1024 * 1024;
#elif CPU(ARM64)
static const size_t fixedExecutableMemoryPoolSize = 32 * 1024 * 1024;
#elif CPU(X86_64)
static const size_t fixedExecutableMemoryPoolSize = 1024 * 1024 * 1024;
#else
static const size_t fixedExecutableMemoryPoolSize = 32 * 1024 * 1024;
#endif
#if CPU(ARM)
static const double executablePoolReservationFraction = 0.15;
#else
static const double executablePoolReservationFraction = 0.25;
#endif

extern JS_EXPORTDATA uintptr_t startOfFixedExecutableMemoryPool;
extern JS_EXPORTDATA uintptr_t endOfFixedExecutableMemoryPool;

typedef void (*JITWriteSeparateHeapsFunction)(off_t, const void*, size_t);
extern JS_EXPORTDATA JITWriteSeparateHeapsFunction jitWriteSeparateHeapsFunction;

extern JS_EXPORTDATA bool useFastPermisionsJITCopy;

static inline void* performJITMemcpy(void *dst, const void *src, size_t n)
{
    if (reinterpret_cast<uintptr_t>(dst) >= startOfFixedExecutableMemoryPool && reinterpret_cast<uintptr_t>(dst) < endOfFixedExecutableMemoryPool) {
#if ENABLE(FAST_JIT_PERMISSIONS)
        if (useFastPermisionsJITCopy) {
            os_thread_self_restrict_rwx_to_rw();
            memcpy(dst, src, n);
            os_thread_self_restrict_rwx_to_rx();
            return dst;
        }
#endif

        if (jitWriteSeparateHeapsFunction) {
            // Use execute-only write thunk for writes inside the JIT region. This is a variant of
            // memcpy that takes an offset into the JIT region as its destination (first) parameter.
            off_t offset = (off_t)((uintptr_t)dst - startOfFixedExecutableMemoryPool);
            jitWriteSeparateHeapsFunction(offset, src, n);
            return dst;
        }
    }

    // Use regular memcpy for writes outside the JIT region.
    return memcpy(dst, src, n);
}

class ExecutableAllocator {
    enum ProtectionSetting { Writable, Executable };

public:
    ExecutableAllocator(VM&);
    ~ExecutableAllocator();
    
    static void initializeAllocator();

    bool isValid() const;

    static bool underMemoryPressure();
    
    static double memoryPressureMultiplier(size_t addedMemoryUsage);
    
#if ENABLE(META_ALLOCATOR_PROFILE)
    static void dumpProfile();
#else
    static void dumpProfile() { }
#endif

    RefPtr<ExecutableMemoryHandle> allocate(VM&, size_t sizeInBytes, void* ownerUID, JITCompilationEffort);

    bool isValidExecutableMemory(const AbstractLocker&, void* address);

    static size_t committedByteCount();

    Lock& getLock() const;
};

#endif // ENABLE(JIT) && ENABLE(ASSEMBLER)

} // namespace JSC
