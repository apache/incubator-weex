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

#include <wtf/Condition.h>
#include <wtf/Deque.h>
#include <wtf/HashSet.h>
#include <wtf/Lock.h>
#include <wtf/Locker.h>

namespace WTF {

template<typename T, size_t BufferSize>
class SynchronizedFixedQueue {
public:
    SynchronizedFixedQueue()
    {
        static_assert(!((BufferSize - 1) & BufferSize), "BufferSize must be power of 2.");
    }
    
    void open()
    {
        LockHolder lockHolder(m_mutex);
        if (m_open)
            return;

        // Restore the queue to its original state.
        m_open = true;
        m_queue.clear();
    }
    
    void close()
    {
        LockHolder lockHolder(m_mutex);
        if (!m_open)
            return;

        // Wake all the sleeping threads up with a closing state.
        m_open = false;
        m_condition.notifyAll();
    }
    
    bool isOpen()
    {
        LockHolder lockHolder(m_mutex);
        return m_open;
    }

    bool enqueue(const T& value)
    {
        LockHolder lockHolder(m_mutex);

        // Wait for an empty place to be available in the queue.
        m_condition.wait(m_mutex, [this]() { return !m_open || m_queue.size() < BufferSize; });
        
        // The queue is closing, exit immediately.
        if (!m_open)
            return false;
        
        // Add the item in the queue.
        m_queue.append(value);

        // Notify the other threads that an item was added to the queue.
        m_condition.notifyAll();
        return true;
    }
    
    bool dequeue(T& value)
    {
        LockHolder lockHolder(m_mutex);
        
        // Wait for an item to be added.
        m_condition.wait(m_mutex, [this]() { return !m_open || m_queue.size(); });

        // The queue is closing, exit immediately.
        if (!m_open)
            return false;

        // Get a copy from m_queue.first and then remove it.
        value = m_queue.first();
        m_queue.removeFirst();

        // Notify the other threads that an item was removed from the queue.
        m_condition.notifyAll();
        return true;
    }

private:
    Lock m_mutex;
    Condition m_condition;

    bool m_open { true };
    Deque<T, BufferSize> m_queue;
};

}

using WTF::SynchronizedFixedQueue;
