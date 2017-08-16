/*
 * Copyright (C) 2011-2017 Apple Inc. All Rights Reserved.
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

#include <wtf/MemoryFootprint.h>

#define LOG_CHANNEL_PREFIX Log

namespace WTF {

#if RELEASE_LOG_DISABLED
WTFLogChannel LogMemoryPressure = { WTFLogChannelOn, "MemoryPressure" };
#else
WTFLogChannel LogMemoryPressure = { WTFLogChannelOn, "MemoryPressure", LOG_CHANNEL_WEBKIT_SUBSYSTEM, OS_LOG_DEFAULT };
#endif

WTF_EXPORT_PRIVATE bool MemoryPressureHandler::ReliefLogger::s_loggingEnabled = false;

MemoryPressureHandler& MemoryPressureHandler::singleton()
{
    static NeverDestroyed<MemoryPressureHandler> memoryPressureHandler;
    return memoryPressureHandler;
}

MemoryPressureHandler::MemoryPressureHandler()
#if OS(LINUX)
    : m_holdOffTimer(RunLoop::main(), this, &MemoryPressureHandler::holdOffTimerFired)
#elif OS(WINDOWS)
    : m_windowsMeasurementTimer(RunLoop::main(), this, &MemoryPressureHandler::windowsMeasurementTimerFired)
#endif
{
}

void MemoryPressureHandler::setShouldUsePeriodicMemoryMonitor(bool use)
{
    if (use) {
        m_measurementTimer = std::make_unique<RunLoop::Timer<MemoryPressureHandler>>(RunLoop::main(), this, &MemoryPressureHandler::measurementTimerFired);
        m_measurementTimer->startRepeating(30);
    } else
        m_measurementTimer = nullptr;
}

#if !RELEASE_LOG_DISABLED
static const char* toString(MemoryUsagePolicy policy)
{
    switch (policy) {
    case MemoryUsagePolicy::Unrestricted: return "Unrestricted";
    case MemoryUsagePolicy::Conservative: return "Conservative";
    case MemoryUsagePolicy::Strict: return "Strict";
    case MemoryUsagePolicy::Panic: return "Panic";
    }
}
#endif

static size_t thresholdForPolicy(MemoryUsagePolicy policy)
{
    switch (policy) {
    case MemoryUsagePolicy::Conservative:
        return 1 * GB;
    case MemoryUsagePolicy::Strict:
        return 2 * GB;
    case MemoryUsagePolicy::Panic:
#if CPU(X86_64) || CPU(ARM64)
        return 4 * GB;
#else
        return 3 * GB;
#endif
    case MemoryUsagePolicy::Unrestricted:
    default:
        ASSERT_NOT_REACHED();
        return 0;
    }
}

static MemoryUsagePolicy policyForFootprint(size_t footprint)
{
    if (footprint >= thresholdForPolicy(MemoryUsagePolicy::Panic))
        return MemoryUsagePolicy::Panic;
    if (footprint >= thresholdForPolicy(MemoryUsagePolicy::Strict))
        return MemoryUsagePolicy::Strict;
    if (footprint >= thresholdForPolicy(MemoryUsagePolicy::Conservative))
        return MemoryUsagePolicy::Conservative;
    return MemoryUsagePolicy::Unrestricted;
}

void MemoryPressureHandler::measurementTimerFired()
{
    auto footprint = memoryFootprint();
    if (!footprint)
        return;

    RELEASE_LOG(MemoryPressure, "Current memory footprint: %lu MB", footprint.value() / MB);

    auto newPolicy = policyForFootprint(footprint.value());
    if (newPolicy == m_memoryUsagePolicy) {
        if (m_memoryUsagePolicy != MemoryUsagePolicy::Panic)
            return;
        RELEASE_LOG(MemoryPressure, "Memory usage still above panic threshold");
    } else
        RELEASE_LOG(MemoryPressure, "Memory usage policy changed: %s -> %s", toString(m_memoryUsagePolicy), toString(newPolicy));

    m_memoryUsagePolicy = newPolicy;
    memoryPressureStatusChanged();

    if (newPolicy == MemoryUsagePolicy::Unrestricted)
        return;

    if (newPolicy == MemoryUsagePolicy::Conservative) {
        // FIXME: Implement this policy by choosing which caches should respect it, and hooking them up.
        return;
    }

    if (newPolicy == MemoryUsagePolicy::Strict) {
        RELEASE_LOG(MemoryPressure, "Attempting to reduce memory footprint by freeing less important objects.");
        releaseMemory(Critical::No, Synchronous::No);
        return;
    }

    RELEASE_ASSERT(newPolicy == MemoryUsagePolicy::Panic);

    RELEASE_LOG(MemoryPressure, "Attempting to reduce memory footprint by freeing more important objects.");
    if (m_processIsEligibleForMemoryKillCallback) {
        if (!m_processIsEligibleForMemoryKillCallback()) {
            releaseMemory(Critical::Yes, Synchronous::No);
            return;
        }
    }

    releaseMemory(Critical::Yes, Synchronous::Yes);

    // Remeasure footprint to see how well the pressure handler did.
    footprint = memoryFootprint();
    RELEASE_ASSERT(footprint);

    RELEASE_LOG(MemoryPressure, "New memory footprint: %lu MB", footprint.value() / MB);
    if (footprint.value() < thresholdForPolicy(MemoryUsagePolicy::Panic)) {
        m_memoryUsagePolicy = policyForFootprint(footprint.value());
        RELEASE_LOG(MemoryPressure, "Pressure reduced below panic threshold. New memory usage policy: %s", toString(m_memoryUsagePolicy));
        memoryPressureStatusChanged();
        return;
    }

    if (m_memoryKillCallback)
        m_memoryKillCallback();
}

void MemoryPressureHandler::beginSimulatedMemoryPressure()
{
    if (m_isSimulatingMemoryPressure)
        return;
    m_isSimulatingMemoryPressure = true;
    memoryPressureStatusChanged();
    respondToMemoryPressure(Critical::Yes, Synchronous::Yes);
}

void MemoryPressureHandler::endSimulatedMemoryPressure()
{
    if (!m_isSimulatingMemoryPressure)
        return;
    m_isSimulatingMemoryPressure = false;
    memoryPressureStatusChanged();
}

void MemoryPressureHandler::releaseMemory(Critical critical, Synchronous synchronous)
{
    if (!m_lowMemoryHandler)
        return;

    ReliefLogger log("Total");
    m_lowMemoryHandler(critical, synchronous);
    platformReleaseMemory(critical);
}

void MemoryPressureHandler::setUnderMemoryPressure(bool underMemoryPressure)
{
    if (m_underMemoryPressure == underMemoryPressure)
        return;
    m_underMemoryPressure = underMemoryPressure;
    memoryPressureStatusChanged();
}

void MemoryPressureHandler::memoryPressureStatusChanged()
{
    if (m_memoryPressureStatusChangedCallback)
        m_memoryPressureStatusChangedCallback(isUnderMemoryPressure());
}

void MemoryPressureHandler::ReliefLogger::logMemoryUsageChange()
{
#if !RELEASE_LOG_DISABLED
#define STRING_SPECIFICATION "%{public}s"
#define MEMORYPRESSURE_LOG(...) RELEASE_LOG(MemoryPressure, __VA_ARGS__)
#else
#define STRING_SPECIFICATION "%s"
#define MEMORYPRESSURE_LOG(...) WTFLogAlways(__VA_ARGS__)
#endif

    auto currentMemory = platformMemoryUsage();
    if (!currentMemory || !m_initialMemory) {
        MEMORYPRESSURE_LOG("Memory pressure relief: " STRING_SPECIFICATION ": (Unable to get dirty memory information for process)", m_logString);
        return;
    }

    long residentDiff = currentMemory->resident - m_initialMemory->resident;
    long physicalDiff = currentMemory->physical - m_initialMemory->physical;

    MEMORYPRESSURE_LOG("Memory pressure relief: " STRING_SPECIFICATION ": res = %zu/%zu/%ld, res+swap = %zu/%zu/%ld",
        m_logString,
        m_initialMemory->resident, currentMemory->resident, residentDiff,
        m_initialMemory->physical, currentMemory->physical, physicalDiff);
}

#if !PLATFORM(COCOA) && !OS(LINUX) && !PLATFORM(WIN)
void MemoryPressureHandler::install() { }
void MemoryPressureHandler::uninstall() { }
void MemoryPressureHandler::holdOff(unsigned) { }
void MemoryPressureHandler::respondToMemoryPressure(Critical, Synchronous) { }
void MemoryPressureHandler::platformReleaseMemory(Critical) { }
std::optional<MemoryPressureHandler::ReliefLogger::MemoryUsage> MemoryPressureHandler::ReliefLogger::platformMemoryUsage() { return std::nullopt; }
#endif

#if !PLATFORM(WIN)
void MemoryPressureHandler::platformInitialize() { }
#endif

} // namespace WebCore
