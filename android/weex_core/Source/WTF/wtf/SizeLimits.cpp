/*
 * Copyright (C) 2010 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"

#include <type_traits>
#include <utility>
#include <wtf/Assertions.h>
#include <wtf/PassRefPtr.h>
#include <wtf/RefCounted.h>
#include <wtf/RefPtr.h>
#include <wtf/Vector.h>

namespace WTF {

#if !defined(NDEBUG) || ENABLE(SECURITY_ASSERTIONS)
struct SameSizeAsRefCounted {
    int a;
    bool b;
    bool c;
    // The debug version may get bigger.
};
#else
struct SameSizeAsRefCounted {
    int a;
    // Don't add anything here because this should stay small.
};
#endif

static_assert(sizeof(PassRefPtr<RefCounted<int>>) == sizeof(int*), "PassRefPtr should stay small!");
static_assert(sizeof(RefCounted<int>) == sizeof(SameSizeAsRefCounted), "RefCounted should stay small!");
static_assert(sizeof(RefPtr<RefCounted<int>>) == sizeof(int*), "RefPtr should stay small!");

#if !ASAN_ENABLED
template<typename T, unsigned inlineCapacity = 0>
struct SameSizeAsVectorWithInlineCapacity;

template<typename T>
struct SameSizeAsVectorWithInlineCapacity<T, 0> {
    void* bufferPointer;
    unsigned capacity;
    unsigned size;
};

template<typename T, unsigned inlineCapacity>
struct SameSizeAsVectorWithInlineCapacity {
    SameSizeAsVectorWithInlineCapacity<T, 0> baseCapacity;
    typename std::aligned_storage<sizeof(T), std::alignment_of<T>::value>::type inlineBuffer[inlineCapacity];
};

static_assert(sizeof(Vector<int>) == sizeof(SameSizeAsVectorWithInlineCapacity<int>), "Vector should stay small!");
static_assert(sizeof(Vector<int, 1>) == sizeof(SameSizeAsVectorWithInlineCapacity<int, 1>), "Vector should stay small!");
static_assert(sizeof(Vector<int, 2>) == sizeof(SameSizeAsVectorWithInlineCapacity<int, 2>), "Vector should stay small!");
static_assert(sizeof(Vector<int, 3>) == sizeof(SameSizeAsVectorWithInlineCapacity<int, 3>), "Vector should stay small!");
#endif
}
