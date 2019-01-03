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
#ifndef MessageQueue_h
#define MessageQueue_h

#include <limits>
#include <wtf/Assertions.h>
#include <wtf/Condition.h>
#include <wtf/Deque.h>
#include <wtf/Lock.h>
#include <wtf/Noncopyable.h>
#include <wtf/Threading.h>
#include <wtf/WallTime.h>

namespace WTF {

    enum MessageQueueWaitResult {
        MessageQueueTerminated,       // Queue was destroyed while waiting for message.
        MessageQueueTimeout,          // Timeout was specified and it expired.
        MessageQueueMessageReceived   // A message was successfully received and returned.
    };

    // The queue takes ownership of messages and transfer it to the new owner
    // when messages are fetched from the queue.
    // Essentially, MessageQueue acts as a queue of std::unique_ptr<DataType>.
    template<typename DataType>
    class MessageQueue {
        WTF_MAKE_NONCOPYABLE(MessageQueue);
    public:
        MessageQueue() : m_killed(false) { }
        ~MessageQueue();

        void append(std::unique_ptr<DataType>);
        void appendAndKill(std::unique_ptr<DataType>);
        bool appendAndCheckEmpty(std::unique_ptr<DataType>);
        void prepend(std::unique_ptr<DataType>);

        std::unique_ptr<DataType> waitForMessage();
        std::unique_ptr<DataType> tryGetMessage();
        Deque<std::unique_ptr<DataType>> takeAllMessages();
        std::unique_ptr<DataType> tryGetMessageIgnoringKilled();
        template<typename Predicate>
        std::unique_ptr<DataType> waitForMessageFilteredWithTimeout(MessageQueueWaitResult&, Predicate&&, WallTime absoluteTime);

        template<typename Predicate>
        void removeIf(Predicate&&);

        void kill();
        bool killed() const;

        // The result of isEmpty() is only valid if no other thread is manipulating the queue at the same time.
        bool isEmpty();

    private:
        mutable Lock m_mutex;
        Condition m_condition;
        Deque<std::unique_ptr<DataType>> m_queue;
        bool m_killed;
    };

    template<typename DataType>
    MessageQueue<DataType>::~MessageQueue()
    {
    }

    template<typename DataType>
    inline void MessageQueue<DataType>::append(std::unique_ptr<DataType> message)
    {
        LockHolder lock(m_mutex);
        m_queue.append(WTFMove(message));
        m_condition.notifyOne();
    }

    template<typename DataType>
    inline void MessageQueue<DataType>::appendAndKill(std::unique_ptr<DataType> message)
    {
        LockHolder lock(m_mutex);
        m_queue.append(WTFMove(message));
        m_killed = true;
        m_condition.notifyAll();
    }

    // Returns true if the queue was empty before the item was added.
    template<typename DataType>
    inline bool MessageQueue<DataType>::appendAndCheckEmpty(std::unique_ptr<DataType> message)
    {
        LockHolder lock(m_mutex);
        bool wasEmpty = m_queue.isEmpty();
        m_queue.append(WTFMove(message));
        m_condition.notifyOne();
        return wasEmpty;
    }

    template<typename DataType>
    inline void MessageQueue<DataType>::prepend(std::unique_ptr<DataType> message)
    {
        LockHolder lock(m_mutex);
        m_queue.prepend(WTFMove(message));
        m_condition.notifyOne();
    }

    template<typename DataType>
    inline auto MessageQueue<DataType>::waitForMessage() -> std::unique_ptr<DataType>
    {
        MessageQueueWaitResult exitReason; 
        std::unique_ptr<DataType> result = waitForMessageFilteredWithTimeout(exitReason, [](const DataType&) { return true; }, WallTime::infinity());
        ASSERT(exitReason == MessageQueueTerminated || exitReason == MessageQueueMessageReceived);
        return result;
    }

    template<typename DataType>
    template<typename Predicate>
    inline auto MessageQueue<DataType>::waitForMessageFilteredWithTimeout(MessageQueueWaitResult& result, Predicate&& predicate, WallTime absoluteTime) -> std::unique_ptr<DataType>
    {
        LockHolder lock(m_mutex);
        bool timedOut = false;

        auto found = m_queue.end();
        while (!m_killed && !timedOut) {
            found = m_queue.findIf([&predicate](const std::unique_ptr<DataType>& ptr) -> bool {
                ASSERT(ptr);
                return predicate(*ptr);
            });
            if (found != m_queue.end())
                break;

            timedOut = !m_condition.waitUntil(m_mutex, absoluteTime);
        }

        ASSERT(!timedOut || absoluteTime != WallTime::infinity());

        if (m_killed) {
            result = MessageQueueTerminated;
            return nullptr;
        }

        if (timedOut) {
            result = MessageQueueTimeout;
            return nullptr;
        }

        ASSERT(found != m_queue.end());
        std::unique_ptr<DataType> message = WTFMove(*found);
        m_queue.remove(found);
        result = MessageQueueMessageReceived;
        return message;
    }

    template<typename DataType>
    inline auto MessageQueue<DataType>::tryGetMessage() -> std::unique_ptr<DataType>
    {
        LockHolder lock(m_mutex);
        if (m_killed)
            return nullptr;
        if (m_queue.isEmpty())
            return nullptr;

        return m_queue.takeFirst();
    }

    template<typename DataType>
    inline auto MessageQueue<DataType>::takeAllMessages() -> Deque<std::unique_ptr<DataType>>
    {
        LockHolder lock(m_mutex);
        if (m_killed)
            return { };
        return WTFMove(m_queue);
    }

    template<typename DataType>
    inline auto MessageQueue<DataType>::tryGetMessageIgnoringKilled() -> std::unique_ptr<DataType>
    {
        LockHolder lock(m_mutex);
        if (m_queue.isEmpty())
            return nullptr;

        return m_queue.takeFirst();
    }

    template<typename DataType>
    template<typename Predicate>
    inline void MessageQueue<DataType>::removeIf(Predicate&& predicate)
    {
        LockHolder lock(m_mutex);
        while (true) {
            auto found = m_queue.findIf([&predicate](const std::unique_ptr<DataType>& ptr) -> bool {
                ASSERT(ptr);
                return predicate(*ptr);
            });

            if (found == m_queue.end())
                break;

            m_queue.remove(found);
        }
    }

    template<typename DataType>
    inline bool MessageQueue<DataType>::isEmpty()
    {
        LockHolder lock(m_mutex);
        if (m_killed)
            return true;
        return m_queue.isEmpty();
    }

    template<typename DataType>
    inline void MessageQueue<DataType>::kill()
    {
        LockHolder lock(m_mutex);
        m_killed = true;
        m_condition.notifyAll();
    }

    template<typename DataType>
    inline bool MessageQueue<DataType>::killed() const
    {
        LockHolder lock(m_mutex);
        return m_killed;
    }
} // namespace WTF

using WTF::MessageQueue;
// MessageQueueWaitResult enum and all its values.
using WTF::MessageQueueWaitResult;
using WTF::MessageQueueTerminated;
using WTF::MessageQueueTimeout;
using WTF::MessageQueueMessageReceived;

#endif // MessageQueue_h
