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

#if HAVE(FAST_TLS)

#include <pthread.h>
#include <System/pthread_machdep.h>

namespace WTF {

// __PTK_FRAMEWORK_JAVASCRIPTCORE_KEY0 is taken by bmalloc, so WTF's KEY0 maps to the
// system's KEY1.
#define WTF_FAST_TLS_KEY0 __PTK_FRAMEWORK_JAVASCRIPTCORE_KEY1
#define WTF_FAST_TLS_KEY1 __PTK_FRAMEWORK_JAVASCRIPTCORE_KEY2
#define WTF_FAST_TLS_KEY2 __PTK_FRAMEWORK_JAVASCRIPTCORE_KEY3
#define WTF_FAST_TLS_KEY3 __PTK_FRAMEWORK_JAVASCRIPTCORE_KEY4

// NOTE: We should manage our use of these keys here. If you want to use a key for something,
// put a #define in here to give your key a symbolic name. This ensures that we don't
// accidentally use the same key for more than one thing.

#define WTF_THREAD_DATA_KEY WTF_FAST_TLS_KEY0
#define WTF_TESTING_KEY WTF_FAST_TLS_KEY3

#if ENABLE(FAST_TLS_JIT)
// Below is the code that the JIT will emit.

#if CPU(X86_64)
inline uintptr_t loadFastTLS(unsigned offset)
{
    uintptr_t result;
    asm volatile(
        "movq %%gs:%1, %0"
        : "=r"(result)
        : "r"(offset)
        : "memory");
    return result;
}
#elif CPU(ARM64)
inline uintptr_t loadFastTLS(unsigned passedOffset)
{
    uintptr_t result;
    uintptr_t offset = passedOffset;
    asm volatile(
        "mrs %0, TPIDRRO_EL0\n\t"
        "and %0, %0, #0xfffffffffffffff8\n\t"
        "ldr %0, [%0, %1]"
        : "=r"(result)
        : "r"(offset)
        : "memory");
    return result;
}
#else
#error "Bad architecture"
#endif
#endif // ENABLE(FAST_TLS_JIT)

inline unsigned fastTLSOffsetForKey(unsigned long slot)
{
    return slot * sizeof(void*);
}

} // namespace WTF

using WTF::fastTLSOffsetForKey;

#if ENABLE(FAST_TLS_JIT)
using WTF::loadFastTLS;
#endif

#endif // HAVE(FAST_TLS)

