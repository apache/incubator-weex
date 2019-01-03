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
#ifndef MainThread_h
#define MainThread_h

#include <functional>
#include <stdint.h>
#include <wtf/Function.h>
#include <wtf/Optional.h>

namespace WTF {

class PrintStream;

typedef uint32_t ThreadIdentifier;

// Must be called from the main thread.
WTF_EXPORT_PRIVATE void initializeMainThread();

WTF_EXPORT_PRIVATE void callOnMainThread(Function<void ()>&&);

#if PLATFORM(COCOA)
WTF_EXPORT_PRIVATE void callOnWebThreadOrDispatchAsyncOnMainThread(void (^block)());
#endif

WTF_EXPORT_PRIVATE void setMainThreadCallbacksPaused(bool paused);

WTF_EXPORT_PRIVATE bool isMainThread();

WTF_EXPORT_PRIVATE bool canAccessThreadLocalDataForThread(ThreadIdentifier);

#if USE(WEB_THREAD)
WTF_EXPORT_PRIVATE bool isWebThread();
WTF_EXPORT_PRIVATE bool isUIThread();
WTF_EXPORT_PRIVATE void initializeWebThread();
WTF_EXPORT_PRIVATE void initializeApplicationUIThreadIdentifier();
WTF_EXPORT_PRIVATE void initializeWebThreadIdentifier();
void initializeWebThreadPlatform();
#else
inline bool isWebThread() { return isMainThread(); }
inline bool isUIThread() { return isMainThread(); }
#endif // USE(WEB_THREAD)

WTF_EXPORT_PRIVATE void initializeGCThreads();

enum class GCThreadType {
    Main,
    Helper
};

void printInternal(PrintStream&, GCThreadType);

WTF_EXPORT_PRIVATE void registerGCThread(GCThreadType);
WTF_EXPORT_PRIVATE std::optional<GCThreadType> mayBeGCThread();
WTF_EXPORT_PRIVATE bool isMainThreadOrGCThread();

// NOTE: these functions are internal to the callOnMainThread implementation.
void initializeMainThreadPlatform();
void scheduleDispatchFunctionsOnMainThread();
void dispatchFunctionsFromMainThread();

#if OS(DARWIN) && !PLATFORM(GTK)
#if !USE(WEB_THREAD)
// This version of initializeMainThread sets up the main thread as corresponding
// to the process's main thread, and not necessarily the thread that calls this
// function. It should only be used as a legacy aid for Mac WebKit.
WTF_EXPORT_PRIVATE void initializeMainThreadToProcessMainThread();
#endif // !USE(WEB_THREAD)
void initializeMainThreadToProcessMainThreadPlatform();
#endif

} // namespace WTF

using WTF::GCThreadType;
using WTF::callOnMainThread;
using WTF::canAccessThreadLocalDataForThread;
using WTF::isMainThread;
using WTF::isMainThreadOrGCThread;
using WTF::isUIThread;
using WTF::isWebThread;
using WTF::mayBeGCThread;
using WTF::setMainThreadCallbacksPaused;
#if PLATFORM(COCOA)
using WTF::callOnWebThreadOrDispatchAsyncOnMainThread;
#endif
#if USE(WEB_THREAD)
using WTF::initializeWebThread;
using WTF::initializeApplicationUIThreadIdentifier;
using WTF::initializeWebThreadIdentifier;
#endif

#endif // MainThread_h
