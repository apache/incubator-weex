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
