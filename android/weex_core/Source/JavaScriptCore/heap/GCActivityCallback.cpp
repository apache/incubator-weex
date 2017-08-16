/*
 * Copyright (C) 2010-2017 Apple Inc. All rights reserved.
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
#include "GCActivityCallback.h"

#include "HeapInlines.h"
#include "JSLock.h"
#include "JSObject.h"
#include "VM.h"

#if USE(GLIB)
#include <glib.h>
#endif

namespace JSC {

bool GCActivityCallback::s_shouldCreateGCTimer = true;

#if USE(CF) || USE(GLIB)

const double timerSlop = 2.0; // Fudge factor to avoid performance cost of resetting timer.

#if USE(CF)
GCActivityCallback::GCActivityCallback(Heap* heap)
    : GCActivityCallback(heap->vm())
{
}
#elif USE(GLIB)
GCActivityCallback::GCActivityCallback(Heap* heap)
    : GCActivityCallback(heap->vm())
{
    g_source_set_ready_time(m_timer.get(), g_get_monotonic_time() + s_decade * G_USEC_PER_SEC);
}
#endif

void GCActivityCallback::doWork()
{
    Heap* heap = &m_vm->heap;
    if (!isEnabled())
        return;
    
    JSLockHolder locker(m_vm);
    if (heap->isDeferred()) {
        scheduleTimer(0);
        return;
    }

    doCollection();
}

#if USE(CF)
void GCActivityCallback::scheduleTimer(double newDelay)
{
    if (newDelay * timerSlop > m_delay)
        return;
    double delta = m_delay - newDelay;
    m_delay = newDelay;
    m_nextFireTime = WTF::currentTime() + newDelay;
    CFRunLoopTimerSetNextFireDate(m_timer.get(), CFRunLoopTimerGetNextFireDate(m_timer.get()) - delta);
}

void GCActivityCallback::cancelTimer()
{
    m_delay = s_decade;
    m_nextFireTime = 0;
    CFRunLoopTimerSetNextFireDate(m_timer.get(), CFAbsoluteTimeGetCurrent() + s_decade);
}
#elif USE(GLIB)
void GCActivityCallback::scheduleTimer(double newDelay)
{
    ASSERT(newDelay >= 0);
    if (newDelay * timerSlop > m_delay)
        return;

    double delta = m_delay - newDelay;
    m_delay = newDelay;
    m_nextFireTime = WTF::currentTime() + newDelay;

    gint64 readyTime = g_source_get_ready_time(m_timer.get());
    g_source_set_ready_time(m_timer.get(), readyTime - delta * G_USEC_PER_SEC);
}

void GCActivityCallback::cancelTimer()
{
    m_delay = s_decade;
    m_nextFireTime = 0;
    g_source_set_ready_time(m_timer.get(), g_get_monotonic_time() + s_decade * G_USEC_PER_SEC);
}
#endif

void GCActivityCallback::didAllocate(size_t bytes)
{
    // The first byte allocated in an allocation cycle will report 0 bytes to didAllocate. 
    // We pretend it's one byte so that we don't ignore this allocation entirely.
    if (!bytes)
        bytes = 1;
    double bytesExpectedToReclaim = static_cast<double>(bytes) * deathRate();
    double newDelay = lastGCLength() / gcTimeSlice(bytesExpectedToReclaim);
    scheduleTimer(newDelay);
}

void GCActivityCallback::willCollect()
{
    cancelTimer();
}

void GCActivityCallback::cancel()
{
    cancelTimer();
}

#else

GCActivityCallback::GCActivityCallback(Heap* heap)
    : GCActivityCallback(heap->vm())
{
}

void GCActivityCallback::doWork()
{
}

void GCActivityCallback::didAllocate(size_t)
{
}

void GCActivityCallback::willCollect()
{
}

void GCActivityCallback::cancel()
{
}

#endif

}

