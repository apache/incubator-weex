/*
 * Copyright (C) 2010 Apple Inc. All rights reserved.
 * Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies)
 * Portions Copyright (c) 2010 Motorola Mobility, Inc.  All rights reserved.
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

#ifndef RunLoop_h
#define RunLoop_h

#include <wtf/Condition.h>
#include <wtf/Deque.h>
#include <wtf/Forward.h>
#include <wtf/FunctionDispatcher.h>
#include <wtf/HashMap.h>
#include <wtf/RetainPtr.h>
#include <wtf/Seconds.h>
#include <wtf/Threading.h>

#if USE(GLIB_EVENT_LOOP)
#include <wtf/glib/GRefPtr.h>
#endif

namespace WTF {

class RunLoop : public FunctionDispatcher {
    WTF_MAKE_NONCOPYABLE(RunLoop);
public:
    // Must be called from the main thread (except for the Mac platform, where it
    // can be called from any thread).
    WTF_EXPORT_PRIVATE static void initializeMainRunLoop();

    WTF_EXPORT_PRIVATE static RunLoop& current();
    WTF_EXPORT_PRIVATE static RunLoop& main();
    WTF_EXPORT_PRIVATE static bool isMain();
    ~RunLoop();

    void dispatch(Function<void()>&&) override;

    WTF_EXPORT_PRIVATE static void run();
    WTF_EXPORT_PRIVATE void stop();
    WTF_EXPORT_PRIVATE void wakeUp();

#if USE(COCOA_EVENT_LOOP)
    WTF_EXPORT_PRIVATE void runForDuration(double duration);
#endif

#if USE(GLIB_EVENT_LOOP)
    WTF_EXPORT_PRIVATE GMainContext* mainContext() const { return m_mainContext.get(); }
#endif

#if USE(GENERIC_EVENT_LOOP)
    // Run the single iteration of the RunLoop. It consumes the pending tasks and expired timers, but it won't be blocked.
    WTF_EXPORT_PRIVATE static void iterate();
#endif

#if USE(GLIB_EVENT_LOOP) || USE(GENERIC_EVENT_LOOP)
    WTF_EXPORT_PRIVATE void dispatchAfter(Seconds, Function<void()>&&);
#endif

    class TimerBase {
        friend class RunLoop;
    public:
        WTF_EXPORT_PRIVATE explicit TimerBase(RunLoop&);
        WTF_EXPORT_PRIVATE virtual ~TimerBase();

        void startRepeating(double repeatInterval) { start(repeatInterval, true); }
        void startRepeating(std::chrono::milliseconds repeatInterval) { startRepeating(repeatInterval.count() * 0.001); }
        void startRepeating(Seconds repeatInterval) { startRepeating(repeatInterval.value()); }
        void startOneShot(double interval) { start(interval, false); }
        void startOneShot(std::chrono::milliseconds interval) { start(interval.count() * 0.001, false); }
        void startOneShot(Seconds interval) { start(interval.value(), false); }

        WTF_EXPORT_PRIVATE void stop();
        WTF_EXPORT_PRIVATE bool isActive() const;

        virtual void fired() = 0;

#if USE(GLIB_EVENT_LOOP)
        void setPriority(int);
#endif

    private:
        WTF_EXPORT_PRIVATE void start(double nextFireInterval, bool repeat);

        RunLoop& m_runLoop;

#if USE(WINDOWS_EVENT_LOOP)
        static void timerFired(RunLoop*, uint64_t ID);
        uint64_t m_ID;
        bool m_isRepeating;
#elif USE(COCOA_EVENT_LOOP)
        static void timerFired(CFRunLoopTimerRef, void*);
        RetainPtr<CFRunLoopTimerRef> m_timer;
#elif USE(GLIB_EVENT_LOOP)
        void updateReadyTime();
        GRefPtr<GSource> m_source;
        bool m_isRepeating { false };
        Seconds m_fireInterval { 0 };
#elif USE(GENERIC_EVENT_LOOP)
        class ScheduledTask;
        RefPtr<ScheduledTask> m_scheduledTask;
#endif
    };

    template <typename TimerFiredClass>
    class Timer : public TimerBase {
    public:
        typedef void (TimerFiredClass::*TimerFiredFunction)();

        Timer(RunLoop& runLoop, TimerFiredClass* o, TimerFiredFunction f)
            : TimerBase(runLoop)
            , m_object(o)
            , m_function(f)
        {
        }

    private:
        void fired() override { (m_object->*m_function)(); }

        TimerFiredClass* m_object;
        TimerFiredFunction m_function;
    };

    class Holder;

private:
    RunLoop();

    void performWork();

    Mutex m_functionQueueLock;
    Deque<Function<void()>> m_functionQueue;

#if USE(WINDOWS_EVENT_LOOP)
    static bool registerRunLoopMessageWindowClass();
    static LRESULT CALLBACK RunLoopWndProc(HWND, UINT, WPARAM, LPARAM);
    LRESULT wndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    HWND m_runLoopMessageWindow;

    typedef HashMap<uint64_t, TimerBase*> TimerMap;
    TimerMap m_activeTimers;
#elif USE(COCOA_EVENT_LOOP)
    static void performWork(void*);
    RetainPtr<CFRunLoopRef> m_runLoop;
    RetainPtr<CFRunLoopSourceRef> m_runLoopSource;
#elif USE(GLIB_EVENT_LOOP)
    GRefPtr<GMainContext> m_mainContext;
    Vector<GRefPtr<GMainLoop>> m_mainLoops;
    GRefPtr<GSource> m_source;
#elif USE(GENERIC_EVENT_LOOP)
    void schedule(Ref<TimerBase::ScheduledTask>&&);
    void schedule(const AbstractLocker&, Ref<TimerBase::ScheduledTask>&&);
    void wakeUp(const AbstractLocker&);
    void scheduleAndWakeUp(Ref<TimerBase::ScheduledTask>&&);

    enum class RunMode {
        Iterate,
        Drain
    };

    enum class Status {
        Clear,
        Stopping,
    };
    void runImpl(RunMode);
    bool populateTasks(RunMode, Status&, Deque<Ref<TimerBase::ScheduledTask>>&);

    Lock m_loopLock;
    Condition m_readyToRun;
    Condition m_stopCondition;
    Vector<Ref<TimerBase::ScheduledTask>> m_schedules;
    Vector<Status*> m_mainLoops;
    bool m_shutdown { false };
    bool m_pendingTasks { false };
#endif
};

} // namespace WTF

using WTF::RunLoop;

#endif // RunLoop_h
