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

#include "config.h"
#include "AutomaticThread.h"

#include "DataLog.h"

namespace WTF {

static const bool verbose = false;

RefPtr<AutomaticThreadCondition> AutomaticThreadCondition::create()
{
    return adoptRef(new AutomaticThreadCondition());
}

AutomaticThreadCondition::AutomaticThreadCondition()
{
}

AutomaticThreadCondition::~AutomaticThreadCondition()
{
}

void AutomaticThreadCondition::notifyOne(const AbstractLocker& locker)
{
    for (AutomaticThread* thread : m_threads) {
        if (thread->isWaiting(locker)) {
            thread->notify(locker);
            return;
        }
    }

    for (AutomaticThread* thread : m_threads) {
        if (!thread->hasUnderlyingThread(locker)) {
            thread->start(locker);
            return;
        }
    }

    m_condition.notifyOne();
}

void AutomaticThreadCondition::notifyAll(const AbstractLocker& locker)
{
    m_condition.notifyAll();

    for (AutomaticThread* thread : m_threads) {
        if (thread->isWaiting(locker))
            thread->notify(locker);
        else if (!thread->hasUnderlyingThread(locker))
            thread->start(locker);
    }
}

void AutomaticThreadCondition::wait(Lock& lock)
{
    m_condition.wait(lock);
}

void AutomaticThreadCondition::add(const AbstractLocker&, AutomaticThread* thread)
{
    ASSERT(!m_threads.contains(thread));
    m_threads.append(thread);
}

void AutomaticThreadCondition::remove(const AbstractLocker&, AutomaticThread* thread)
{
    m_threads.removeFirst(thread);
    ASSERT(!m_threads.contains(thread));
}

bool AutomaticThreadCondition::contains(const AbstractLocker&, AutomaticThread* thread)
{
    return m_threads.contains(thread);
}

AutomaticThread::AutomaticThread(const AbstractLocker& locker, Box<Lock> lock, RefPtr<AutomaticThreadCondition> condition)
    : m_lock(lock)
    , m_condition(condition)
{
    if (verbose)
        dataLog(RawPointer(this), ": Allocated AutomaticThread.\n");
    m_condition->add(locker, this);
}

AutomaticThread::~AutomaticThread()
{
    if (verbose)
        dataLog(RawPointer(this), ": Deleting AutomaticThread.\n");
    LockHolder locker(*m_lock);
    
    // It's possible that we're in a waiting state with the thread shut down. This is a goofy way to
    // die, but it could happen.
    m_condition->remove(locker, this);
}

bool AutomaticThread::tryStop(const AbstractLocker&)
{
    if (!m_isRunning)
        return true;
    if (m_hasUnderlyingThread)
        return false;
    m_isRunning = false;
    return true;
}

bool AutomaticThread::isWaiting(const AbstractLocker& locker)
{
    return hasUnderlyingThread(locker) && m_isWaiting;
}

bool AutomaticThread::notify(const AbstractLocker& locker)
{
    ASSERT_UNUSED(locker, hasUnderlyingThread(locker));
    m_isWaiting = false;
    return m_waitCondition.notifyOne();
}

void AutomaticThread::join()
{
    LockHolder locker(*m_lock);
    while (m_isRunning)
        m_isRunningCondition.wait(*m_lock);
}

void AutomaticThread::start(const AbstractLocker&)
{
    RELEASE_ASSERT(m_isRunning);
    
    RefPtr<AutomaticThread> preserveThisForThread = this;
    
    m_hasUnderlyingThread = true;
    
    ThreadIdentifier thread = createThread(
        "WTF::AutomaticThread",
        [=] () {
            if (verbose)
                dataLog(RawPointer(this), ": Running automatic thread!\n");
            
            RefPtr<AutomaticThread> thread = preserveThisForThread;
            thread->threadDidStart();
            
            if (!ASSERT_DISABLED) {
                LockHolder locker(*m_lock);
                ASSERT(m_condition->contains(locker, this));
            }
            
            auto stopImpl = [&] (const AbstractLocker& locker) {
                thread->threadIsStopping(locker);
                thread->m_hasUnderlyingThread = false;
            };
            
            auto stopPermanently = [&] (const AbstractLocker& locker) {
                m_isRunning = false;
                m_isRunningCondition.notifyAll();
                stopImpl(locker);
            };
            
            auto stopForTimeout = [&] (const AbstractLocker& locker) {
                stopImpl(locker);
            };
            
            for (;;) {
                {
                    LockHolder locker(*m_lock);
                    for (;;) {
                        PollResult result = poll(locker);
                        if (result == PollResult::Work)
                            break;
                        if (result == PollResult::Stop)
                            return stopPermanently(locker);
                        RELEASE_ASSERT(result == PollResult::Wait);
                        // Shut the thread down after one second.
                        m_isWaiting = true;
                        bool awokenByNotify =
                            m_waitCondition.waitFor(*m_lock, 1_s);
                        if (verbose && !awokenByNotify && !m_isWaiting)
                            dataLog(RawPointer(this), ": waitFor timed out, but notified via m_isWaiting flag!\n");
                        if (m_isWaiting) {
                            m_isWaiting = false;
                            if (verbose)
                                dataLog(RawPointer(this), ": Going to sleep!\n");
                            // It's important that we don't release the lock until we have completely
                            // indicated that the thread is kaput. Otherwise we'll have a a notify
                            // race that manifests as a deadlock on VM shutdown.
                            return stopForTimeout(locker);
                        }
                    }
                }
                
                WorkResult result = work();
                if (result == WorkResult::Stop) {
                    LockHolder locker(*m_lock);
                    return stopPermanently(locker);
                }
                RELEASE_ASSERT(result == WorkResult::Continue);
            }
        });
    detachThread(thread);
}

void AutomaticThread::threadDidStart()
{
}

void AutomaticThread::threadIsStopping(const AbstractLocker&)
{
}

} // namespace WTF

