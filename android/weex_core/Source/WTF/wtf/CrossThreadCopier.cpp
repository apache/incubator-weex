/*
 * Copyright (C) 2009 Google Inc. All rights reserved.
 * Copyright (C) 2011-2016 Apple Inc. All rights reserved.
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
#include <wtf/CrossThreadCopier.h>

#include <wtf/CrossThreadTask.h>

namespace WTF {

// Test CrossThreadCopier using COMPILE_ASSERT.

// Verify that ThreadSafeRefCounted objects get handled correctly.
class CopierThreadSafeRefCountedTest : public ThreadSafeRefCounted<CopierThreadSafeRefCountedTest> {
};

COMPILE_ASSERT((std::is_same<
                    PassRefPtr<CopierThreadSafeRefCountedTest>,
                    CrossThreadCopier<PassRefPtr<CopierThreadSafeRefCountedTest>>::Type
                >::value),
                PassRefPtrTest);
COMPILE_ASSERT((std::is_same<
                    PassRefPtr<CopierThreadSafeRefCountedTest>,
                    CrossThreadCopier<RefPtr<CopierThreadSafeRefCountedTest>>::Type
                >::value),
                RefPtrTest);
COMPILE_ASSERT((std::is_same<
                    PassRefPtr<CopierThreadSafeRefCountedTest>,
                    CrossThreadCopier<CopierThreadSafeRefCountedTest*>::Type
                >::value),
                RawPointerTest);

// Add specializations for RefCounted types which will let us verify that no other template matches.
template<typename T> struct CrossThreadCopierBase<false, false, RefPtr<T>> {
    typedef int Type;
};

template<typename T> struct CrossThreadCopierBase<false, false, PassRefPtr<T>> {
    typedef int Type;
};

template<typename T> struct CrossThreadCopierBase<false, false, T*> {
    typedef int Type;
};

// Verify that RefCounted objects only match the above templates which expose Type as int.
class CopierRefCountedTest : public RefCounted<CopierRefCountedTest> {
};

static_assert((std::is_same<int, CrossThreadCopier<PassRefPtr<CopierRefCountedTest>>::Type>::value), "CrossThreadCopier specialization improperly applied to PassRefPtr<> of a RefCounted (but not ThreadSafeRefCounted) type");
static_assert((std::is_same<int, CrossThreadCopier<RefPtr<CopierRefCountedTest>>::Type>::value), "CrossThreadCopier specialization improperly applied to RefPtr<> of a RefCounted (but not ThreadSafeRefCounted) type");
static_assert((std::is_same<int, CrossThreadCopier<CopierRefCountedTest*>::Type>::value), "CrossThreadCopier specialization improperly applied to raw pointer of a RefCounted (but not ThreadSafeRefCounted) type");

} // namespace WTF

