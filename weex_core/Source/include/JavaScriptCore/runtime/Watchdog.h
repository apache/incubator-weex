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

#include <wtf/Lock.h>
#include <wtf/Ref.h>
#include <wtf/ThreadSafeRefCounted.h>
#include <wtf/WorkQueue.h>

namespace JSC {

class ExecState;
class VM;

class Watchdog : public WTF::ThreadSafeRefCounted<Watchdog> {
    WTF_MAKE_FAST_ALLOCATED;
public:
    class Scope;

    Watchdog(VM*);
    void willDestroyVM(VM*);

    typedef bool (*ShouldTerminateCallback)(ExecState*, void* data1, void* data2);
    void setTimeLimit(std::chrono::microseconds limit, ShouldTerminateCallback = 0, void* data1 = 0, void* data2 = 0);

    bool shouldTerminate(ExecState*);

    bool hasTimeLimit();
    void enteredVM();
    void exitedVM();

    static const std::chrono::microseconds noTimeLimit;

private:
    void startTimer(std::chrono::microseconds timeLimit);
    void stopTimer();

    Lock m_lock; // Guards access to m_vm.
    VM* m_vm;

    std::chrono::microseconds m_timeLimit;

    std::chrono::microseconds m_cpuDeadline;
    std::chrono::microseconds m_wallClockDeadline;

    bool m_hasEnteredVM { false };

    ShouldTerminateCallback m_callback;
    void* m_callbackData1;
    void* m_callbackData2;

    Ref<WorkQueue> m_timerQueue;

    friend class LLIntOffsetsExtractor;
};

} // namespace JSC
