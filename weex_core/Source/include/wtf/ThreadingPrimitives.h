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
#ifndef ThreadingPrimitives_h
#define ThreadingPrimitives_h

#include <wtf/Assertions.h>
#include <wtf/FastMalloc.h>
#include <wtf/Locker.h>
#include <wtf/Noncopyable.h>

#if OS(WINDOWS)
#include <windows.h>
#endif

#if USE(PTHREADS)
#include <pthread.h>
#endif

namespace WTF {

#if USE(PTHREADS)
typedef pthread_mutex_t PlatformMutex;
typedef pthread_cond_t PlatformCondition;
#elif OS(WINDOWS)
struct PlatformMutex {
    CRITICAL_SECTION m_internalMutex;
    size_t m_recursionCount;
};
struct PlatformCondition {
    size_t m_waitersGone;
    size_t m_waitersBlocked;
    size_t m_waitersToUnblock; 
    HANDLE m_blockLock;
    HANDLE m_blockQueue;
    HANDLE m_unblockLock;

    bool timedWait(PlatformMutex&, DWORD durationMilliseconds);
    void signal(bool unblockAll);
};
#else
typedef void* PlatformMutex;
typedef void* PlatformCondition;
#endif
    
class Mutex {
    WTF_MAKE_NONCOPYABLE(Mutex); WTF_MAKE_FAST_ALLOCATED;
public:
    WTF_EXPORT_PRIVATE Mutex();
    WTF_EXPORT_PRIVATE ~Mutex();

    WTF_EXPORT_PRIVATE void lock();
    WTF_EXPORT_PRIVATE bool tryLock();
    WTF_EXPORT_PRIVATE void unlock();

public:
    PlatformMutex& impl() { return m_mutex; }
private:
    PlatformMutex m_mutex;
};

typedef Locker<Mutex> MutexLocker;

class ThreadCondition {
    WTF_MAKE_NONCOPYABLE(ThreadCondition);
public:
    WTF_EXPORT_PRIVATE ThreadCondition();
    WTF_EXPORT_PRIVATE ~ThreadCondition();
    
    WTF_EXPORT_PRIVATE void wait(Mutex& mutex);
    // Returns true if the condition was signaled before absoluteTime, false if the absoluteTime was reached or is in the past.
    // The absoluteTime is in seconds, starting on January 1, 1970. The time is assumed to use the same time zone as WTF::currentTime().
    WTF_EXPORT_PRIVATE bool timedWait(Mutex&, double absoluteTime);
    WTF_EXPORT_PRIVATE void signal();
    WTF_EXPORT_PRIVATE void broadcast();
    
private:
    PlatformCondition m_condition;
};

#if OS(WINDOWS)
// The absoluteTime is in seconds, starting on January 1, 1970. The time is assumed to use the same time zone as WTF::currentTime().
// Returns an interval in milliseconds suitable for passing to one of the Win32 wait functions (e.g., ::WaitForSingleObject).
WTF_EXPORT_PRIVATE DWORD absoluteTimeToWaitTimeoutInterval(double absoluteTime);
#endif

} // namespace WTF

using WTF::Mutex;
using WTF::MutexLocker;
using WTF::ThreadCondition;

#if OS(WINDOWS)
using WTF::absoluteTimeToWaitTimeoutInterval;
#endif

#endif // ThreadingPrimitives_h
