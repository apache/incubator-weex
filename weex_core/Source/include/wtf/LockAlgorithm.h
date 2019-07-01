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

#ifndef WTF_LockAlgorithm_h
#define WTF_LockAlgorithm_h

#include <thread>
#include <wtf/Atomics.h>
#include <wtf/Compiler.h>
#include <wtf/ParkingLot.h>

namespace WTF {

// This is the algorithm used by WTF::Lock. You can use it to project one lock onto any atomic
// field. The limit of one lock is due to the use of the field's address as a key to find the lock's
// queue.

template<typename LockType, LockType isHeldBit, LockType hasParkedBit>
class LockAlgorithm {
    static const bool verbose = false;
    static const LockType mask = isHeldBit | hasParkedBit;

public:
    static bool lockFastAssumingZero(Atomic<LockType>& lock)
    {
        return lock.compareExchangeWeak(0, isHeldBit, std::memory_order_acquire);
    }
    
    static bool lockFast(Atomic<LockType>& lock)
    {
        return lock.transaction(
            [&] (LockType& value) -> bool {
                if (value & isHeldBit)
                    return false;
                value |= isHeldBit;
                return true;
            },
            std::memory_order_acquire,
            TransactionAbortLikelihood::Unlikely);
    }
    
    static void lock(Atomic<LockType>& lock)
    {
        if (UNLIKELY(!lockFast(lock)))
            lockSlow(lock);
    }
    
    static bool tryLock(Atomic<LockType>& lock)
    {
        for (;;) {
            uint8_t currentByteValue = lock.load(std::memory_order_relaxed);
            if (currentByteValue & isHeldBit)
                return false;
            if (lock.compareExchangeWeak(currentByteValue, currentByteValue | isHeldBit, std::memory_order_acquire))
                return true;
        }
    }

    static bool unlockFastAssumingZero(Atomic<LockType>& lock)
    {
        return lock.compareExchangeWeak(isHeldBit, 0, std::memory_order_release);
    }
    
    static bool unlockFast(Atomic<LockType>& lock)
    {
        return lock.transaction(
            [&] (LockType& value) -> bool {
                if ((value & mask) != isHeldBit)
                    return false;
                value &= ~isHeldBit;
                return true;
            },
            std::memory_order_relaxed,
            TransactionAbortLikelihood::Unlikely);
    }
    
    static void unlock(Atomic<LockType>& lock)
    {
        if (UNLIKELY(!unlockFast(lock)))
            unlockSlow(lock, Unfair);
    }
    
    static void unlockFairly(Atomic<LockType>& lock)
    {
        if (UNLIKELY(!unlockFast(lock)))
            unlockSlow(lock, Fair);
    }
    
    static bool safepointFast(const Atomic<LockType>& lock)
    {
        WTF::compilerFence();
        return !(lock.load(std::memory_order_relaxed) & hasParkedBit);
    }
    
    static void safepoint(Atomic<LockType>& lock)
    {
        if (UNLIKELY(!safepointFast(lock)))
            safepointSlow(lock);
    }
    
    static bool isLocked(const Atomic<LockType>& lock)
    {
        return lock.load(std::memory_order_acquire) & isHeldBit;
    }
    
    NEVER_INLINE static void lockSlow(Atomic<LockType>& lock)
    {
        unsigned spinCount = 0;

        // This magic number turns out to be optimal based on past JikesRVM experiments.
        const unsigned spinLimit = 40;
    
        for (;;) {
            uint8_t currentByteValue = lock.load();

            // We allow ourselves to barge in.
            if (!(currentByteValue & isHeldBit)
                && lock.compareExchangeWeak(currentByteValue, currentByteValue | isHeldBit))
                return;

            // If there is nobody parked and we haven't spun too much, we can just try to spin around.
            if (!(currentByteValue & hasParkedBit) && spinCount < spinLimit) {
                spinCount++;
                std::this_thread::yield();
                continue;
            }

            // Need to park. We do this by setting the parked bit first, and then parking. We spin around
            // if the parked bit wasn't set and we failed at setting it.
            if (!(currentByteValue & hasParkedBit)
                && !lock.compareExchangeWeak(currentByteValue, currentByteValue | hasParkedBit))
                continue;

            // We now expect the value to be isHeld|hasParked. So long as that's the case, we can park.
            ParkingLot::ParkResult parkResult =
                ParkingLot::compareAndPark(&lock, currentByteValue | isHeldBit | hasParkedBit);
            if (parkResult.wasUnparked) {
                switch (static_cast<Token>(parkResult.token)) {
                case DirectHandoff:
                    // The lock was never released. It was handed to us directly by the thread that did
                    // unlock(). This means we're done!
                    RELEASE_ASSERT(isLocked(lock));
                    return;
                case BargingOpportunity:
                    // This is the common case. The thread that called unlock() has released the lock,
                    // and we have been woken up so that we may get an opportunity to grab the lock. But
                    // other threads may barge, so the best that we can do is loop around and try again.
                    break;
                }
            }

            // We have awoken, or we never parked because the byte value changed. Either way, we loop
            // around and try again.
        }
    }
    
    enum Fairness {
        Fair,
        Unfair
    };
    NEVER_INLINE static void unlockSlow(Atomic<LockType>& lock, Fairness fairness)
    {
        // We could get here because the weak CAS in unlock() failed spuriously, or because there is
        // someone parked. So, we need a CAS loop: even if right now the lock is just held, it could
        // be held and parked if someone attempts to lock just as we are unlocking.
        for (;;) {
            uint8_t oldByteValue = lock.load();
            RELEASE_ASSERT(
                (oldByteValue & mask) == isHeldBit
                || (oldByteValue & mask) == (isHeldBit | hasParkedBit));
        
            if ((oldByteValue & mask) == isHeldBit) {
                if (lock.compareExchangeWeak(oldByteValue, oldByteValue & ~isHeldBit))
                    return;
                continue;
            }

            // Someone is parked. Unpark exactly one thread. We may hand the lock to that thread
            // directly, or we will unlock the lock at the same time as we unpark to allow for barging.
            // When we unlock, we may leave the parked bit set if there is a chance that there are still
            // other threads parked.
            ASSERT((oldByteValue & mask) == (isHeldBit | hasParkedBit));
            ParkingLot::unparkOne(
                &lock,
                [&] (ParkingLot::UnparkResult result) -> intptr_t {
                    // We are the only ones that can clear either the isHeldBit or the hasParkedBit,
                    // so we should still see both bits set right now.
                    ASSERT((lock.load() & mask) == (isHeldBit | hasParkedBit));
                
                    if (result.didUnparkThread && (fairness == Fair || result.timeToBeFair)) {
                        // We don't unlock anything. Instead, we hand the lock to the thread that was
                        // waiting.
                        return DirectHandoff;
                    }
                    
                    lock.transaction(
                        [&] (LockType& value) -> bool {
                            value &= ~mask;
                            if (result.mayHaveMoreThreads)
                                value |= hasParkedBit;
                            return true;
                        });
                    return BargingOpportunity;
                });
            return;
        }
    }
    
    NEVER_INLINE static void safepointSlow(Atomic<LockType>& lockWord)
    {
        unlockFairly(lockWord);
        lock(lockWord);
    }
    
private:
    enum Token {
        BargingOpportunity,
        DirectHandoff
    };
};

} // namespace WTF

using WTF::LockAlgorithm;

#endif // WTF_LockAlgorithm_h

