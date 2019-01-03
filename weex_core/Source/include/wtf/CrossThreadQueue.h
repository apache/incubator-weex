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

#include <limits>
#include <wtf/Assertions.h>
#include <wtf/Condition.h>
#include <wtf/Deque.h>
#include <wtf/Lock.h>
#include <wtf/Noncopyable.h>
#include <wtf/Optional.h>
#include <wtf/Threading.h>

namespace WTF {

template<typename DataType>
class CrossThreadQueue {
    WTF_MAKE_NONCOPYABLE(CrossThreadQueue);
public:
    CrossThreadQueue() = default;

    void append(DataType&&);

    DataType waitForMessage();
    std::optional<DataType> tryGetMessage();

    bool isKilled() const { return false; }

private:
    mutable Lock m_lock;
    Condition m_condition;
    Deque<DataType> m_queue;
};

template<typename DataType>
void CrossThreadQueue<DataType>::append(DataType&& message)
{
    LockHolder lock(m_lock);
    m_queue.append(WTFMove(message));
    m_condition.notifyOne();
}

template<typename DataType>
DataType CrossThreadQueue<DataType>::waitForMessage()
{
    LockHolder lock(m_lock);

    auto found = m_queue.end();
    while (found == m_queue.end()) {
        found = m_queue.begin();
        if (found != m_queue.end())
            break;

        m_condition.wait(m_lock);
    }

    return m_queue.takeFirst();
}

template<typename DataType>
std::optional<DataType> CrossThreadQueue<DataType>::tryGetMessage()
{
    LockHolder lock(m_lock);

    if (m_queue.isEmpty())
        return { };

    return m_queue.takeFirst();
}

} // namespace WTF

using WTF::CrossThreadQueue;
