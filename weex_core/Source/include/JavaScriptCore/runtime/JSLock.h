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

#include "PlatformThread.h"
#include <mutex>
#include <wtf/Assertions.h>
#include <wtf/Lock.h>
#include <wtf/Noncopyable.h>
#include <wtf/RefPtr.h>
#include <wtf/ThreadSafeRefCounted.h>
#include <wtf/WTFThreadData.h>

namespace JSC {

// To make it safe to use JavaScript on multiple threads, it is
// important to lock before doing anything that allocates a
// JavaScript data structure or that interacts with shared state
// such as the protect count hash table. The simplest way to lock
// is to create a local JSLockHolder object in the scope where the lock 
// must be held and pass it the context that requires protection. 
// The lock is recursive so nesting is ok. The JSLock 
// object also acts as a convenience short-hand for running important
// initialization routines.

// To avoid deadlock, sometimes it is necessary to temporarily
// release the lock. Since it is recursive you actually have to
// release all locks held by your thread. This is safe to do if
// you are executing code that doesn't require the lock, and you
// reacquire the right number of locks at the end. You can do this
// by constructing a locally scoped JSLock::DropAllLocks object. The 
// DropAllLocks object takes care to release the JSLock only if your
// thread acquired it to begin with.

class ExecState;
class VM;

// This class is used to protect the initialization of the legacy single 
// shared VM.
class GlobalJSLock {
    WTF_MAKE_NONCOPYABLE(GlobalJSLock);
public:
    JS_EXPORT_PRIVATE GlobalJSLock();
    JS_EXPORT_PRIVATE ~GlobalJSLock();
private:
    static StaticLock s_sharedInstanceMutex;
};

class JSLockHolder {
public:
    JS_EXPORT_PRIVATE JSLockHolder(VM*);
    JS_EXPORT_PRIVATE JSLockHolder(VM&);
    JS_EXPORT_PRIVATE JSLockHolder(ExecState*);

    JS_EXPORT_PRIVATE ~JSLockHolder();
private:
    void init();

    RefPtr<VM> m_vm;
};

class JSLock : public ThreadSafeRefCounted<JSLock> {
    WTF_MAKE_NONCOPYABLE(JSLock);
public:
    JSLock(VM*);
    JS_EXPORT_PRIVATE ~JSLock();

    JS_EXPORT_PRIVATE void lock();
    JS_EXPORT_PRIVATE void unlock();

    static void lock(ExecState*);
    static void unlock(ExecState*);
    static void lock(VM&);
    static void unlock(VM&);

    VM* vm() { return m_vm; }

    std::optional<PlatformThread> ownerThread() const
    {
        if (m_hasOwnerThread)
            return m_ownerThread;
        return { };
    }
    bool currentThreadIsHoldingLock() { return m_hasOwnerThread && m_ownerThread == currentPlatformThread(); }

    void willDestroyVM(VM*);

    class DropAllLocks {
        WTF_MAKE_NONCOPYABLE(DropAllLocks);
    public:
        JS_EXPORT_PRIVATE DropAllLocks(ExecState*);
        JS_EXPORT_PRIVATE DropAllLocks(VM*);
        JS_EXPORT_PRIVATE DropAllLocks(VM&);
        JS_EXPORT_PRIVATE ~DropAllLocks();

        void setDropDepth(unsigned depth) { m_dropDepth = depth; }
        unsigned dropDepth() const { return m_dropDepth; }

    private:
        intptr_t m_droppedLockCount;
        RefPtr<VM> m_vm;
        unsigned m_dropDepth;
    };

private:
    void lock(intptr_t lockCount);
    void unlock(intptr_t unlockCount);

    void didAcquireLock();
    void willReleaseLock();

    unsigned dropAllLocks(DropAllLocks*);
    void grabAllLocks(DropAllLocks*, unsigned lockCount);

    Lock m_lock;
    // We cannot make m_ownerThread an optional (instead of pairing it with an explicit
    // m_hasOwnerThread) because currentThreadIsHoldingLock() may be called from a
    // different thread, and an optional is vulnerable to races.
    // See https://bugs.webkit.org/show_bug.cgi?id=169042#c6
    bool m_hasOwnerThread { false };
    PlatformThread m_ownerThread;
    intptr_t m_lockCount;
    unsigned m_lockDropDepth;
    bool m_shouldReleaseHeapAccess;
    VM* m_vm;
    AtomicStringTable* m_entryAtomicStringTable; 
};

} // namespace
