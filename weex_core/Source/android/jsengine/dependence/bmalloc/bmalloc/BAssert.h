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

#include "BPlatform.h"
#include "Logging.h"

#if BUSE(OS_LOG)
#include <os/log.h>
#endif

#if defined(NDEBUG) && BOS(DARWIN)

#if BCPU(X86_64) || BCPU(X86)
#define BBreakpointTrap()  __asm__ volatile ("int3")
#elif BCPU(ARM_THUMB2)
#define BBreakpointTrap()  __asm__ volatile ("bkpt #0")
#elif BCPU(ARM64)
#define BBreakpointTrap()  __asm__ volatile ("brk #0")
#else
#error "Unsupported CPU".
#endif

// Crash with a SIGTRAP i.e EXC_BREAKPOINT.
// We are not using __builtin_trap because it is only guaranteed to abort, but not necessarily
// trigger a SIGTRAP. Instead, we use inline asm to ensure that we trigger the SIGTRAP.
#define BCRASH() do { \
        BBreakpointTrap(); \
        __builtin_unreachable(); \
    } while (false)

#else // not defined(NDEBUG) && BOS(DARWIN)

#define BCRASH() do { \
    *(int*)0xbbadbeef = 0; \
} while (0);

#endif // defined(NDEBUG) && BOS(DARWIN)

#define BASSERT_IMPL(x) do { \
    if (!(x)) \
        BCRASH(); \
} while (0);

#define RELEASE_BASSERT(x) BASSERT_IMPL(x)

#if BUSE(OS_LOG)
#define BMALLOC_LOGGING_PREFIX "bmalloc: "
#define BLOG_ERROR(format, ...) os_log_error(OS_LOG_DEFAULT, BMALLOC_LOGGING_PREFIX format, __VA_ARGS__)
#else
#define BLOG_ERROR(format, ...) bmalloc::reportAssertionFailureWithMessage(__FILE__, __LINE__, __PRETTY_FUNCTION__, format, __VA_ARGS__)
#endif

#if defined(NDEBUG)
#define RELEASE_BASSERT_WITH_MESSAGE(x, format, ...) BASSERT_IMPL(x)
#else
#define RELEASE_BASSERT_WITH_MESSAGE(x, format, ...) do { \
    if (!(x)) { \
        BLOG_ERROR("ASSERTION FAILED: " #x " :: " format, ##__VA_ARGS__); \
        BCRASH(); \
    } \
} while (0);
#endif

#define UNUSED(x) ((void)x)

// ===== Release build =====

#if defined(NDEBUG)

#define BASSERT(x)

#define IF_DEBUG(x)

#endif // defined(NDEBUG)


// ===== Debug build =====

#if !defined(NDEBUG)

#define BASSERT(x) BASSERT_IMPL(x)

#define IF_DEBUG(x) (x)

#endif // !defined(NDEBUG)
