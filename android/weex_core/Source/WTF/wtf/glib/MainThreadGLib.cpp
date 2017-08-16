/*
 * Copyright (C) 2007, 2008 Apple Inc. All rights reserved.
 * Copyright (C) 2007 Justin Haygood (jhaygood@reaktix.com)
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 * 3.  Neither the name of Apple Inc. ("Apple") nor the names of
 *     its contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "MainThread.h"

#include <glib.h>
#include <wtf/RunLoop.h>

static pthread_t mainThreadPthread;

namespace WTF {

class MainThreadDispatcher {
public:
    MainThreadDispatcher()
        : m_timer(RunLoop::main(), this, &MainThreadDispatcher::fired)
    {
        m_timer.setPriority(G_PRIORITY_HIGH_IDLE + 20);
    }

    void schedule()
    {
        m_timer.startOneShot(0);
    }

private:
    void fired()
    {
        dispatchFunctionsFromMainThread();
    }

    RunLoop::Timer<MainThreadDispatcher> m_timer;
};

void initializeMainThreadPlatform()
{
    mainThreadPthread = pthread_self();
}

bool isMainThread()
{
    ASSERT(mainThreadPthread);
    return pthread_equal(pthread_self(), mainThreadPthread);
}

void scheduleDispatchFunctionsOnMainThread()
{
    // Use a RunLoop::Timer instead of RunLoop::dispatch() to be able to use a different priority and
    // avoid the double queue because dispatchOnMainThread also queues the functions.
    static MainThreadDispatcher dispatcher;
    dispatcher.schedule();
}

} // namespace WTF
