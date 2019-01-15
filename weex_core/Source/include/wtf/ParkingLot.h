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
#ifndef WTF_ParkingLot_h
#define WTF_ParkingLot_h

#include <functional>
#include <wtf/Atomics.h>
#include <wtf/ScopedLambda.h>
#include <wtf/Threading.h>
#include <wtf/TimeWithDynamicClockType.h>

namespace WTF {

class ParkingLot {
    ParkingLot() = delete;
    ParkingLot(const ParkingLot&) = delete;

public:
    // ParkingLot will accept any kind of time and convert it internally, but this typedef tells
    // you what kind of time ParkingLot would be able to use without conversions. It's sad that
    // this is WallTime not MonotonicTime, but that's just how OS wait functions work. However,
    // because ParkingLot evaluates whether it should wait by checking if your time has passed
    // using whatever clock you used, specifying timeouts in MonotonicTime is semantically better.
    // For example, if the user sets his computer's clock back during the time that you wanted to
    // wait for one second, and you specified the timeout using the MonotonicTime, then ParkingLot
    // will be smart enough to know that your one second has elapsed.
    typedef WallTime Time;
    
    // Parks the thread in a queue associated with the given address, which cannot be null. The
    // parking only succeeds if the validation function returns true while the queue lock is held.
    //
    // If validation returns false, it will unlock the internal parking queue and then it will
    // return a null ParkResult (wasUnparked = false, token = 0) without doing anything else.
    //
    // If validation returns true, it will enqueue the thread, unlock the parking queue lock, call
    // the beforeSleep function, and then it will sleep so long as the thread continues to be on the
    // queue and the timeout hasn't fired. Finally, this returns wasUnparked = true if we actually
    // got unparked or wasUnparked = false if the timeout was hit. When wasUnparked = true, the
    // token will contain whatever token was returned from the callback to unparkOne(), or 0 if the
    // thread was unparked using unparkAll() or the form of unparkOne() that doesn't take a
    // callback.
    //
    // Note that beforeSleep is called with no locks held, so it's OK to do pretty much anything so
    // long as you don't recursively call parkConditionally(). You can call unparkOne()/unparkAll()
    // though. It's useful to use beforeSleep() to unlock some mutex in the implementation of
    // Condition::wait().
    struct ParkResult {
        bool wasUnparked { false };
        intptr_t token { 0 };
    };
    template<typename ValidationFunctor, typename BeforeSleepFunctor>
    static ParkResult parkConditionally(
        const void* address,
        const ValidationFunctor& validation,
        const BeforeSleepFunctor& beforeSleep,
        const TimeWithDynamicClockType& timeout)
    {
        return parkConditionallyImpl(
            address,
            scopedLambdaRef<bool()>(validation),
            scopedLambdaRef<void()>(beforeSleep),
            timeout);
    }

    // Simple version of parkConditionally() that covers the most common case: you want to park
    // indefinitely so long as the value at the given address hasn't changed.
    template<typename T, typename U>
    static ParkResult compareAndPark(const Atomic<T>* address, U expected)
    {
        return parkConditionally(
            address,
            [address, expected] () -> bool {
                U value = address->load();
                return value == expected;
            },
            [] () { },
            Time::infinity());
    }

    // Unparking status given to you anytime you unparkOne().
    struct UnparkResult {
        // True if some thread was unparked.
        bool didUnparkThread { false };
        // True if there may be more threads on this address. This may be conservatively true.
        bool mayHaveMoreThreads { false };
        // This bit is randomly set to true indicating that it may be profitable to unlock the lock
        // using a fair unlocking protocol. This is most useful when used in conjunction with
        // unparkOne(address, callback).
        bool timeToBeFair { false };
    };

    // Unparks one thread from the queue associated with the given address, which cannot be null.
    // Returns true if there may still be other threads on that queue, or false if there definitely
    // are no more threads on the queue.
    WTF_EXPORT_PRIVATE static UnparkResult unparkOne(const void* address);

    // This is an expert-mode version of unparkOne() that allows for really good thundering herd
    // avoidance and eventual stochastic fairness in adaptive mutexes.
    //
    // Unparks one thread from the queue associated with the given address, and calls the given
    // callback while the address is locked. Reports to the callback whether any thread got
    // unparked, whether there may be any other threads still on the queue, and whether this may be
    // a good time to do fair unlocking. The callback returns an intptr_t token, which is returned
    // to the unparked thread via ParkResult::token.
    //
    // WTF::Lock and WTF::Condition both use this form of unparkOne() because it allows them to use
    // the ParkingLot's internal queue lock to serialize some decision-making. For example, if
    // UnparkResult::mayHaveMoreThreads is false inside the callback, then we know that at that
    // moment nobody can add any threads to the queue because the queue lock is still held. Also,
    // WTF::Lock uses the timeToBeFair and token mechanism to implement eventual fairness.
    template<typename Callback>
    static void unparkOne(const void* address, const Callback& callback)
    {
        unparkOneImpl(address, scopedLambdaRef<intptr_t(UnparkResult)>(callback));
    }
    
    WTF_EXPORT_PRIVATE static unsigned unparkCount(const void* address, unsigned count);

    // Unparks every thread from the queue associated with the given address, which cannot be null.
    WTF_EXPORT_PRIVATE static void unparkAll(const void* address);

    // Locks the parking lot and walks all of the parked threads and the addresses they are waiting
    // on. Threads that are on the same queue are guaranteed to be walked from first to last, but the
    // queues may be randomly interleaved. For example, if the queue for address A1 has T1 and T2 and
    // the queue for address A2 has T3 and T4, then you might see iteration orders like:
    //
    // A1,T1 A1,T2 A2,T3 A2,T4
    // A2,T3 A2,T4 A1,T1 A1,T2
    // A1,T1 A2,T3 A1,T2 A2,T4
    // A1,T1 A2,T3 A2,T4 A1,T2
    //
    // As well as many other possible interleavings that all have T1 before T2 and T3 before T4 but are
    // otherwise unconstrained. This method is useful primarily for debugging. It's also used by unit
    // tests.
    template<typename Func>
    static void forEach(const Func& func)
    {
        forEachImpl(scopedLambdaRef<void(ThreadIdentifier, const void*)>(func));
    }

private:
    WTF_EXPORT_PRIVATE static ParkResult parkConditionallyImpl(
        const void* address,
        const ScopedLambda<bool()>& validation,
        const ScopedLambda<void()>& beforeSleep,
        const TimeWithDynamicClockType& timeout);
    
    WTF_EXPORT_PRIVATE static void unparkOneImpl(
        const void* address, const ScopedLambda<intptr_t(UnparkResult)>& callback);

    WTF_EXPORT_PRIVATE static void forEachImpl(const ScopedLambda<void(ThreadIdentifier, const void*)>&);
};

} // namespace WTF

using WTF::ParkingLot;

#endif // WTF_ParkingLot_h

