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

#include <atomic>
#include <ctime>
#include <functional>
#include <wtf/FastMalloc.h>
#include <wtf/Forward.h>
#include <wtf/NeverDestroyed.h>
#include <wtf/Optional.h>
#include <wtf/RunLoop.h>

#if USE(GLIB)
#include <wtf/glib/GRefPtr.h>
#endif

#if PLATFORM(WIN)
#include <wtf/win/Win32Handle.h>
#endif

namespace WTF {

enum class MemoryUsagePolicy {
    Unrestricted, // Allocate as much as you want
    Conservative, // Maybe you don't cache every single thing
    Strict, // Time to start pinching pennies for real
    Panic, // OH GOD WE'RE SINKING, THROW EVERYTHING OVERBOARD
};

enum class Critical { No, Yes };
enum class Synchronous { No, Yes };

typedef std::function<void(Critical, Synchronous)> LowMemoryHandler;

class MemoryPressureHandler {
    friend class WTF::NeverDestroyed<MemoryPressureHandler>;
public:
    WTF_EXPORT_PRIVATE static MemoryPressureHandler& singleton();

    WTF_EXPORT_PRIVATE void install();

    WTF_EXPORT_PRIVATE void setShouldUsePeriodicMemoryMonitor(bool);

    void setMemoryKillCallback(WTF::Function<void()> function) { m_memoryKillCallback = WTFMove(function); }
    void setProcessIsEligibleForMemoryKillCallback(WTF::Function<bool()> function) { m_processIsEligibleForMemoryKillCallback = WTFMove(function); }
    void setMemoryPressureStatusChangedCallback(WTF::Function<void(bool)> function) { m_memoryPressureStatusChangedCallback = WTFMove(function); }

    void setLowMemoryHandler(LowMemoryHandler&& handler)
    {
        m_lowMemoryHandler = WTFMove(handler);
    }

    bool isUnderMemoryPressure() const
    {
        return m_underMemoryPressure
#if PLATFORM(MAC)
            || m_memoryUsagePolicy >= MemoryUsagePolicy::Strict
#endif
            || m_isSimulatingMemoryPressure;
    }
    void setUnderMemoryPressure(bool);

#if OS(LINUX)
    void setMemoryPressureMonitorHandle(int fd);
#endif

    class ReliefLogger {
    public:
        explicit ReliefLogger(const char *log)
            : m_logString(log)
            , m_initialMemory(loggingEnabled() ? platformMemoryUsage() : MemoryUsage { })
        {
        }

        ~ReliefLogger()
        {
            if (loggingEnabled())
                logMemoryUsageChange();
        }


        const char* logString() const { return m_logString; }
        static void setLoggingEnabled(bool enabled) { s_loggingEnabled = enabled; }
        static bool loggingEnabled()
        {
#if RELEASE_LOG_DISABLED
            return s_loggingEnabled;
#else
            return true;
#endif
        }

    private:
        struct MemoryUsage {
            MemoryUsage() = default;
            MemoryUsage(size_t resident, size_t physical)
                : resident(resident)
                , physical(physical)
            {
            }
            size_t resident { 0 };
            size_t physical { 0 };
        };
        std::optional<MemoryUsage> platformMemoryUsage();
        void logMemoryUsageChange();

        const char* m_logString;
        std::optional<MemoryUsage> m_initialMemory;

        WTF_EXPORT_PRIVATE static bool s_loggingEnabled;
    };

    WTF_EXPORT_PRIVATE void releaseMemory(Critical, Synchronous = Synchronous::No);

    WTF_EXPORT_PRIVATE void beginSimulatedMemoryPressure();
    WTF_EXPORT_PRIVATE void endSimulatedMemoryPressure();

private:
    void memoryPressureStatusChanged();

    void uninstall();

    void holdOff(unsigned);

    MemoryPressureHandler();
    ~MemoryPressureHandler() = delete;

    void respondToMemoryPressure(Critical, Synchronous = Synchronous::No);
    void platformReleaseMemory(Critical);
    void platformInitialize();

    NO_RETURN_DUE_TO_CRASH void didExceedMemoryLimitAndFailedToRecover();
    void measurementTimerFired();

#if OS(LINUX)
    class EventFDPoller {
        WTF_MAKE_NONCOPYABLE(EventFDPoller); WTF_MAKE_FAST_ALLOCATED;
    public:
        EventFDPoller(int fd, std::function<void ()>&& notifyHandler);
        ~EventFDPoller();

    private:
        void readAndNotify() const;

        std::optional<int> m_fd;
        std::function<void ()> m_notifyHandler;
#if USE(GLIB)
        GRefPtr<GSource> m_source;
#else
        ThreadIdentifier m_threadID;
#endif
    };
#endif

    bool m_installed { false };
    LowMemoryHandler m_lowMemoryHandler;

    std::atomic<bool> m_underMemoryPressure;
    bool m_isSimulatingMemoryPressure { false };

    std::unique_ptr<RunLoop::Timer<MemoryPressureHandler>> m_measurementTimer;
    MemoryUsagePolicy m_memoryUsagePolicy { MemoryUsagePolicy::Unrestricted };
    WTF::Function<void()> m_memoryKillCallback;
    WTF::Function<bool()> m_processIsEligibleForMemoryKillCallback;
    WTF::Function<void(bool)> m_memoryPressureStatusChangedCallback;

#if OS(WINDOWS)
    void windowsMeasurementTimerFired();
    RunLoop::Timer<MemoryPressureHandler> m_windowsMeasurementTimer;
    Win32Handle m_lowMemoryHandle;
#endif

#if OS(LINUX)
    std::optional<int> m_eventFD;
    std::optional<int> m_pressureLevelFD;
    std::unique_ptr<EventFDPoller> m_eventFDPoller;
    RunLoop::Timer<MemoryPressureHandler> m_holdOffTimer;
    void holdOffTimerFired();
    void logErrorAndCloseFDs(const char* error);
    bool tryEnsureEventFD();
#endif
};

extern WTFLogChannel LogMemoryPressure;

} // namespace WTF

using WTF::Critical;
using WTF::MemoryPressureHandler;
using WTF::Synchronous;
