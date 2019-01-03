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

#include <atomic>
#include <wtf/FastMalloc.h>
#include <wtf/Noncopyable.h>

namespace WTF {

class ThreadSafeRefCountedBase {
    WTF_MAKE_NONCOPYABLE(ThreadSafeRefCountedBase);
    WTF_MAKE_FAST_ALLOCATED;
public:
    ThreadSafeRefCountedBase() = default;

    void ref() const
    {
        ++m_refCount;
    }

    bool hasOneRef() const
    {
        return refCount() == 1;
    }

    unsigned refCount() const
    {
        return m_refCount;
    }

protected:
    // Returns whether the pointer should be freed or not.
    bool derefBase() const
    {
        return !--m_refCount;
    }

private:
    mutable std::atomic<unsigned> m_refCount { 1 };
};

template<class T> class ThreadSafeRefCounted : public ThreadSafeRefCountedBase {
public:
    void deref() const
    {
        if (derefBase())
            delete static_cast<const T*>(this);
    }

protected:
    ThreadSafeRefCounted() = default;
};

} // namespace WTF

using WTF::ThreadSafeRefCounted;
