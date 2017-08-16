/*
 * Copyright (C) 2015-2016 Apple Inc. All rights reserved.
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

