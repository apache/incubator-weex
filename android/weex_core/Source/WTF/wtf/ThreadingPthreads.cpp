/*
 * Copyright (C) 2007, 2009, 2015 Apple Inc. All rights reserved.
 * Copyright (C) 2007 Justin Haygood (jhaygood@reaktix.com)
 * Copyright (C) 2011 Research In Motion Limited. All rights reserved.
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
#include "Threading.h"

#if USE(PTHREADS)

#include "CurrentTime.h"
#include "DateMath.h"
#include "dtoa.h"
#include "dtoa/cached-powers.h"
#include "HashMap.h"
#include "RandomNumberSeed.h"
#include "StdLibExtras.h"
#include "ThreadFunctionInvocation.h"
#include "ThreadIdentifierDataPthreads.h"
#include "ThreadSpecific.h"
#include <wtf/DataLog.h>
#include <wtf/NeverDestroyed.h>
#include <wtf/RawPointer.h>
#include <wtf/WTFThreadData.h>
#include <errno.h>

#if !COMPILER(MSVC)
#include <limits.h>
#include <sched.h>
#include <sys/time.h>
#endif

#if OS(LINUX)
#include <sys/prctl.h>
#endif

namespace WTF {

class PthreadState {
    WTF_MAKE_FAST_ALLOCATED;
public:
    enum JoinableState {
        Joinable, // The default thread state. The thread can be joined on.

        Joined, // Somebody waited on this thread to exit and this thread finally exited. This state is here because there can be a 
                // period of time between when the thread exits (which causes pthread_join to return and the remainder of waitOnThreadCompletion to run) 
                // and when threadDidExit is called. We need threadDidExit to take charge and delete the thread data since there's 
                // nobody else to pick up the slack in this case (since waitOnThreadCompletion has already returned).

        Detached // The thread has been detached and can no longer be joined on. At this point, the thread must take care of cleaning up after itself.
    };

    // Currently all threads created by WTF start out as joinable. 
    PthreadState(pthread_t handle)
        : m_joinableState(Joinable)
        , m_didExit(false)
        , m_pthreadHandle(handle)
    {
    }

    JoinableState joinableState() { return m_joinableState; }
    pthread_t pthreadHandle() { return m_pthreadHandle; }
    void didBecomeDetached() { m_joinableState = Detached; }
    void didExit() { m_didExit = true; }
    void didJoin() { m_joinableState = Joined; }
    bool hasExited() { return m_didExit; }

private:
    JoinableState m_joinableState;
    bool m_didExit;
    pthread_t m_pthreadHandle;
};

typedef HashMap<ThreadIdentifier, std::unique_ptr<PthreadState>> ThreadMap;

void unsafeThreadWasDetached(ThreadIdentifier);
void threadDidExit(ThreadIdentifier);
void threadWasJoined(ThreadIdentifier);

static Mutex& threadMapMutex()
{
    static NeverDestroyed<Mutex> mutex;
    return mutex;
}

void initializeThreading()
{
    static bool isInitialized;

    if (isInitialized)
        return;

    isInitialized = true;

    WTF::double_conversion::initialize();
    // StringImpl::empty() does not construct its static string in a threadsafe fashion,
    // so ensure it has been initialized from here.
    StringImpl::empty();
    threadMapMutex();
    initializeRandomNumberGenerator();
    ThreadIdentifierData::initializeOnce();
    wtfThreadData();
    initializeDates();
}

static ThreadMap& threadMap()
{
    static NeverDestroyed<ThreadMap> map;
    return map;
}

static ThreadIdentifier identifierByPthreadHandle(const pthread_t& pthreadHandle)
{
    MutexLocker locker(threadMapMutex());

    ThreadMap::iterator i = threadMap().begin();
    for (; i != threadMap().end(); ++i) {
        if (pthread_equal(i->value->pthreadHandle(), pthreadHandle) && !i->value->hasExited())
            return i->key;
    }

    return 0;
}

static ThreadIdentifier establishIdentifierForPthreadHandle(const pthread_t& pthreadHandle)
{
    // ASSERT(!identifierByPthreadHandle(pthreadHandle));
    MutexLocker locker(threadMapMutex());
    static ThreadIdentifier identifierCount = 1;
    threadMap().add(identifierCount, std::make_unique<PthreadState>(pthreadHandle));
    return identifierCount++;
}

static pthread_t pthreadHandleForIdentifierWithLockAlreadyHeld(ThreadIdentifier id)
{
    return threadMap().get(id)->pthreadHandle();
}

static void* wtfThreadEntryPoint(void* param)
{
    // Balanced by .leakPtr() in createThreadInternal.
    auto invocation = std::unique_ptr<ThreadFunctionInvocation>(static_cast<ThreadFunctionInvocation*>(param));
    invocation->function(invocation->data);
    return nullptr;
}

ThreadIdentifier createThreadInternal(ThreadFunction entryPoint, void* data, const char*)
{
    auto invocation = std::make_unique<ThreadFunctionInvocation>(entryPoint, data);
    pthread_t threadHandle;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
#if HAVE(QOS_CLASSES)
    pthread_attr_set_qos_class_np(&attr, adjustedQOSClass(QOS_CLASS_USER_INITIATED), 0);
#endif
    int error = pthread_create(&threadHandle, &attr, wtfThreadEntryPoint, invocation.get());
    pthread_attr_destroy(&attr);
    if (error) {
        LOG_ERROR("Failed to create pthread at entry point %p with data %p", wtfThreadEntryPoint, invocation.get());
        return 0;
    }

    // Balanced by std::unique_ptr constructor in wtfThreadEntryPoint.
    ThreadFunctionInvocation* leakedInvocation = invocation.release();
    UNUSED_PARAM(leakedInvocation);

    return establishIdentifierForPthreadHandle(threadHandle);
}

void initializeCurrentThreadInternal(const char* threadName)
{
#if HAVE(PTHREAD_SETNAME_NP)
    pthread_setname_np(normalizeThreadName(threadName));
#elif OS(LINUX)
    prctl(PR_SET_NAME, normalizeThreadName(threadName));
#else
    UNUSED_PARAM(threadName);
#endif

    ThreadIdentifier id = identifierByPthreadHandle(pthread_self());
    ASSERT(id);
    ThreadIdentifierData::initialize(id);
}
    
void changeThreadPriority(ThreadIdentifier threadID, int delta)
{
    pthread_t pthreadHandle;
    ASSERT(threadID);

    {
        MutexLocker locker(threadMapMutex());
        pthreadHandle = pthreadHandleForIdentifierWithLockAlreadyHeld(threadID);
        ASSERT(pthreadHandle);
    }

    int policy;
    struct sched_param param;

    if (pthread_getschedparam(pthreadHandle, &policy, &param))
        return;

    param.sched_priority += delta;

    pthread_setschedparam(pthreadHandle, policy, &param);
}

int waitForThreadCompletion(ThreadIdentifier threadID)
{
    pthread_t pthreadHandle;
    ASSERT(threadID);

    {
        // We don't want to lock across the call to join, since that can block our thread and cause deadlock.
        MutexLocker locker(threadMapMutex());
        pthreadHandle = pthreadHandleForIdentifierWithLockAlreadyHeld(threadID);
        ASSERT(pthreadHandle);
    }

    int joinResult = pthread_join(pthreadHandle, 0);

    if (joinResult == EDEADLK)
        LOG_ERROR("ThreadIdentifier %u was found to be deadlocked trying to quit", threadID);
    else if (joinResult)
        LOG_ERROR("ThreadIdentifier %u was unable to be joined.\n", threadID);

    MutexLocker locker(threadMapMutex());
    PthreadState* state = threadMap().get(threadID);
    ASSERT(state);
    ASSERT(state->joinableState() == PthreadState::Joinable);

    // The thread has already exited, so clean up after it.
    if (state->hasExited())
        threadMap().remove(threadID);
    // The thread hasn't exited yet, so don't clean anything up. Just signal that we've already joined on it so that it will clean up after itself.
    else
        state->didJoin();

    return joinResult;
}

void detachThread(ThreadIdentifier threadID)
{
    ASSERT(threadID);

    MutexLocker locker(threadMapMutex());
    pthread_t pthreadHandle = pthreadHandleForIdentifierWithLockAlreadyHeld(threadID);
    ASSERT(pthreadHandle);

    int detachResult = pthread_detach(pthreadHandle);
    if (detachResult)
        LOG_ERROR("ThreadIdentifier %u was unable to be detached\n", threadID);

    PthreadState* state = threadMap().get(threadID);
    ASSERT(state);
    if (state->hasExited())
        threadMap().remove(threadID);
    else
        threadMap().get(threadID)->didBecomeDetached();
}

void threadDidExit(ThreadIdentifier threadID)
{
    MutexLocker locker(threadMapMutex());
    PthreadState* state = threadMap().get(threadID);
    ASSERT(state);
    
    state->didExit();

    if (state->joinableState() != PthreadState::Joinable)
        threadMap().remove(threadID);
}

ThreadIdentifier currentThread()
{
    ThreadIdentifier id = ThreadIdentifierData::identifier();
    if (id)
        return id;

    // Not a WTF-created thread, ThreadIdentifier is not established yet.
    id = establishIdentifierForPthreadHandle(pthread_self());
    ThreadIdentifierData::initialize(id);
    return id;
}

Mutex::Mutex()
{
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_NORMAL);

    int result = pthread_mutex_init(&m_mutex, &attr);
    ASSERT_UNUSED(result, !result);

    pthread_mutexattr_destroy(&attr);
}

Mutex::~Mutex()
{
    int result = pthread_mutex_destroy(&m_mutex);
    ASSERT_UNUSED(result, !result);
}

void Mutex::lock()
{
    int result = pthread_mutex_lock(&m_mutex);
    ASSERT_UNUSED(result, !result);
}

bool Mutex::tryLock()
{
    int result = pthread_mutex_trylock(&m_mutex);

    if (result == 0)
        return true;
    if (result == EBUSY)
        return false;

    ASSERT_NOT_REACHED();
    return false;
}

void Mutex::unlock()
{
    int result = pthread_mutex_unlock(&m_mutex);
    ASSERT_UNUSED(result, !result);
}

ThreadCondition::ThreadCondition()
{ 
    pthread_cond_init(&m_condition, NULL);
}

ThreadCondition::~ThreadCondition()
{
    pthread_cond_destroy(&m_condition);
}
    
void ThreadCondition::wait(Mutex& mutex)
{
    int result = pthread_cond_wait(&m_condition, &mutex.impl());
    ASSERT_UNUSED(result, !result);
}

bool ThreadCondition::timedWait(Mutex& mutex, double absoluteTime)
{
    if (absoluteTime < currentTime())
        return false;

    if (absoluteTime > INT_MAX) {
        wait(mutex);
        return true;
    }

    int timeSeconds = static_cast<int>(absoluteTime);
    int timeNanoseconds = static_cast<int>((absoluteTime - timeSeconds) * 1E9);

    timespec targetTime;
    targetTime.tv_sec = timeSeconds;
    targetTime.tv_nsec = timeNanoseconds;

    return pthread_cond_timedwait(&m_condition, &mutex.impl(), &targetTime) == 0;
}

void ThreadCondition::signal()
{
    int result = pthread_cond_signal(&m_condition);
    ASSERT_UNUSED(result, !result);
}

void ThreadCondition::broadcast()
{
    int result = pthread_cond_broadcast(&m_condition);
    ASSERT_UNUSED(result, !result);
}

} // namespace WTF

#endif // USE(PTHREADS)
