/*
 * Copyright (C) 2010, 2015 Apple Inc. All rights reserved.
 * Portions Copyright (c) 2010 Motorola Mobility, Inc.  All rights reserved.
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

#ifndef WorkQueue_h
#define WorkQueue_h

#include <functional>
#include <wtf/Forward.h>
#include <wtf/FunctionDispatcher.h>
#include <wtf/RefCounted.h>
#include <wtf/Seconds.h>
#include <wtf/Threading.h>

#if USE(COCOA_EVENT_LOOP)
#include <dispatch/dispatch.h>
#endif

#if USE(WINDOWS_EVENT_LOOP)
#include <wtf/Vector.h>
#endif

#if USE(GLIB_EVENT_LOOP) || USE(GENERIC_EVENT_LOOP)
#include <wtf/Condition.h>
#include <wtf/RunLoop.h>
#endif

namespace WTF {

class WorkQueue final : public FunctionDispatcher {
public:
    enum class Type {
        Serial,
        Concurrent
    };
    enum class QOS {
        UserInteractive,
        UserInitiated,
        Default,
        Utility,
        Background
    };

    WTF_EXPORT_PRIVATE static Ref<WorkQueue> create(const char* name, Type = Type::Serial, QOS = QOS::Default);
    virtual ~WorkQueue();

    WTF_EXPORT_PRIVATE void dispatch(Function<void()>&&) override;
    WTF_EXPORT_PRIVATE void dispatchAfter(Seconds, Function<void()>&&);

    WTF_EXPORT_PRIVATE static void concurrentApply(size_t iterations, const std::function<void(size_t index)>&);

#if USE(COCOA_EVENT_LOOP)
    dispatch_queue_t dispatchQueue() const { return m_dispatchQueue; }
#elif USE(GLIB_EVENT_LOOP) || USE(GENERIC_EVENT_LOOP)
    RunLoop& runLoop() const { return *m_runLoop; }
#endif

private:
    explicit WorkQueue(const char* name, Type, QOS);

    void platformInitialize(const char* name, Type, QOS);
    void platformInvalidate();

#if USE(WINDOWS_EVENT_LOOP)
    static void CALLBACK timerCallback(void* context, BOOLEAN timerOrWaitFired);
    static DWORD WINAPI workThreadCallback(void* context);

    bool tryRegisterAsWorkThread();
    void unregisterAsWorkThread();
    void performWorkOnRegisteredWorkThread();
#endif

#if USE(COCOA_EVENT_LOOP)
    static void executeFunction(void*);
    dispatch_queue_t m_dispatchQueue;
#elif USE(WINDOWS_EVENT_LOOP)
    volatile LONG m_isWorkThreadRegistered;

    Mutex m_functionQueueLock;
    Vector<Function<void()>> m_functionQueue;

    HANDLE m_timerQueue;
#elif USE(GLIB_EVENT_LOOP) || USE(GENERIC_EVENT_LOOP)
    ThreadIdentifier m_workQueueThread;
    Lock m_initializeRunLoopConditionMutex;
    Condition m_initializeRunLoopCondition;
    RunLoop* m_runLoop;
#endif
};

}

using WTF::WorkQueue;

#endif
