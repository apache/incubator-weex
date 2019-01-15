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
#ifndef StaticMutex_h
#define StaticMutex_h

#include "BAssert.h"
#include <atomic>
#include <mutex>
#include <thread>

// A fast replacement for std::mutex, for use in static storage.

// Use StaticMutex in static storage, where global constructors and exit-time
// destructors are prohibited, but all memory is zero-initialized automatically.

namespace bmalloc {

class StaticMutex {
protected:
    // Subclasses that support non-static storage must use explicit initialization.
    void init();

public:
    void lock();
    bool try_lock();
    void unlock();

private:
    void lockSlowCase();

    std::atomic_flag m_flag;
    std::atomic_flag m_isSpinning;
};

static inline void sleep(
    std::unique_lock<StaticMutex>& lock, std::chrono::milliseconds duration)
{
    if (duration == std::chrono::milliseconds(0))
        return;
    
    lock.unlock();
    std::this_thread::sleep_for(duration);
    lock.lock();
}

static inline void waitUntilFalse(
    std::unique_lock<StaticMutex>& lock, std::chrono::milliseconds sleepDuration,
    bool& condition)
{
    while (condition) {
        condition = false;
        sleep(lock, sleepDuration);
    }
}

inline void StaticMutex::init()
{
    m_flag.clear();
    m_isSpinning.clear();
}

inline bool StaticMutex::try_lock()
{
    return !m_flag.test_and_set(std::memory_order_acquire);
}

inline void StaticMutex::lock()
{
    if (!try_lock())
        lockSlowCase();
}

inline void StaticMutex::unlock()
{
    m_flag.clear(std::memory_order_release);
}

} // namespace bmalloc

#endif // StaticMutex_h
