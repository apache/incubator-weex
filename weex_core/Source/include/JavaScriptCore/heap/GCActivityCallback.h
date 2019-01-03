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
