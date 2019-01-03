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

#include "MutatorScheduler.h"
#include <wtf/Seconds.h>

namespace JSC {

class Heap;

// The JSC concurrent GC sometimes stops the world in order to stay ahead of it. These deliberate,
// synthetic pauses ensure that the GC won't have to do one huge pause in order to catch up to the
// retreating wavefront. The scheduler is called "space-time" because it links the amount of time
// that the world is paused for to the amount of space that the world allocated since the GC cycle
// began.

class SpaceTimeMutatorScheduler : public MutatorScheduler {
public:
    SpaceTimeMutatorScheduler(Heap&);
    ~SpaceTimeMutatorScheduler();
    
    State state() const override;
    
    void beginCollection() override;
    
    void didStop() override;
    void willResume() override;
    void didExecuteConstraints() override;
    
    MonotonicTime timeToStop() override;
    MonotonicTime timeToResume() override;
    
    void log() override;
    
    void endCollection() override;
    
private:
    class Snapshot;
    friend class Snapshot;
    
    double bytesAllocatedThisCycleImpl();
    
    double bytesSinceBeginningOfCycle(const Snapshot&);
    double maxHeadroom();
    double headroomFullness(const Snapshot&);
    double mutatorUtilization(const Snapshot&);
    double collectorUtilization(const Snapshot&);
    Seconds elapsedInPeriod(const Snapshot&);
    double phase(const Snapshot&);
    bool shouldBeResumed(const Snapshot&);
    
    Heap& m_heap;
    Seconds m_period;
    State m_state { Normal };
    
    double m_bytesAllocatedThisCycleAtTheBeginning { 0 };
    double m_bytesAllocatedThisCycleAtTheEnd { 0 };
    MonotonicTime m_startTime;
};

} // namespace JSC

