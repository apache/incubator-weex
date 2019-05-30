/*
 * Copyright (C) 2007, 2008, 2010, 2014 Apple Inc. All rights reserved.
 * Copyright (C) 2007 Justin Haygood (jhaygood@reaktix.com)
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

#ifndef Threading_h
#define Threading_h

// FIXME: Not sure why there are so many includes here.
// Is this intended to be convenience so that others don't have to include the individual files?
// Nothing in this header depends on Assertions, Atomics, Locker, Noncopyable, ThreadSafeRefCounted, or ThreadingPrimitives.

#include <functional>
#include <stdint.h>
#include <wtf/Assertions.h>
#include <wtf/Atomics.h>
#include <wtf/Locker.h>
#include <wtf/Noncopyable.h>
#include <wtf/ThreadSafeRefCounted.h>
#include <wtf/ThreadingPrimitives.h>

namespace WTF {

typedef uint32_t ThreadIdentifier;
typedef void (*ThreadFunction)(void* argument);

// This function must be called from the main thread. It is safe to call it repeatedly.
// Darwin is an exception to this rule: it is OK to call it from any thread, the only 
// requirement is that the calls are not reentrant.
WTF_EXPORT_PRIVATE void initializeThreading();

// Returns 0 if thread creation failed.
// The thread name must be a literal since on some platforms it's passed in to the thread.
WTF_EXPORT_PRIVATE ThreadIdentifier createThread(const char* threadName, std::function<void()>);

// Mark the current thread as requiring UI responsiveness.
// relativePriority is a value in the range [-15, 0] where a lower value indicates a lower priority.
WTF_EXPORT_PRIVATE void setCurrentThreadIsUserInteractive(int relativePriority = 0);
WTF_EXPORT_PRIVATE void setCurrentThreadIsUserInitiated(int relativePriority = 0);

WTF_EXPORT_PRIVATE ThreadIdentifier currentThread();
WTF_EXPORT_PRIVATE void changeThreadPriority(ThreadIdentifier, int);
WTF_EXPORT_PRIVATE int waitForThreadCompletion(ThreadIdentifier);
WTF_EXPORT_PRIVATE void detachThread(ThreadIdentifier);

// Deprecated function-pointer-based thread creation.
WTF_EXPORT_PRIVATE ThreadIdentifier createThread(ThreadFunction, void*, const char* threadName);

// Internal platform-specific createThread implementation.
ThreadIdentifier createThreadInternal(ThreadFunction, void*, const char* threadName);

// Called in the thread during initialization.
// Helpful for platforms where the thread name must be set from within the thread.
void initializeCurrentThreadInternal(const char* threadName);

const char* normalizeThreadName(const char* threadName);

#if HAVE(QOS_CLASSES)
WTF_EXPORT_PRIVATE void setGlobalMaxQOSClass(qos_class_t);
WTF_EXPORT_PRIVATE qos_class_t adjustedQOSClass(qos_class_t);
#endif

} // namespace WTF

using WTF::ThreadIdentifier;
using WTF::createThread;
using WTF::currentThread;
using WTF::changeThreadPriority;
using WTF::detachThread;
using WTF::waitForThreadCompletion;

#if HAVE(QOS_CLASSES)
using WTF::setGlobalMaxQOSClass;
using WTF::adjustedQOSClass;
#endif

#endif // Threading_h
