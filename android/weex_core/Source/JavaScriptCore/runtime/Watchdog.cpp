/*
 * Copyright (C) 2013-2017 Apple Inc. All rights reserved.
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

#include "config.h"
#include "Watchdog.h"

#include "CallFrame.h"
#include <wtf/CurrentTime.h>
#include <wtf/MathExtras.h>

namespace JSC {

const std::chrono::microseconds Watchdog::noTimeLimit = std::chrono::microseconds::max();

static std::chrono::microseconds currentWallClockTime()
{
    auto steadyTimeSinceEpoch = std::chrono::steady_clock::now().time_since_epoch();
    return std::chrono::duration_cast<std::chrono::microseconds>(steadyTimeSinceEpoch);
}

Watchdog::Watchdog(VM* vm)
    : m_vm(vm)
    , m_timeLimit(noTimeLimit)
    , m_cpuDeadline(noTimeLimit)
    , m_wallClockDeadline(noTimeLimit)
    , m_callback(0)
    , m_callbackData1(0)
    , m_callbackData2(0)
    , m_timerQueue(WorkQueue::create("jsc.watchdog.queue", WorkQueue::Type::Serial, WorkQueue::QOS::Utility))
{
}

void Watchdog::setTimeLimit(std::chrono::microseconds limit,
    ShouldTerminateCallback callback, void* data1, void* data2)
{
    ASSERT(m_vm->currentThreadIsHoldingAPILock());

    m_timeLimit = limit;
    m_callback = callback;
    m_callbackData1 = data1;
    m_callbackData2 = data2;

    if (m_hasEnteredVM && hasTimeLimit())
        startTimer(m_timeLimit);
}

bool Watchdog::shouldTerminate(ExecState* exec)
{
    ASSERT(m_vm->currentThreadIsHoldingAPILock());
    // FIXME: Will unindent the following before landing. Leaving indented for now to minimize the code diff.
    {
        if (currentWallClockTime() < m_wallClockDeadline)
            return false; // Just a stale timer firing. Nothing to do.

        // Set m_wallClockDeadline to noTimeLimit here so that we can reject all future
        // spurious wakes.
        m_wallClockDeadline = noTimeLimit;

        auto cpuTime = currentCPUTime();
        if (cpuTime < m_cpuDeadline) {
            auto remainingCPUTime = m_cpuDeadline - cpuTime;
            startTimer(remainingCPUTime);
            return false;
        }
    }

    // Note: we should not be holding the lock while calling the callbacks. The callbacks may
    // call setTimeLimit() which will try to lock as well.

    // If m_callback is not set, then we terminate by default.
    // Else, we let m_callback decide if we should terminate or not.
    bool needsTermination = !m_callback
        || m_callback(exec, m_callbackData1, m_callbackData2);
    if (needsTermination)
        return true;

    // FIXME: Will unindent the following before landing. Leaving indented for now to minimize the code diff.
    {
        // If we get here, then the callback above did not want to terminate execution. As a
        // result, the callback may have done one of the following:
        //   1. cleared the time limit (i.e. watchdog is disabled),
        //   2. set a new time limit via Watchdog::setTimeLimit(), or
        //   3. did nothing (i.e. allow another cycle of the current time limit).
        //
        // In the case of 1, we don't have to do anything.
        // In the case of 2, Watchdog::setTimeLimit() would already have started the timer.
        // In the case of 3, we need to re-start the timer here.

        ASSERT(m_hasEnteredVM);
        bool callbackAlreadyStartedTimer = (m_cpuDeadline != noTimeLimit);
        if (hasTimeLimit() && !callbackAlreadyStartedTimer)
            startTimer(m_timeLimit);
    }
    return false;
}

bool Watchdog::hasTimeLimit()
{
    return (m_timeLimit != noTimeLimit);
}

void Watchdog::enteredVM()
{
    m_hasEnteredVM = true;
    if (hasTimeLimit())
        startTimer(m_timeLimit);
}

void Watchdog::exitedVM()
{
    ASSERT(m_hasEnteredVM);
    stopTimer();
    m_hasEnteredVM = false;
}

void Watchdog::startTimer(std::chrono::microseconds timeLimit)
{
    ASSERT(m_hasEnteredVM);
    ASSERT(m_vm->currentThreadIsHoldingAPILock());
    ASSERT(hasTimeLimit());
    ASSERT(timeLimit <= m_timeLimit);

    m_cpuDeadline = currentCPUTime() + timeLimit;
    auto wallClockTime = currentWallClockTime();
    auto wallClockDeadline = wallClockTime + timeLimit;

    if ((wallClockTime < m_wallClockDeadline)
        && (m_wallClockDeadline <= wallClockDeadline))
        return; // Wait for the current active timer to expire before starting a new one.

    // Else, the current active timer won't fire soon enough. So, start a new timer.
    m_wallClockDeadline = wallClockDeadline;

    // We need to ensure that the Watchdog outlives the timer.
    // For the same reason, the timer may also outlive the VM that the Watchdog operates on.
    // So, we always need to null check m_vm before using it. The VM will notify the Watchdog
    // via willDestroyVM() before it goes away.
    RefPtr<Watchdog> protectedThis = this;
    m_timerQueue->dispatchAfter(Seconds::fromMicroseconds(timeLimit.count()), [this, protectedThis] {
        LockHolder locker(m_lock);
        if (m_vm)
            m_vm->notifyNeedWatchdogCheck();
    });
}

void Watchdog::stopTimer()
{
    ASSERT(m_hasEnteredVM);
    ASSERT(m_vm->currentThreadIsHoldingAPILock());
    m_cpuDeadline = noTimeLimit;
}

void Watchdog::willDestroyVM(VM* vm)
{
    LockHolder locker(m_lock);
    ASSERT_UNUSED(vm, m_vm == vm);
    m_vm = nullptr;
}

} // namespace JSC
