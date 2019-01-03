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
