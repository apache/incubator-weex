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
#ifndef WTF_Lock_h
#define WTF_Lock_h

#include <wtf/LockAlgorithm.h>
#include <wtf/Locker.h>
#include <wtf/Noncopyable.h>

namespace TestWebKitAPI {
struct LockInspector;
};

namespace WTF {

typedef LockAlgorithm<uint8_t, 1, 2> DefaultLockAlgorithm;

// This is a fully adaptive mutex that only requires 1 byte of storage. It has fast paths that are
// competetive to a spinlock (uncontended locking is inlined and is just a CAS, microcontention is
// handled by spinning and yielding), and a slow path that is competetive to std::mutex (if a lock
// cannot be acquired in a short period of time, the thread is put to sleep until the lock is
// available again). It uses less memory than a std::mutex. This lock guarantees eventual stochastic
// fairness, even in programs that relock the lock immediately after unlocking it. Except when there
// are collisions between this lock and other locks in the ParkingLot, this lock will guarantee that
// at worst one call to unlock() per millisecond will do a direct hand-off to the thread that is at
// the head of the queue. When there are collisions, each collision increases the fair unlock delay
// by one millisecond in the worst case.

// This is a struct without a constructor or destructor so that it can be statically initialized.
// Use Lock in instance variables.
struct LockBase {
    void lock()
    {
        if (UNLIKELY(!DefaultLockAlgorithm::lockFastAssumingZero(m_byte)))
            lockSlow();
    }

    bool tryLock()
    {
        return DefaultLockAlgorithm::tryLock(m_byte);
    }

    // Need this version for std::unique_lock.
    bool try_lock()
    {
        return tryLock();
    }

    // Relinquish the lock. Either one of the threads that were waiting for the lock, or some other
    // thread that happens to be running, will be able to grab the lock. This bit of unfairness is
    // called barging, and we allow it because it maximizes throughput. However, we bound how unfair
    // barging can get by ensuring that every once in a while, when there is a thread waiting on the
    // lock, we hand the lock to that thread directly. Every time unlock() finds a thread waiting,
    // we check if the last time that we did a fair unlock was more than roughly 1ms ago; if so, we
    // unlock fairly. Fairness matters most for long critical sections, and this virtually
    // guarantees that long critical sections always get a fair lock.
    void unlock()
    {
        if (UNLIKELY(!DefaultLockAlgorithm::unlockFastAssumingZero(m_byte)))
            unlockSlow();
    }

    // This is like unlock() but it guarantees that we unlock the lock fairly. For short critical
    // sections, this is much slower than unlock(). For long critical sections, unlock() will learn
    // to be fair anyway. However, if you plan to relock the lock right after unlocking and you want
    // to ensure that some other thread runs in the meantime, this is probably the function you
    // want.
    void unlockFairly()
    {
        if (UNLIKELY(!DefaultLockAlgorithm::unlockFastAssumingZero(m_byte)))
            unlockFairlySlow();
    }
    
    void safepoint()
    {
        if (UNLIKELY(!DefaultLockAlgorithm::safepointFast(m_byte)))
            safepointSlow();
    }

    bool isHeld() const
    {
        return DefaultLockAlgorithm::isLocked(m_byte);
    }

    bool isLocked() const
    {
        return isHeld();
    }

protected:
    friend struct TestWebKitAPI::LockInspector;
    
    static const uint8_t isHeldBit = 1;
    static const uint8_t hasParkedBit = 2;
    
    WTF_EXPORT_PRIVATE void lockSlow();
    WTF_EXPORT_PRIVATE void unlockSlow();
    WTF_EXPORT_PRIVATE void unlockFairlySlow();
    WTF_EXPORT_PRIVATE void safepointSlow();

    // Method used for testing only.
    bool isFullyReset() const
    {
        return !m_byte.load();
    }

    Atomic<uint8_t> m_byte;
};

class Lock : public LockBase {
    WTF_MAKE_NONCOPYABLE(Lock);
    WTF_MAKE_FAST_ALLOCATED;
public:
    Lock()
    {
        m_byte.store(0, std::memory_order_relaxed);
    }
};

typedef LockBase StaticLock;
typedef Locker<LockBase> LockHolder;

} // namespace WTF

using WTF::Lock;
using WTF::LockHolder;
using WTF::StaticLock;

#endif // WTF_Lock_h

