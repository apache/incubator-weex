/*
 * Copyright (C) 2016 Konstantin Tokavev <annulen@yandex.ru>
 * Copyright (C) 2016 Yusuke Suzuki <utatane.tea@gmail.com>
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
#include "RunLoop.h"

namespace WTF {

class RunLoop::TimerBase::ScheduledTask : public ThreadSafeRefCounted<ScheduledTask> {
WTF_MAKE_NONCOPYABLE(ScheduledTask);
public:
    static Ref<ScheduledTask> create(Function<void()>&& function, Seconds interval, bool repeating)
    {
        return adoptRef(*new ScheduledTask(WTFMove(function), interval, repeating));
    }

    ScheduledTask(Function<void()>&& function, Seconds interval, bool repeating)
        : m_function(WTFMove(function))
        , m_fireInterval(interval)
        , m_isRepeating(repeating)
    {
        updateReadyTime();
    }

    bool fired()
    {
        if (!isActive())
            return false;

        m_function();

        if (!m_isRepeating)
            return false;

        updateReadyTime();
        return isActive();
    }

    MonotonicTime scheduledTimePoint() const
    {
        return m_scheduledTimePoint;
    }

    void updateReadyTime()
    {
        m_scheduledTimePoint = MonotonicTime::now();
        if (!m_fireInterval)
            return;
        m_scheduledTimePoint += m_fireInterval;
    }

    struct EarliestSchedule {
        bool operator()(const Ref<ScheduledTask>& lhs, const Ref<ScheduledTask>& rhs)
        {
            return lhs->scheduledTimePoint() > rhs->scheduledTimePoint();
        }
    };

    bool isActive() const
    {
        return m_isActive.load();
    }

    void deactivate()
    {
        m_isActive.store(false);
    }

private:
    Function<void ()> m_function;
    MonotonicTime m_scheduledTimePoint;
    Seconds m_fireInterval;
    std::atomic<bool> m_isActive { true };
    bool m_isRepeating;
};

RunLoop::RunLoop()
{
}

RunLoop::~RunLoop()
{
    LockHolder locker(m_loopLock);
    m_shutdown = true;
    m_readyToRun.notifyOne();

    // Here is running main loops. Wait until all the main loops are destroyed.
    if (!m_mainLoops.isEmpty())
        m_stopCondition.wait(m_loopLock);
}

inline bool RunLoop::populateTasks(RunMode runMode, Status& statusOfThisLoop, Deque<Ref<TimerBase::ScheduledTask>>& firedTimers)
{
    LockHolder locker(m_loopLock);

    if (runMode == RunMode::Drain) {
        MonotonicTime sleepUntil = MonotonicTime::infinity();
        if (!m_schedules.isEmpty())
            sleepUntil = m_schedules.first()->scheduledTimePoint();

        m_readyToRun.waitUntil(m_loopLock, sleepUntil, [&] {
            return m_shutdown || m_pendingTasks || statusOfThisLoop == Status::Stopping;
        });
    }

    if (statusOfThisLoop == Status::Stopping || m_shutdown) {
        m_mainLoops.removeLast();
        if (m_mainLoops.isEmpty())
            m_stopCondition.notifyOne();
        return false;
    }
    m_pendingTasks = false;
    if (runMode == RunMode::Iterate)
        statusOfThisLoop = Status::Stopping;

    // Check expired timers.
    MonotonicTime now = MonotonicTime::now();
    while (!m_schedules.isEmpty()) {
        Ref<TimerBase::ScheduledTask> earliest = m_schedules.first().copyRef();
        if (earliest->scheduledTimePoint() > now)
            break;
        std::pop_heap(m_schedules.begin(), m_schedules.end(), TimerBase::ScheduledTask::EarliestSchedule());
        m_schedules.removeLast();
        firedTimers.append(WTFMove(earliest));
    }

    return true;
}

void RunLoop::runImpl(RunMode runMode)
{
    ASSERT(this == &RunLoop::current());

    Status statusOfThisLoop = Status::Clear;
    {
        LockHolder locker(m_loopLock);
        m_mainLoops.append(&statusOfThisLoop);
    }

    Deque<Ref<TimerBase::ScheduledTask>> firedTimers;
    while (true) {
        if (!populateTasks(runMode, statusOfThisLoop, firedTimers))
            return;

        // Dispatch scheduled timers.
        while (!firedTimers.isEmpty()) {
            Ref<TimerBase::ScheduledTask> task = firedTimers.takeFirst();
            if (task->fired()) {
                // Reschedule because the timer requires repeating.
                // Since we will query the timers' time points before sleeping,
                // we do not call wakeUp() here.
                schedule(WTFMove(task));
            }
        }
        performWork();
    }
}

void RunLoop::run()
{
    RunLoop::current().runImpl(RunMode::Drain);
}

void RunLoop::iterate()
{
    RunLoop::current().runImpl(RunMode::Iterate);
}

// RunLoop operations are thread-safe. These operations can be called from outside of the RunLoop's thread.
// For example, WorkQueue::{dispatch, dispatchAfter} call the operations of the WorkQueue thread's RunLoop
// from the caller's thread.

void RunLoop::stop()
{
    LockHolder locker(m_loopLock);
    if (m_mainLoops.isEmpty())
        return;

    Status* status = m_mainLoops.last();
    if (*status != Status::Stopping) {
        *status = Status::Stopping;
        m_readyToRun.notifyOne();
    }
}

void RunLoop::wakeUp(const AbstractLocker&)
{
    m_pendingTasks = true;
    m_readyToRun.notifyOne();
}

void RunLoop::wakeUp()
{
    LockHolder locker(m_loopLock);
    wakeUp(locker);
}

void RunLoop::schedule(const AbstractLocker&, Ref<TimerBase::ScheduledTask>&& task)
{
    m_schedules.append(WTFMove(task));
    std::push_heap(m_schedules.begin(), m_schedules.end(), TimerBase::ScheduledTask::EarliestSchedule());
}

void RunLoop::schedule(Ref<TimerBase::ScheduledTask>&& task)
{
    LockHolder locker(m_loopLock);
    schedule(locker, WTFMove(task));
}

void RunLoop::scheduleAndWakeUp(Ref<TimerBase::ScheduledTask>&& task)
{
    LockHolder locker(m_loopLock);
    schedule(locker, WTFMove(task));
    wakeUp(locker);
}

void RunLoop::dispatchAfter(Seconds delay, Function<void()>&& function)
{
    LockHolder locker(m_loopLock);
    bool repeating = false;
    schedule(locker, TimerBase::ScheduledTask::create(WTFMove(function), delay, repeating));
    wakeUp(locker);
}

// Since RunLoop does not own the registered TimerBase,
// TimerBase and its owner should manage these lifetime.
//
// And more importantly, TimerBase operations are not thread-safe.
// So threads that do not belong to the ScheduledTask's RunLoop
// should not operate the RunLoop::TimerBase.
// This is the same to the RunLoopWin, which is RunLoop for Windows.
RunLoop::TimerBase::TimerBase(RunLoop& runLoop)
    : m_runLoop(runLoop)
    , m_scheduledTask(nullptr)
{
}

RunLoop::TimerBase::~TimerBase()
{
    stop();
}

void RunLoop::TimerBase::start(double interval, bool repeating)
{
    stop();
    m_scheduledTask = ScheduledTask::create([this] {
        fired();
    }, Seconds(interval), repeating);
    m_runLoop.scheduleAndWakeUp(*m_scheduledTask);
}

void RunLoop::TimerBase::stop()
{
    if (m_scheduledTask) {
        m_scheduledTask->deactivate();
        m_scheduledTask = nullptr;
    }
}

bool RunLoop::TimerBase::isActive() const
{
    return m_scheduledTask;
}

} // namespace WTF
