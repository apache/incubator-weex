/*
 * Copyright (C) 2011, 2012 Apple Inc. All Rights Reserved.
 * Copyright (C) 2014 Raspberry Pi Foundation. All Rights Reserved.
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
#include "MemoryPressureHandler.h"

#if OS(LINUX)

#include <errno.h>
#include <fcntl.h>
#include <malloc.h>
#include <sys/eventfd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <wtf/CurrentTime.h>
#include <wtf/MainThread.h>
#include <wtf/linux/CurrentProcessMemoryStatus.h>
#include <wtf/text/WTFString.h>

#if USE(GLIB)
#include <glib-unix.h>
#endif

#define LOG_CHANNEL_PREFIX Log

namespace WTF {

// Disable memory event reception for a minimum of s_minimumHoldOffTime
// seconds after receiving an event. Don't let events fire any sooner than
// s_holdOffMultiplier times the last cleanup processing time. Effectively
// this is 1 / s_holdOffMultiplier percent of the time.
// If after releasing the memory we don't free at least s_minimumBytesFreedToUseMinimumHoldOffTime,
// we wait longer to try again (s_maximumHoldOffTime).
// These value seems reasonable and testing verifies that it throttles frequent
// low memory events, greatly reducing CPU usage.
static const unsigned s_minimumHoldOffTime = 5;
static const unsigned s_maximumHoldOffTime = 30;
static const size_t s_minimumBytesFreedToUseMinimumHoldOffTime = 1 * MB;
static const unsigned s_holdOffMultiplier = 20;

static const char* s_cgroupMemoryPressureLevel = "/sys/fs/cgroup/memory/memory.pressure_level";
static const char* s_cgroupEventControl = "/sys/fs/cgroup/memory/cgroup.event_control";

#if USE(GLIB)
typedef struct {
    GSource source;
    gpointer fdTag;
    GIOCondition condition;
} EventFDSource;

static const unsigned eventFDSourceCondition = G_IO_IN | G_IO_HUP | G_IO_ERR | G_IO_NVAL;

static GSourceFuncs eventFDSourceFunctions = {
    nullptr, // prepare
    nullptr, // check
    // dispatch
    [](GSource* source, GSourceFunc callback, gpointer userData) -> gboolean
    {
        EventFDSource* eventFDSource = reinterpret_cast<EventFDSource*>(source);
        unsigned events = g_source_query_unix_fd(source, eventFDSource->fdTag) & eventFDSourceCondition;
        if (events & G_IO_HUP || events & G_IO_ERR || events & G_IO_NVAL)
            return G_SOURCE_REMOVE;

        gboolean returnValue = G_SOURCE_CONTINUE;
        if (events & G_IO_IN)
            returnValue = callback(userData);
        g_source_set_ready_time(source, -1);
        return returnValue;
    },
    nullptr, // finalize
    nullptr, // closure_callback
    nullptr, // closure_marshall
};
#endif

MemoryPressureHandler::EventFDPoller::EventFDPoller(int fd, std::function<void ()>&& notifyHandler)
    : m_fd(fd)
    , m_notifyHandler(WTFMove(notifyHandler))
{
#if USE(GLIB)
    m_source = adoptGRef(g_source_new(&eventFDSourceFunctions, sizeof(EventFDSource)));
    g_source_set_name(m_source.get(), "WTF: MemoryPressureHandler");
    if (!g_unix_set_fd_nonblocking(m_fd.value(), TRUE, nullptr)) {
        LOG(MemoryPressure, "Failed to set eventfd nonblocking");
        return;
    }

    EventFDSource* eventFDSource = reinterpret_cast<EventFDSource*>(m_source.get());
    eventFDSource->fdTag = g_source_add_unix_fd(m_source.get(), m_fd.value(), static_cast<GIOCondition>(eventFDSourceCondition));
    g_source_set_callback(m_source.get(), [](gpointer userData) -> gboolean {
        static_cast<EventFDPoller*>(userData)->readAndNotify();
        return G_SOURCE_REMOVE;
    }, this, nullptr);
    g_source_attach(m_source.get(), nullptr);
#else
    m_threadID = createThread("WTF: MemoryPressureHandler", [this] { readAndNotify(); }
#endif
}

MemoryPressureHandler::EventFDPoller::~EventFDPoller()
{
    m_fd = std::nullopt;
#if USE(GLIB)
    g_source_destroy(m_source.get());
#else
    detachThread(m_threadID);
#endif
}

static inline bool isFatalReadError(int error)
{
#if USE(GLIB)
    // We don't really need to read the buffer contents, if the poller
    // notified us, but read would block or is no longer available, is
    // enough to trigger the memory pressure handler.
    return error != EAGAIN && error != EWOULDBLOCK;
#else
    return true;
#endif
}

void MemoryPressureHandler::EventFDPoller::readAndNotify() const
{
    if (!m_fd) {
        LOG(MemoryPressure, "Invalidate eventfd.");
        return;
    }

    uint64_t buffer;
    if (read(m_fd.value(), &buffer, sizeof(buffer)) == -1) {
        if (isFatalReadError(errno)) {
            LOG(MemoryPressure, "Failed to read eventfd.");
            return;
        }
    }

    m_notifyHandler();
}

inline void MemoryPressureHandler::logErrorAndCloseFDs(const char* log)
{
    if (log)
        LOG(MemoryPressure, "%s, error : %m", log);

    if (m_eventFD) {
        close(m_eventFD.value());
        m_eventFD = std::nullopt;
    }
    if (m_pressureLevelFD) {
        close(m_pressureLevelFD.value());
        m_pressureLevelFD = std::nullopt;
    }
}

bool MemoryPressureHandler::tryEnsureEventFD()
{
    if (m_eventFD)
        return true;

    // Try to use cgroups instead.
    int fd = eventfd(0, EFD_CLOEXEC);
    if (fd == -1) {
        LOG(MemoryPressure, "eventfd() failed: %m");
        return false;
    }
    m_eventFD = fd;

    fd = open(s_cgroupMemoryPressureLevel, O_CLOEXEC | O_RDONLY);
    if (fd == -1) {
        logErrorAndCloseFDs("Failed to open memory.pressure_level");
        return false;
    }
    m_pressureLevelFD = fd;

    fd = open(s_cgroupEventControl, O_CLOEXEC | O_WRONLY);
    if (fd == -1) {
        logErrorAndCloseFDs("Failed to open cgroup.event_control");
        return false;
    }

    char line[128] = {0, };
    if (snprintf(line, sizeof(line), "%d %d low", m_eventFD.value(), m_pressureLevelFD.value()) < 0
        || write(fd, line, strlen(line) + 1) < 0) {
        logErrorAndCloseFDs("Failed to write cgroup.event_control");
        close(fd);
        return false;
    }
    close(fd);

    return true;
}

void MemoryPressureHandler::install()
{
    if (m_installed || m_holdOffTimer.isActive())
        return;

    if (!tryEnsureEventFD())
        return;

    m_eventFDPoller = std::make_unique<EventFDPoller>(m_eventFD.value(), [this] {
        // FIXME: Current memcg does not provide any way for users to know how serious the memory pressure is.
        // So we assume all notifications from memcg are critical for now. If memcg had better inferfaces
        // to get a detailed memory pressure level in the future, we should update here accordingly.
        bool critical = true;
        if (ReliefLogger::loggingEnabled())
            LOG(MemoryPressure, "Got memory pressure notification (%s)", critical ? "critical" : "non-critical");

        setUnderMemoryPressure(critical);
        if (isMainThread())
            respondToMemoryPressure(critical ? Critical::Yes : Critical::No);
        else
            RunLoop::main().dispatch([this, critical] { respondToMemoryPressure(critical ? Critical::Yes : Critical::No); });
    });

    if (ReliefLogger::loggingEnabled() && isUnderMemoryPressure())
        LOG(MemoryPressure, "System is no longer under memory pressure.");

    setUnderMemoryPressure(false);
    m_installed = true;
}

void MemoryPressureHandler::uninstall()
{
    if (!m_installed)
        return;

    m_holdOffTimer.stop();
    m_eventFDPoller = nullptr;

    if (m_pressureLevelFD) {
        close(m_pressureLevelFD.value());
        m_pressureLevelFD = std::nullopt;

        // Only close the eventFD used for cgroups.
        if (m_eventFD) {
            close(m_eventFD.value());
            m_eventFD = std::nullopt;
        }
    }

    m_installed = false;
}

void MemoryPressureHandler::holdOffTimerFired()
{
    install();
}

void MemoryPressureHandler::holdOff(unsigned seconds)
{
    m_holdOffTimer.startOneShot(seconds);
}

static size_t processMemoryUsage()
{
    ProcessMemoryStatus memoryStatus;
    currentProcessMemoryStatus(memoryStatus);
    return (memoryStatus.resident - memoryStatus.shared);
}

void MemoryPressureHandler::respondToMemoryPressure(Critical critical, Synchronous synchronous)
{
    uninstall();

    double startTime = monotonicallyIncreasingTime();
    int64_t processMemory = processMemoryUsage();
    releaseMemory(critical, synchronous);
    int64_t bytesFreed = processMemory - processMemoryUsage();
    unsigned holdOffTime = s_maximumHoldOffTime;
    if (bytesFreed > 0 && static_cast<size_t>(bytesFreed) >= s_minimumBytesFreedToUseMinimumHoldOffTime)
        holdOffTime = (monotonicallyIncreasingTime() - startTime) * s_holdOffMultiplier;
    holdOff(std::max(holdOffTime, s_minimumHoldOffTime));
}

void MemoryPressureHandler::platformReleaseMemory(Critical)
{
#ifdef __GLIBC__
    malloc_trim(0);
#endif
}

std::optional<MemoryPressureHandler::ReliefLogger::MemoryUsage> MemoryPressureHandler::ReliefLogger::platformMemoryUsage()
{
    return MemoryUsage {processMemoryUsage(), 0};
}

void MemoryPressureHandler::setMemoryPressureMonitorHandle(int fd)
{
    ASSERT(!m_eventFD);
    m_eventFD = fd;
}

} // namespace WTF

#endif // OS(LINUX)
