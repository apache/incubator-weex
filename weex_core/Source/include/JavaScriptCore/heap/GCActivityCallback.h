/*
 * Copyright (C) 2010, 2016 Apple Inc. All rights reserved.
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

#pragma once

#include "HeapTimer.h"
#include <wtf/RefPtr.h>

#if USE(CF)
#include <CoreFoundation/CoreFoundation.h>
#endif

namespace JSC {

class FullGCActivityCallback;
class Heap;

class JS_EXPORT_PRIVATE GCActivityCallback : public HeapTimer {
public:
    static RefPtr<FullGCActivityCallback> createFullTimer(Heap*);
    static RefPtr<GCActivityCallback> createEdenTimer(Heap*);

    GCActivityCallback(Heap*);

    void doWork() override;

    virtual void doCollection() = 0;

    virtual void didAllocate(size_t);
    virtual void willCollect();
    virtual void cancel();
    bool isEnabled() const { return m_enabled; }
    void setEnabled(bool enabled) { m_enabled = enabled; }

    static bool s_shouldCreateGCTimer;

#if USE(CF) || USE(GLIB)
    double nextFireTime() const { return m_nextFireTime; }
#endif

protected:
    virtual double lastGCLength() = 0;
    virtual double gcTimeSlice(size_t bytes) = 0;
    virtual double deathRate() = 0;

#if USE(CF)
    GCActivityCallback(VM* vm)
        : HeapTimer(vm)
        , m_enabled(true)
        , m_delay(s_decade)
    {
    }
#elif USE(GLIB)
    GCActivityCallback(VM* vm)
        : HeapTimer(vm)
        , m_enabled(true)
        , m_delay(s_decade)
    {
    }
#else
    GCActivityCallback(VM* vm)
        : HeapTimer(vm)
        , m_enabled(true)
    {
    }
#endif

    bool m_enabled;

#if USE(CF) || USE(GLIB)
protected:
    void cancelTimer();
    void scheduleTimer(double);

private:
    double m_delay;
    double m_nextFireTime { 0 };
#endif
};

} // namespace JSC
