/*
* Copyright (C) 2010, 2015 Apple Inc. All rights reserved.
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
* THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
* THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
* PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
* BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
* INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
* CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
* THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "config.h"
#include "WorkQueue.h"

#include <wtf/MathExtras.h>
#include <wtf/Threading.h>

namespace WTF {

DWORD WorkQueue::workThreadCallback(void* context)
{
    ASSERT_ARG(context, context);

    WorkQueue* queue = static_cast<WorkQueue*>(context);

    if (!queue->tryRegisterAsWorkThread())
        return 0;

    queue->performWorkOnRegisteredWorkThread();
    return 0;
}

void WorkQueue::performWorkOnRegisteredWorkThread()
{
    ASSERT(m_isWorkThreadRegistered);

    m_functionQueueLock.lock();

    while (!m_functionQueue.isEmpty()) {
        Vector<Function<void()>> functionQueue;
        m_functionQueue.swap(functionQueue);

        // Allow more work to be scheduled while we're not using the queue directly.
        m_functionQueueLock.unlock();
        for (auto& function : functionQueue) {
            function();
            deref();
        }
        m_functionQueueLock.lock();
    }

    // One invariant we maintain is that any work scheduled while a work thread is registered will
    // be handled by that work thread. Unregister as the work thread while the queue lock is still
    // held so that no work can be scheduled while we're still registered.
    unregisterAsWorkThread();

    m_functionQueueLock.unlock();
}

void WorkQueue::platformInitialize(const char* name, Type, QOS)
{
    m_isWorkThreadRegistered = 0;
    m_timerQueue = ::CreateTimerQueue();
    ASSERT_WITH_MESSAGE(m_timerQueue, "::CreateTimerQueue failed with error %lu", ::GetLastError());
}

bool WorkQueue::tryRegisterAsWorkThread()
{
    LONG result = ::InterlockedCompareExchange(&m_isWorkThreadRegistered, 1, 0);
    ASSERT(!result || result == 1);
    return !result;
}

void WorkQueue::unregisterAsWorkThread()
{
    LONG result = ::InterlockedCompareExchange(&m_isWorkThreadRegistered, 0, 1);
    ASSERT_UNUSED(result, result == 1);
}

void WorkQueue::platformInvalidate()
{
    // FIXME: We need to ensure that any timer-queue timers that fire after this point don't try to
    // access this WorkQueue <http://webkit.org/b/44690>.
    ::DeleteTimerQueueEx(m_timerQueue, 0);
}

void WorkQueue::dispatch(Function<void()>&& function)
{
    MutexLocker locker(m_functionQueueLock);
    ref();
    m_functionQueue.append(WTFMove(function));

    // Spawn a work thread to perform the work we just added. As an optimization, we avoid
    // spawning the thread if a work thread is already registered. This prevents multiple work
    // threads from being spawned in most cases. (Note that when a work thread has been spawned but
    // hasn't registered itself yet, m_isWorkThreadRegistered will be false and we'll end up
    // spawning a second work thread here. But work thread registration process will ensure that
    // only one thread actually ends up performing work.)
    if (!m_isWorkThreadRegistered)
        ::QueueUserWorkItem(workThreadCallback, this, WT_EXECUTEDEFAULT);
}

struct TimerContext : public ThreadSafeRefCounted<TimerContext> {
    static RefPtr<TimerContext> create() { return adoptRef(new TimerContext); }

    WorkQueue* queue;
    Function<void()> function;
    Mutex timerMutex;
    HANDLE timer;

private:
    TimerContext()
        : queue(nullptr)
        , timer(0)
    {
    }
};

void WorkQueue::timerCallback(void* context, BOOLEAN timerOrWaitFired)
{
    ASSERT_ARG(context, context);
    ASSERT_UNUSED(timerOrWaitFired, timerOrWaitFired);

    // Balanced by leakRef in scheduleWorkAfterDelay.
    RefPtr<TimerContext> timerContext = adoptRef(static_cast<TimerContext*>(context));

    timerContext->queue->dispatch(WTFMove(timerContext->function));

    MutexLocker lock(timerContext->timerMutex);
    ASSERT(timerContext->timer);
    ASSERT(timerContext->queue->m_timerQueue);
    if (!::DeleteTimerQueueTimer(timerContext->queue->m_timerQueue, timerContext->timer, 0)) {
        // Getting ERROR_IO_PENDING here means that the timer will be destroyed once the callback is done executing.
        ASSERT_WITH_MESSAGE(::GetLastError() == ERROR_IO_PENDING, "::DeleteTimerQueueTimer failed with error %lu", ::GetLastError());
    }
}

void WorkQueue::dispatchAfter(Seconds duration, Function<void()>&& function)
{
    ASSERT(m_timerQueue);
    ref();

    RefPtr<TimerContext> context = TimerContext::create();
    context->queue = this;
    context->function = WTFMove(function);

    {
        // The timer callback could fire before ::CreateTimerQueueTimer even returns, so we protect
        // context->timer with a mutex to ensure the timer callback doesn't access it before the
        // timer handle has been stored in it.
        MutexLocker lock(context->timerMutex);

        int64_t milliseconds = duration.milliseconds();

        // From empirical testing, we've seen CreateTimerQueueTimer() sometimes fire up to 5+ ms early.
        // This causes havoc for clients of this code that expect to not be called back until the
        // specified duration has expired. Other folks online have also observed some slop in the
        // firing times of CreateTimerQuqueTimer(). From the data posted at
        // http://omeg.pl/blog/2011/11/on-winapi-timers-and-their-resolution, it appears that the slop
        // can be up to about 10 ms. To ensure that we don't fire the timer early, we'll tack on a
        // slop adjustment to the duration, and we'll use double the worst amount of slop observed
        // so far.
        const int64_t slopAdjustment = 20;
        if (milliseconds) 
            milliseconds += slopAdjustment;

        // Since our timer callback is quick, we can execute in the timer thread itself and avoid
        // an extra thread switch over to a worker thread.
        if (!::CreateTimerQueueTimer(&context->timer, m_timerQueue, timerCallback, context.get(), clampTo<DWORD>(milliseconds), 0, WT_EXECUTEINTIMERTHREAD)) {
            ASSERT_WITH_MESSAGE(false, "::CreateTimerQueueTimer failed with error %lu", ::GetLastError());
            return;
        }
    }

    // The timer callback will handle destroying context.
    context.release().leakRef();
}

} // namespace WTF
