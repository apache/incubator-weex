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
#pragma once

#if OS(DARWIN) || OS(UNIX)
#include <pthread.h>
#endif

#if USE(PTHREADS) && !OS(WINDOWS) && !OS(DARWIN)
#include <signal.h>
#endif

#if OS(DARWIN)
#include <mach/thread_act.h>
#elif OS(WINDOWS)
#include <windows.h>
#endif

namespace JSC {

#if OS(DARWIN)
typedef mach_port_t PlatformThread;
#elif OS(WINDOWS)
typedef DWORD PlatformThread;
#elif USE(PTHREADS)
typedef pthread_t PlatformThread;
#endif // OS(DARWIN)
    
inline PlatformThread currentPlatformThread()
{
#if OS(DARWIN)
    return pthread_mach_thread_np(pthread_self());
#elif OS(WINDOWS)
    return GetCurrentThreadId();
#elif USE(PTHREADS)
    return pthread_self();
#endif
}

#if OS(DARWIN)
inline bool platformThreadSignal(PlatformThread platformThread, int signalNumber)
{
    pthread_t pthreadID = pthread_from_mach_thread_np(platformThread);
    int errNo = pthread_kill(pthreadID, signalNumber);
    return !errNo; // A 0 errNo means success.
}
#elif USE(PTHREADS)
inline bool platformThreadSignal(PlatformThread pthreadID, int signalNumber)
{
    int errNo = pthread_kill(pthreadID, signalNumber);
    return !errNo; // A 0 errNo means success.
}
#endif

} // namespace JSC
