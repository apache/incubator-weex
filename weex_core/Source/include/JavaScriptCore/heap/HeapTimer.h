/*
 * Copyright (C) 2012, 2015-2016 Apple Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
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
