/*
 * Copyright (C) 2010 Apple Inc. All rights reserved.
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

#include "config.h"
#include "WorkQueue.h"

#include <mutex>
#include <wtf/Condition.h>
#include <wtf/Deque.h>
#include <wtf/Lock.h>
#include <wtf/NeverDestroyed.h>
#include <wtf/NumberOfCores.h>
#include <wtf/Ref.h>
#include <wtf/Threading.h>
#include <wtf/text/WTFString.h>
#include <wtf/threads/BinarySemaphore.h>

namespace WTF {

Ref<WorkQueue> WorkQueue::create(const char* name, Type type, QOS qos)
{
    return adoptRef(*new WorkQueue(name, type, qos));
}

WorkQueue::WorkQueue(const char* name, Type type, QOS qos)
{
    platformInitialize(name, type, qos);
}

WorkQueue::~WorkQueue()
{
    platformInvalidate();
}

#if !PLATFORM(COCOA)
void WorkQueue::concurrentApply(size_t iterations, const std::function<void (size_t index)>& function)
{
    if (!iterations)
        return;

    if (iterations == 1) {
        function(0);
        return;
    }

    class ThreadPool {
    public:
        ThreadPool()
        {
            // We don't need a thread for the current core.
            unsigned threadCount = numberOfProcessorCores() - 1;

            m_workers.reserveInitialCapacity(threadCount);
            for (unsigned i = 0; i < threadCount; ++i) {
                m_workers.append(createThread(String::format("ThreadPool Worker %u", i).utf8().data(), [this] {
                    threadBody();
                }));
            }
        }

        size_t workerCount() const { return m_workers.size(); }

        void dispatch(const std::function<void ()>* function)
        {
            LockHolder holder(m_lock);

            m_queue.append(function);
            m_condition.notifyOne();
        }

    private:
        NO_RETURN void threadBody()
        {
            while (true) {
                const std::function<void ()>* function;

                {
                    LockHolder holder(m_lock);

                    m_condition.wait(m_lock, [this] {
                        return !m_queue.isEmpty();
                    });

                    function = m_queue.takeFirst();
                }

                (*function)();
            }
        }

        Lock m_lock;
        Condition m_condition;
        Deque<const std::function<void ()>*> m_queue;

        Vector<ThreadIdentifier> m_workers;
    };

    static LazyNeverDestroyed<ThreadPool> threadPool;
    static std::once_flag onceFlag;
    std::call_once(onceFlag, [] {
        threadPool.construct();
    });

    // Cap the worker count to the number of iterations (excluding this thread)
    const size_t workerCount = std::min(iterations - 1, threadPool->workerCount());

    std::atomic<size_t> currentIndex(0);
    std::atomic<size_t> activeThreads(workerCount + 1);

    Condition condition;
    Lock lock;

    std::function<void ()> applier = [&] {
        size_t index;

        // Call the function for as long as there are iterations left.
        while ((index = currentIndex++) < iterations)
            function(index);

        // If there are no active threads left, signal the caller.
        if (!--activeThreads) {
            LockHolder holder(lock);
            condition.notifyOne();
        }
    };

    for (size_t i = 0; i < workerCount; ++i)
        threadPool->dispatch(&applier);
    applier();

    LockHolder holder(lock);
    condition.wait(lock, [&] { return !activeThreads; });
}
#endif

}
