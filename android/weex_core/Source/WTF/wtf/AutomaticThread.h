/*
 * Copyright (C) 2016-2017 Apple Inc. All rights reserved.
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

#ifndef WTF_AutomaticThread_h
#define WTF_AutomaticThread_h

#include <wtf/Box.h>
#include <wtf/Condition.h>
#include <wtf/Lock.h>
#include <wtf/Ref.h>
#include <wtf/ThreadSafeRefCounted.h>
#include <wtf/Threading.h>
#include <wtf/Vector.h>

namespace WTF {

// Often, we create threads that have this as their body:
//
//     for (;;) {
//         {
//             LockHolder locker(m_lock);
//             for (;;) {
//  [1]            stuff that could break, return, or fall through;
//                 m_condition.wait(m_lock);
//             }
//         }
//         
//  [2]    do work;
//     }
//
// When we do this, we don't always do a good job of managing this thread's lifetime, which may lead
// to this thread sitting around even when it is not needed.
//
// AutomaticThread is here to help you in these situations. It encapsulates a lock, a condition
// variable, and a thread. It will automatically shut the thread down after 1 second of inactivity.
// You use AutomaticThread by subclassing it, and put any state that is needed between [1] and [2]
// in the subclass.
//
// The terminology we use is:
//
// [1] PollResult AutomaticThread::poll()
// [2] WordResult AutomaticThread::work()
//
// Note that poll() and work() may not be called on the same thread every time, since this will shut
// down the thread as necessary. This is legal since m_condition.wait(m_lock) can drop the lock, and
// so there is no reason to keep the thread around.

class AutomaticThread;

class AutomaticThreadCondition : public ThreadSafeRefCounted<AutomaticThreadCondition> {
public:
    static WTF_EXPORT_PRIVATE RefPtr<AutomaticThreadCondition> create();
    
    WTF_EXPORT_PRIVATE ~AutomaticThreadCondition();
    
    WTF_EXPORT_PRIVATE void notifyOne(const AbstractLocker&);
    WTF_EXPORT_PRIVATE void notifyAll(const AbstractLocker&);
    
    // You can reuse this condition for other things, just as you would any other condition.
    // However, since conflating conditions could lead to thundering herd, it's best to avoid it.
    // One known-good case for one-true-condition is when the communication involves just two
    // threads. In such cases, the thread doing the notifyAll() can wake up at most one thread -
    // its partner.
    WTF_EXPORT_PRIVATE void wait(Lock&);
    
private:
    friend class AutomaticThread;
    
    WTF_EXPORT_PRIVATE AutomaticThreadCondition();

    void add(const AbstractLocker&, AutomaticThread*);
    void remove(const AbstractLocker&, AutomaticThread*);
    bool contains(const AbstractLocker&, AutomaticThread*);
    
    Condition m_condition;
    Vector<AutomaticThread*> m_threads;
};

class WTF_EXPORT_PRIVATE AutomaticThread : public ThreadSafeRefCounted<AutomaticThread> {
public:
    // Note that if you drop all of your references to an AutomaticThread then as soon as there is a
    // second during which it doesn't get woken up, it will simply die on its own. This is a
    // permanent kind of death where the AutomaticThread object goes away, rather than the temporary
    // kind of death where AutomaticThread lives but its underlying thread dies. All you have to do
    // to prevent permanent death is keep a ref to AutomaticThread. At time of writing, every user of
    // AutomaticThread keeps a ref to it and does join() as part of the shutdown process, so only the
    // temporary kind of automatic death happens in practice. We keep the permanent death feature
    // because it leads to an easy-to-understand reference counting discipline (AutomaticThread holds
    // strong ref to AutomaticThreadCondition and the underlying thread holds a strong ref to
    // AutomaticThread).
    virtual ~AutomaticThread();
    
    // Sometimes it's possible to optimize for the case that there is no underlying thread.
    bool hasUnderlyingThread(const AbstractLocker&) const { return m_hasUnderlyingThread; }
    
    // This attempts to quickly stop the thread. This will succeed if the thread happens to not be
    // running. Returns true if the thread has been stopped. A good idiom for stopping your automatic
    // thread is to first try this, and if that doesn't work, to tell the thread using your own
    // mechanism (set some flag and then notify the condition).
    bool tryStop(const AbstractLocker&);

    bool isWaiting(const AbstractLocker&);

    bool notify(const AbstractLocker&);

    void join();
    
protected:
    // This logically creates the thread, but in reality the thread won't be created until someone
    // calls AutomaticThreadCondition::notifyOne() or notifyAll().
    AutomaticThread(const AbstractLocker&, Box<Lock>, RefPtr<AutomaticThreadCondition>);
    
    // To understand PollResult and WorkResult, imagine that poll() and work() are being called like
    // so:
    //
    // void AutomaticThread::runThread()
    // {
    //     for (;;) {
    //         {
    //             LockHolder locker(m_lock);
    //             for (;;) {
    //                 PollResult result = poll();
    //                 if (result == PollResult::Work)
    //                     break;
    //                 if (result == PollResult::Stop)
    //                     return;
    //                 RELEASE_ASSERT(result == PollResult::Wait);
    //                 m_condition.wait(m_lock);
    //             }
    //         }
    //         
    //         WorkResult result = work();
    //         if (result == WorkResult::Stop)
    //             return;
    //         RELEASE_ASSERT(result == WorkResult::Continue);
    //     }
    // }
    
    enum class PollResult { Work, Stop, Wait };
    virtual PollResult poll(const AbstractLocker&) = 0;
    
    enum class WorkResult { Continue, Stop };
    virtual WorkResult work() = 0;
    
    // It's sometimes useful to allocate resources while the thread is running, and to destroy them
    // when the thread dies. These methods let you do this. You can override these methods, and you
    // can be sure that the default ones don't do anything (so you don't need a super call).
    virtual void threadDidStart();
    virtual void threadIsStopping(const AbstractLocker&);
    
private:
    friend class AutomaticThreadCondition;
    
    void start(const AbstractLocker&);
    
    Box<Lock> m_lock;
    RefPtr<AutomaticThreadCondition> m_condition;
    bool m_isRunning { true };
    bool m_isWaiting { false };
    bool m_hasUnderlyingThread { false };
    Condition m_waitCondition;
    Condition m_isRunningCondition;
};

} // namespace WTF

using WTF::AutomaticThread;
using WTF::AutomaticThreadCondition;

#endif // WTF_AutomaticThread_h

