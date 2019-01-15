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
#ifndef WTF_Condition_h
#define WTF_Condition_h

#include <functional>
#include <wtf/CurrentTime.h>
#include <wtf/Noncopyable.h>
#include <wtf/ParkingLot.h>
#include <wtf/TimeWithDynamicClockType.h>

namespace WTF {

// This is a condition variable that is suitable for use with any lock-like object, including
// our own WTF::Lock. It features standard wait()/notifyOne()/notifyAll() methods in addition to
// a variety of wait-with-timeout methods. This includes methods that use WTF's own notion of
// time, like wall-clock time (i.e. currentTime()) and monotonic time (i.e.
// monotonicallyIncreasingTime()). This is a very efficient condition variable. It only requires
// one byte of memory. notifyOne() and notifyAll() require just a load and branch for the fast
// case where no thread is waiting. This condition variable, when used with WTF::Lock, can
// outperform a system condition variable and lock by up to 58x.

// This is a struct without a constructor or destructor so that it can be statically initialized.
// Use Lock in instance variables.
struct ConditionBase {
    // Condition will accept any kind of time and convert it internally, but this typedef tells
    // you what kind of time Condition would be able to use without conversions. However, if you
    // are unlikely to be affected by the cost of conversions, it is better to use MonotonicTime.
    typedef ParkingLot::Time Time;
    
    // Wait on a parking queue while releasing the given lock. It will unlock the lock just before
    // parking, and relock it upon wakeup. Returns true if we woke up due to some call to
    // notifyOne() or notifyAll(). Returns false if we woke up due to a timeout. Note that this form
    // of waitUntil() has some quirks:
    //
    // No spurious wake-up: in order for this to return before the timeout, some notifyOne() or
    // notifyAll() call must have happened. No scenario other than timeout or notify can lead to this
    // method returning. This means, for example, that you can't use pthread cancelation or signals to
    // cause early return.
    //
    // Past timeout: it's possible for waitUntil() to be called with a timeout in the past. In that
    // case, waitUntil() will still release the lock and reacquire it. waitUntil() will always return
    // false in that case. This is subtly different from some pthread_cond_timedwait() implementations,
    // which may not release the lock for past timeout. But, this behavior is consistent with OpenGroup
    // documentation for timedwait().
    template<typename LockType>
    bool waitUntil(LockType& lock, const TimeWithDynamicClockType& timeout)
    {
        bool result;
        if (timeout < timeout.nowWithSameClock()) {
            lock.unlock();
            result = false;
        } else {
            result = ParkingLot::parkConditionally(
                &m_hasWaiters,
                [this] () -> bool {
                    // Let everyone know that we will be waiting. Do this while we hold the queue lock,
                    // to prevent races with notifyOne().
                    m_hasWaiters.store(true);
                    return true;
                },
                [&lock] () { lock.unlock(); },
                timeout).wasUnparked;
        }
        lock.lock();
        return result;
    }

    // Wait until the given predicate is satisfied. Returns true if it is satisfied in the end.
    // May return early due to timeout.
    template<typename LockType, typename Functor>
    bool waitUntil(
        LockType& lock, const TimeWithDynamicClockType& timeout, const Functor& predicate)
    {
        while (!predicate()) {
            if (!waitUntil(lock, timeout))
                return predicate();
        }
        return true;
    }

    // Wait until the given predicate is satisfied. Returns true if it is satisfied in the end.
    // May return early due to timeout.
    template<typename LockType, typename Functor>
    bool waitFor(
        LockType& lock, Seconds relativeTimeout, const Functor& predicate)
    {
        return waitUntil(lock, MonotonicTime::now() + relativeTimeout, predicate);
    }
    
    template<typename LockType>
    bool waitFor(LockType& lock, Seconds relativeTimeout)
    {
        return waitUntil(lock, MonotonicTime::now() + relativeTimeout);
    }

    template<typename LockType>
    void wait(LockType& lock)
    {
        waitUntil(lock, Time::infinity());
    }

    template<typename LockType, typename Functor>
    void wait(LockType& lock, const Functor& predicate)
    {
        while (!predicate())
            wait(lock);
    }

    // Note that this method is extremely fast when nobody is waiting. It is not necessary to try to
    // avoid calling this method. This returns true if someone was actually woken up.
    bool notifyOne()
    {
        if (!m_hasWaiters.load()) {
            // At this exact instant, there is nobody waiting on this condition. The way to visualize
            // this is that if unparkOne() ran to completion without obstructions at this moment, it
            // wouldn't wake anyone up. Hence, we have nothing to do!
            return false;
        }
        
        bool didNotifyThread = false;
        ParkingLot::unparkOne(
            &m_hasWaiters,
            [&] (ParkingLot::UnparkResult result) -> intptr_t {
                if (!result.mayHaveMoreThreads)
                    m_hasWaiters.store(false);
                didNotifyThread = result.didUnparkThread;
                return 0;
            });
        return didNotifyThread;
    }
    
    void notifyAll()
    {
        if (!m_hasWaiters.load()) {
            // See above.
            return;
        }

        // It's totally safe for us to set this to false without any locking, because this thread is
        // guaranteed to then unparkAll() anyway. So, if there is a race with some thread calling
        // wait() just before this store happens, that thread is guaranteed to be awoken by the call to
        // unparkAll(), below.
        m_hasWaiters.store(false);
        
        ParkingLot::unparkAll(&m_hasWaiters);
    }
    
protected:
    Atomic<bool> m_hasWaiters;
};

class Condition : public ConditionBase {
    WTF_MAKE_NONCOPYABLE(Condition);
public:
    Condition()
    {
        m_hasWaiters.store(false);
    }
};

typedef ConditionBase StaticCondition;

} // namespace WTF

using WTF::Condition;
using WTF::StaticCondition;

#endif // WTF_Condition_h

