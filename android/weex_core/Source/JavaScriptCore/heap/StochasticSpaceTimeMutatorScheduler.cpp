/*
 * Copyright (C) 2017 Apple Inc. All rights reserved.
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
#include "StochasticSpaceTimeMutatorScheduler.h"

#include "JSCInlines.h"

namespace JSC {

// The scheduler will often make decisions based on state that is in flux. It will be fine so
// long as multiple uses of the same value all see the same value. We wouldn't get this for free,
// since our need to modularize the calculation results in a tendency to access the same mutable
// field in Heap multiple times, and to access the current time multiple times.
class StochasticSpaceTimeMutatorScheduler::Snapshot {
public:
    Snapshot(StochasticSpaceTimeMutatorScheduler& scheduler)
    {
        m_now = MonotonicTime::now();
        m_bytesAllocatedThisCycle = scheduler.bytesAllocatedThisCycleImpl();
    }
    
    MonotonicTime now() const { return m_now; }
    
    double bytesAllocatedThisCycle() const { return m_bytesAllocatedThisCycle; }
    
private:
    MonotonicTime m_now;
    double m_bytesAllocatedThisCycle;
};

StochasticSpaceTimeMutatorScheduler::StochasticSpaceTimeMutatorScheduler(Heap& heap)
    : m_heap(heap)
    , m_minimumPause(Seconds::fromMilliseconds(Options::minimumGCPauseMS()))
    , m_pauseScale(Options::gcPauseScale())
{
}

StochasticSpaceTimeMutatorScheduler::~StochasticSpaceTimeMutatorScheduler()
{
}

MutatorScheduler::State StochasticSpaceTimeMutatorScheduler::state() const
{
    return m_state;
}

void StochasticSpaceTimeMutatorScheduler::beginCollection()
{
    RELEASE_ASSERT(m_state == Normal);
    m_state = Stopped;

    m_bytesAllocatedThisCycleAtTheBeginning = m_heap.m_bytesAllocatedThisCycle;
    m_bytesAllocatedThisCycleAtTheEnd = 
        Options::concurrentGCMaxHeadroom() *
        std::max<double>(m_bytesAllocatedThisCycleAtTheBeginning, m_heap.m_maxEdenSize);
    
    if (Options::logGC())
        dataLog("ca=", m_bytesAllocatedThisCycleAtTheBeginning / 1024, "kb h=", (m_bytesAllocatedThisCycleAtTheEnd - m_bytesAllocatedThisCycleAtTheBeginning) / 1024, "kb ");
    
    m_beforeConstraints = MonotonicTime::now();
}

void StochasticSpaceTimeMutatorScheduler::didStop()
{
    RELEASE_ASSERT(m_state == Stopped || m_state == Resumed);
    m_state = Stopped;
}

void StochasticSpaceTimeMutatorScheduler::willResume()
{
    RELEASE_ASSERT(m_state == Stopped || m_state == Resumed);
    m_state = Resumed;
}

void StochasticSpaceTimeMutatorScheduler::didReachTermination()
{
    m_beforeConstraints = MonotonicTime::now();
}

void StochasticSpaceTimeMutatorScheduler::didExecuteConstraints()
{
    Snapshot snapshot(*this);
    
    Seconds constraintExecutionDuration = snapshot.now() - m_beforeConstraints;
    
    m_targetPause = std::max(
        constraintExecutionDuration * m_pauseScale,
        m_minimumPause);
    
    if (Options::logGC())
        dataLog("tp=", m_targetPause.milliseconds(), "ms ");
    
    m_plannedResumeTime = snapshot.now() + m_targetPause;
}

void StochasticSpaceTimeMutatorScheduler::synchronousDrainingDidStall()
{
    Snapshot snapshot(*this);
    
    double resumeProbability = mutatorUtilization(snapshot);
    if (resumeProbability < Options::epsilonMutatorUtilization()) {
        m_plannedResumeTime = MonotonicTime::infinity();
        return;
    }
    
    bool shouldResume = m_random.get() < resumeProbability;
    
    if (shouldResume) {
        m_plannedResumeTime = snapshot.now();
        return;
    }
    
    m_plannedResumeTime = snapshot.now() + m_targetPause;
}

MonotonicTime StochasticSpaceTimeMutatorScheduler::timeToStop()
{
    switch (m_state) {
    case Normal:
        return MonotonicTime::infinity();
    case Stopped:
        return MonotonicTime::now();
    case Resumed: {
        // Once we're running, we keep going unless we run out of headroom.
        Snapshot snapshot(*this);
        if (mutatorUtilization(snapshot) < Options::epsilonMutatorUtilization())
            return MonotonicTime::now();
        return MonotonicTime::infinity();
    } }
    
    RELEASE_ASSERT_NOT_REACHED();
    return MonotonicTime();
}

MonotonicTime StochasticSpaceTimeMutatorScheduler::timeToResume()
{
    switch (m_state) {
    case Normal:
    case Resumed:
        return MonotonicTime::now();
    case Stopped:
        return m_plannedResumeTime;
    }
    
    RELEASE_ASSERT_NOT_REACHED();
    return MonotonicTime();
}

void StochasticSpaceTimeMutatorScheduler::log()
{
    ASSERT(Options::logGC());
    Snapshot snapshot(*this);
    dataLog(
        "a=", format("%.0lf", bytesSinceBeginningOfCycle(snapshot) / 1024), "kb ",
        "hf=", format("%.3lf", headroomFullness(snapshot)), " ",
        "mu=", format("%.3lf", mutatorUtilization(snapshot)), " ");
}

void StochasticSpaceTimeMutatorScheduler::endCollection()
{
    m_state = Normal;
}

double StochasticSpaceTimeMutatorScheduler::bytesAllocatedThisCycleImpl()
{
    return m_heap.m_bytesAllocatedThisCycle;
}

double StochasticSpaceTimeMutatorScheduler::bytesSinceBeginningOfCycle(const Snapshot& snapshot)
{
    return snapshot.bytesAllocatedThisCycle() - m_bytesAllocatedThisCycleAtTheBeginning;
}

double StochasticSpaceTimeMutatorScheduler::maxHeadroom()
{
    return m_bytesAllocatedThisCycleAtTheEnd - m_bytesAllocatedThisCycleAtTheBeginning;
}

double StochasticSpaceTimeMutatorScheduler::headroomFullness(const Snapshot& snapshot)
{
    double result = bytesSinceBeginningOfCycle(snapshot) / maxHeadroom();

    // headroomFullness can be NaN and other interesting things if
    // bytesAllocatedThisCycleAtTheBeginning is zero. We see that in debug tests. This code
    // defends against all floating point dragons.
    
    if (!(result >= 0))
        result = 0;
    if (!(result <= 1))
        result = 1;

    return result;
}

double StochasticSpaceTimeMutatorScheduler::mutatorUtilization(const Snapshot& snapshot)
{
    double mutatorUtilization = 1 - headroomFullness(snapshot);
    
    // Scale the mutator utilization into the permitted window.
    mutatorUtilization =
        Options::minimumMutatorUtilization() +
        mutatorUtilization * (
            Options::maximumMutatorUtilization() -
            Options::minimumMutatorUtilization());
    
    return mutatorUtilization;
}

} // namespace JSC

