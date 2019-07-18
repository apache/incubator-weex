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

#if ENABLE(CONCURRENT_JS)
typedef Lock ConcurrentJSLock;
typedef LockHolder ConcurrentJSLockerImpl;
#else
typedef NoLock ConcurrentJSLock;
typedef NoLockLocker ConcurrentJSLockerImpl;
#endif

class ConcurrentJSLockerBase : public AbstractLocker {
    WTF_MAKE_NONCOPYABLE(ConcurrentJSLockerBase);
public:
    explicit ConcurrentJSLockerBase(ConcurrentJSLock& lockable)
        : m_locker(&lockable)
    {
    }
    explicit ConcurrentJSLockerBase(ConcurrentJSLock* lockable)
        : m_locker(lockable)
    {
    }

    explicit ConcurrentJSLockerBase(NoLockingNecessaryTag)
        : m_locker(NoLockingNecessary)
    {
    }

    ~ConcurrentJSLockerBase()
    {
    }
    
    void unlockEarly()
    {
        m_locker.unlockEarly();
    }

private:
    ConcurrentJSLockerImpl m_locker;
};

class GCSafeConcurrentJSLocker : public ConcurrentJSLockerBase {
public:
    GCSafeConcurrentJSLocker(ConcurrentJSLock& lockable, Heap& heap)
        : ConcurrentJSLockerBase(lockable)
        , m_deferGC(heap)
    {
    }

    GCSafeConcurrentJSLocker(ConcurrentJSLock* lockable, Heap& heap)
        : ConcurrentJSLockerBase(lockable)
        , m_deferGC(heap)
    {
    }

    ~GCSafeConcurrentJSLocker()
    {
        // We have to unlock early due to the destruction order of base
        // vs. derived classes. If we didn't, then we would destroy the 
        // DeferGC object before unlocking the lock which could cause a GC
        // and resulting deadlock.
        unlockEarly();
    }

private:
    DeferGC m_deferGC;
};

class ConcurrentJSLocker : public ConcurrentJSLockerBase {
public:
    ConcurrentJSLocker(ConcurrentJSLock& lockable)
        : ConcurrentJSLockerBase(lockable)
#if ENABLE(CONCURRENT_JS) && !defined(NDEBUG)
        , m_disallowGC(std::in_place)
#endif
    {
    }

    ConcurrentJSLocker(ConcurrentJSLock* lockable)
        : ConcurrentJSLockerBase(lockable)
#if ENABLE(CONCURRENT_JS) && !defined(NDEBUG)
        , m_disallowGC(std::in_place)
#endif
    {
    }

    ConcurrentJSLocker(NoLockingNecessaryTag)
        : ConcurrentJSLockerBase(NoLockingNecessary)
#if ENABLE(CONCURRENT_JS) && !defined(NDEBUG)
        , m_disallowGC(std::nullopt)
#endif
    {
    }
    
    ConcurrentJSLocker(int) = delete;

#if ENABLE(CONCURRENT_JS) && !defined(NDEBUG)
private:
    std::optional<DisallowGC> m_disallowGC;
#endif
};

} // namespace JSC
