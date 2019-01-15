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
