/*
 * Copyright (C) 2007, 2008, 2015-2016 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 * 3.  Neither the name of Apple Inc. ("Apple") nor the names of
 *     its contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "MainThread.h"

#include "CurrentTime.h"
#include "Deque.h"
#include "StdLibExtras.h"
#include "Threading.h"
#include <mutex>
#include <wtf/Lock.h>
#include <wtf/NeverDestroyed.h>
#include <wtf/ThreadSpecific.h>

namespace WTF {

static bool callbacksPaused; // This global variable is only accessed from main thread.
#if !OS(DARWIN) && !PLATFORM(GTK)
static ThreadIdentifier mainThreadIdentifier;
#endif

static StaticLock mainThreadFunctionQueueMutex;

static Deque<Function<void ()>>& functionQueue()
{
    static NeverDestroyed<Deque<Function<void ()>>> functionQueue;
    return functionQueue;
}

#if OS(DARWIN) || PLATFORM(GTK)
static pthread_once_t initializeMainThreadKeyOnce = PTHREAD_ONCE_INIT;

static void initializeMainThreadOnce()
{
    initializeThreading();
    initializeMainThreadPlatform();
    initializeGCThreads();
}

void initializeMainThread()
{
    pthread_once(&initializeMainThreadKeyOnce, initializeMainThreadOnce);
}

#if !USE(WEB_THREAD) && !PLATFORM(GTK)
static void initializeMainThreadToProcessMainThreadOnce()
{
    initializeThreading();
    initializeMainThreadToProcessMainThreadPlatform();
    initializeGCThreads();
}

void initializeMainThreadToProcessMainThread()
{
    pthread_once(&initializeMainThreadKeyOnce, initializeMainThreadToProcessMainThreadOnce);
}
#elif !PLATFORM(GTK)
static pthread_once_t initializeWebThreadKeyOnce = PTHREAD_ONCE_INIT;

static void initializeWebThreadOnce()
{
    initializeWebThreadPlatform();
}

void initializeWebThread()
{
    pthread_once(&initializeWebThreadKeyOnce, initializeWebThreadOnce);
}
#endif // !USE(WEB_THREAD)

#else
void initializeMainThread()
{
    static bool initializedMainThread;
    if (initializedMainThread)
        return;
    initializedMainThread = true;

    initializeThreading();
    mainThreadIdentifier = currentThread();

    initializeMainThreadPlatform();
    initializeGCThreads();
}
#endif

// 0.1 sec delays in UI is approximate threshold when they become noticeable. Have a limit that's half of that.
static const auto maxRunLoopSuspensionTime = 50_ms;

void dispatchFunctionsFromMainThread()
{
    ASSERT(isMainThread());

    if (callbacksPaused)
        return;

    auto startTime = MonotonicTime::now();

    Function<void ()> function;

    while (true) {
        {
            std::lock_guard<StaticLock> lock(mainThreadFunctionQueueMutex);
            if (!functionQueue().size())
                break;

            function = functionQueue().takeFirst();
        }

        function();

        // Clearing the function can have side effects, so do so outside of the lock above.
        function = nullptr;

        // If we are running accumulated functions for too long so UI may become unresponsive, we need to
        // yield so the user input can be processed. Otherwise user may not be able to even close the window.
        // This code has effect only in case the scheduleDispatchFunctionsOnMainThread() is implemented in a way that
        // allows input events to be processed before we are back here.
        if (MonotonicTime::now() - startTime > maxRunLoopSuspensionTime) {
            scheduleDispatchFunctionsOnMainThread();
            break;
        }
    }
}

void callOnMainThread(Function<void ()>&& function)
{
    ASSERT(function);

    bool needToSchedule = false;

    {
        std::lock_guard<StaticLock> lock(mainThreadFunctionQueueMutex);
        needToSchedule = functionQueue().size() == 0;
        functionQueue().append(WTFMove(function));
    }

    if (needToSchedule)
        scheduleDispatchFunctionsOnMainThread();
}

void setMainThreadCallbacksPaused(bool paused)
{
    ASSERT(isMainThread());

    if (callbacksPaused == paused)
        return;

    callbacksPaused = paused;

    if (!callbacksPaused)
        scheduleDispatchFunctionsOnMainThread();
}

#if !OS(DARWIN) && !USE(GLIB_EVENT_LOOP)
bool isMainThread()
{
    return currentThread() == mainThreadIdentifier;
}
#endif

#if !USE(WEB_THREAD)
bool canAccessThreadLocalDataForThread(ThreadIdentifier threadId)
{
    return threadId == currentThread();
}
#endif

static ThreadSpecific<std::optional<GCThreadType>, CanBeGCThread::True>* isGCThread;

void initializeGCThreads()
{
    static std::once_flag flag;
    std::call_once(
        flag,
        [] {
            isGCThread = new ThreadSpecific<std::optional<GCThreadType>, CanBeGCThread::True>();
        });
}

void registerGCThread(GCThreadType type)
{
    if (!isGCThread) {
        // This happens if we're running in a process that doesn't care about
        // MainThread.
        return;
    }

    **isGCThread = type;
}

bool isMainThreadOrGCThread()
{
    if (mayBeGCThread())
        return true;

    return isMainThread();
}

std::optional<GCThreadType> mayBeGCThread()
{
    if (!isGCThread)
        return std::nullopt;
    if (!isGCThread->isSet())
        return std::nullopt;
    return **isGCThread;
}

} // namespace WTF
