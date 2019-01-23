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
#include <wtf/RefPtr.h>
#include <wtf/RetainPtr.h>
#include <wtf/ThreadSafeRefCounted.h>
#include <wtf/Threading.h>

#if USE(CF)
#include <CoreFoundation/CoreFoundation.h>
#endif

#if USE(GLIB)
#include <wtf/glib/GRefPtr.h>
#endif

namespace JSC {

class JSLock;
class VM;

class HeapTimer : public ThreadSafeRefCounted<HeapTimer> {
public:
    HeapTimer(VM*);
#if USE(CF)
    static void timerDidFire(CFRunLoopTimerRef, void*);
#elif defined(__ANDROID__)
    void timerDidFire();
    static void startTimerThread();
    static void stopTimerThread();
#endif
    
    JS_EXPORT_PRIVATE virtual ~HeapTimer();
    virtual void doWork() = 0;

    void scheduleTimer(double intervalInSeconds);
    void cancelTimer();
    bool isScheduled() const { return m_isScheduled; }

#if USE(CF)
    JS_EXPORT_PRIVATE void setRunLoop(CFRunLoopRef);
#endif // USE(CF)
    
protected:
    VM* m_vm;

    RefPtr<JSLock> m_apiLock;
    bool m_isScheduled { false };
#if USE(CF)
    static const CFTimeInterval s_decade;

    RetainPtr<CFRunLoopTimerRef> m_timer;
    RetainPtr<CFRunLoopRef> m_runLoop;
    
    CFRunLoopTimerContext m_context;

    Lock m_shutdownMutex;
#elif USE(GLIB)
    static const long s_decade;
    void timerDidFire();
    GRefPtr<GSource> m_timer;
#endif
    
private:
    void invalidate();
};

} // namespace JSC
