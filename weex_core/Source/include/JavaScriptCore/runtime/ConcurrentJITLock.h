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

#include "DeferGC.h"
#include <wtf/Lock.h>
#include <wtf/NoLock.h>
#include <wtf/Optional.h>

namespace JSC {

#if ENABLE(CONCURRENT_JIT)
typedef Lock ConcurrentJITLock;
typedef LockHolder ConcurrentJITLockerImpl;
#else
typedef NoLock ConcurrentJITLock;
typedef NoLockLocker ConcurrentJITLockerImpl;
#endif

class ConcurrentJITLockerBase {
    WTF_MAKE_NONCOPYABLE(ConcurrentJITLockerBase);
public:
    explicit ConcurrentJITLockerBase(ConcurrentJITLock& lockable)
        : m_locker(&lockable)
    {
    }
    explicit ConcurrentJITLockerBase(ConcurrentJITLock* lockable)
        : m_locker(lockable)
    {
    }

    explicit ConcurrentJITLockerBase(NoLockingNecessaryTag)
        : m_locker(NoLockingNecessary)
    {
    }

    ~ConcurrentJITLockerBase()
    {
    }
    
    void unlockEarly()
    {
        m_locker.unlockEarly();
    }

private:
    ConcurrentJITLockerImpl m_locker;
};

class GCSafeConcurrentJITLocker : public ConcurrentJITLockerBase {
public:
    GCSafeConcurrentJITLocker(ConcurrentJITLock& lockable, Heap& heap)
        : ConcurrentJITLockerBase(lockable)
        , m_deferGC(heap)
    {
    }

    GCSafeConcurrentJITLocker(ConcurrentJITLock* lockable, Heap& heap)
        : ConcurrentJITLockerBase(lockable)
        , m_deferGC(heap)
    {
    }

    ~GCSafeConcurrentJITLocker()
    {
        // We have to unlock early due to the destruction order of base
        // vs. derived classes. If we didn't, then we would destroy the 
        // DeferGC object before unlocking the lock which could cause a GC
        // and resulting deadlock.
        unlockEarly();
    }

private:
#if ENABLE(CONCURRENT_JIT)
    DeferGC m_deferGC;
#else
    struct NoDefer {
        NoDefer(Heap& heap) : m_heap(heap) { }
        Heap& m_heap;
    };
    NoDefer m_deferGC;
#endif
};

class ConcurrentJITLocker : public ConcurrentJITLockerBase {
public:
    ConcurrentJITLocker(ConcurrentJITLock& lockable)
        : ConcurrentJITLockerBase(lockable)
#if ENABLE(CONCURRENT_JIT) && !defined(NDEBUG)
        , m_disallowGC(InPlace)
#endif
    {
    }

    ConcurrentJITLocker(ConcurrentJITLock* lockable)
        : ConcurrentJITLockerBase(lockable)
#if ENABLE(CONCURRENT_JIT) && !defined(NDEBUG)
        , m_disallowGC(InPlace)
#endif
    {
    }

    ConcurrentJITLocker(NoLockingNecessaryTag)
        : ConcurrentJITLockerBase(NoLockingNecessary)
#if ENABLE(CONCURRENT_JIT) && !defined(NDEBUG)
        , m_disallowGC(Nullopt)
#endif
    {
    }
    
    ConcurrentJITLocker(int) = delete;

#if ENABLE(CONCURRENT_JIT) && !defined(NDEBUG)
private:
    Optional<DisallowGC> m_disallowGC;
#endif
};

} // namespace JSC
