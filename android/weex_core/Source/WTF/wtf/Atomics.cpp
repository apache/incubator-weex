/*
 * Copyright (C) 2007, 2008, 2010, 2012, 2015 Apple Inc. All rights reserved.
 * Copyright (C) 2007 Justin Haygood (jhaygood@reaktix.com)
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "Atomics.h"

// Some architectures, like MIPS32, don't have GCC implementation for builtin __sync_* functions
// with 64 bits variable size. Official GCC answer for the problem: If a target doesn't support
// atomic operations on certain variable sizes, you are out of luck with atomicity in that case
// (http://gcc.gnu.org/bugzilla/show_bug.cgi?id=56296). GCC >= 4.8 will support __atomic_* builtin
// functions for this purpose for all the GCC targets, but for current compilers we have to include
// our own implementation.
#if COMPILER(GCC_OR_CLANG) && !defined(__GCC_HAVE_SYNC_COMPARE_AND_SWAP_8) && USE(PTHREADS)

#include "ThreadingPrimitives.h"

namespace WTF {

static const size_t kSwapLockCount = 32;
static Mutex s_swapLocks[kSwapLockCount];

static inline Mutex& getSwapLock(const volatile int64_t* addr)
{
    return s_swapLocks[(reinterpret_cast<intptr_t>(addr) >> 3U) % kSwapLockCount];
}

static int64_t atomicStep(int64_t volatile* addend, int64_t step)
{
    Mutex& mutex = getSwapLock(addend);

    mutex.lock();
    int64_t value = *addend + step;
    *addend = value;
    mutex.unlock();

    return value;
}

extern "C" {

int64_t __sync_add_and_fetch_8(int64_t volatile* addend, int64_t value)
{
    return atomicStep(addend, value);
}

int64_t __sync_sub_and_fetch_8(int64_t volatile* addend, int64_t value)
{
    return atomicStep(addend, -value);
}

} // extern "C"

} // namespace WTF

#endif
