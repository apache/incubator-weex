/*
 * Copyright (C) 2017 Apple Inc. All rights reserved.
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
