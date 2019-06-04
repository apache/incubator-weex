/*
 * Copyright (C) 2013, 2016 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
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
