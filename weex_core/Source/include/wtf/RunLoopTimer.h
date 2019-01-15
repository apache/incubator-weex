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
#ifndef RunLoopTimer_h
#define RunLoopTimer_h

#include <wtf/SchedulePair.h>
#include <wtf/RetainPtr.h>

namespace WTF {

// Time intervals are all in seconds.

class WTF_EXPORT_PRIVATE RunLoopTimerBase {
    WTF_MAKE_NONCOPYABLE(RunLoopTimerBase);
public:
    RunLoopTimerBase() { }
    WTF_EXPORT_PRIVATE virtual ~RunLoopTimerBase();

    WTF_EXPORT_PRIVATE void schedule(const SchedulePair*);
    WTF_EXPORT_PRIVATE void schedule(const SchedulePairHashSet&);

    WTF_EXPORT_PRIVATE void start(double nextFireInterval, double repeatInterval);

    void startRepeating(double repeatInterval) { start(repeatInterval, repeatInterval); }
    void startOneShot(double interval) { start(interval, 0); }

    WTF_EXPORT_PRIVATE void stop();
    bool isActive() const;

    virtual void fired() = 0;

private:
#if USE(CF)
    RetainPtr<CFRunLoopTimerRef> m_timer;
#endif
};

// FIXME: This doesn't have to be a class template.
template <typename TimerFiredClass> class RunLoopTimer : public RunLoopTimerBase {
public:
    typedef void (TimerFiredClass::*TimerFiredFunction)();

    RunLoopTimer(TimerFiredClass& o, TimerFiredFunction f)
        : m_object(&o), m_function(f) { }

    virtual void fired() { (m_object->*m_function)(); }

private:
    TimerFiredClass* m_object;
    TimerFiredFunction m_function;
};

} // namespace WTF

using WTF::RunLoopTimer;

#endif
