/*
 * Copyright (C) 2016 Konstantin Tokavev <annulen@yandex.ru>
 * Copyright (C) 2016 Yusuke Suzuki <utatane.tea@gmail.com>
 * Copyright (C) 2011 Igalia S.L.
 * Copyright (C) 2010 Apple Inc. All rights reserved.
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
#include "WorkQueue.h"

#include <wtf/text/WTFString.h>

void WorkQueue::platformInitialize(const char* name, Type, QOS)
{
    LockHolder locker(m_initializeRunLoopConditionMutex);
    m_workQueueThread = createThread(name, [this] {
        {
            LockHolder locker(m_initializeRunLoopConditionMutex);
            m_runLoop = &RunLoop::current();
            m_initializeRunLoopCondition.notifyOne();
        }
        m_runLoop->run();
    });
    m_initializeRunLoopCondition.wait(m_initializeRunLoopConditionMutex);
}

void WorkQueue::platformInvalidate()
{
    if (m_runLoop)
        m_runLoop->stop();
    if (m_workQueueThread) {
        detachThread(m_workQueueThread);
        m_workQueueThread = 0;
    }
}

void WorkQueue::dispatch(Function<void()>&& function)
{
    RefPtr<WorkQueue> protect(this);
    m_runLoop->dispatch([protect, function = WTFMove(function)] {
        function();
    });
}

void WorkQueue::dispatchAfter(Seconds delay, Function<void()>&& function)
{
    RefPtr<WorkQueue> protect(this);
    m_runLoop->dispatchAfter(delay, [protect, function = WTFMove(function)] {
        function();
    });
}
