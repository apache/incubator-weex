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
#ifndef AsyncTask_h
#define AsyncTask_h

#include "BAssert.h"
#include "Inline.h"
#include "Mutex.h"
#include "Sizes.h"
#include <atomic>
#include <condition_variable>
#include <thread>

namespace bmalloc {

template<typename Object, typename Function>
class AsyncTask {
public:
    AsyncTask(Object&, const Function&);
    ~AsyncTask();
    
    bool willRun() { return m_state == State::Run; }
    void run();
    
    bool willRunSoon() { return m_state > State::Sleep; }
    void runSoon();
    
private:
    enum class State { Sleep, Run, RunSoon };
    
    void runSlowCase();
    void runSoonSlowCase();
    
    static void threadEntryPoint(AsyncTask*);
    void threadRunLoop();

    std::atomic<State> m_state;

    Mutex m_conditionMutex;
    std::condition_variable_any m_condition;

    std::thread m_thread;

    Object& m_object;
    Function m_function;
};

template<typename Object, typename Function>
AsyncTask<Object, Function>::AsyncTask(Object& object, const Function& function)
    : m_state(State::Sleep)
    , m_condition()
    , m_thread(std::thread(&AsyncTask::threadEntryPoint, this))
    , m_object(object)
    , m_function(function)
{
}

template<typename Object, typename Function>
AsyncTask<Object, Function>::~AsyncTask()
{
    // We'd like to mark our destructor deleted but C++ won't allow it because
    // we are an automatic member of Heap.
    RELEASE_BASSERT(0);
}

template<typename Object, typename Function>
void AsyncTask<Object, Function>::run()
{
    m_state = State::Run;
    
    std::lock_guard<Mutex> lock(m_conditionMutex);
    m_condition.notify_all();
}
    
template<typename Object, typename Function>
void AsyncTask<Object, Function>::runSoon()
{
    m_state = State::RunSoon;
    
    std::lock_guard<Mutex> lock(m_conditionMutex);
    m_condition.notify_all();
}

template<typename Object, typename Function>
void AsyncTask<Object, Function>::threadEntryPoint(AsyncTask* asyncTask)
{
#if BOS(DARWIN)
    pthread_set_qos_class_self_np(QOS_CLASS_USER_INTERACTIVE, 0);
#endif

    asyncTask->threadRunLoop();
}

template<typename Object, typename Function>
void AsyncTask<Object, Function>::threadRunLoop()
{
    // This loop ratchets downward from most active to least active state. While
    // we ratchet downward, any other thread may reset our state.
    
    // We require any state change while we are sleeping to signal to our
    // condition variable and wake us up.
    
    while (1) {
        if (m_state == State::Sleep) {
            std::unique_lock<Mutex> lock(m_conditionMutex);
            m_condition.wait(lock, [&]() { return m_state != State::Sleep; });
        }
        
        if (m_state == State::RunSoon) {
            std::unique_lock<Mutex> lock(m_conditionMutex);
            m_condition.wait_for(lock, asyncTaskSleepDuration, [&]() { return m_state != State::RunSoon; });
        }
        
        m_state = State::Sleep;
        (m_object.*m_function)();
    }
}

} // namespace bmalloc

#endif // AsyncTask_h
