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

#include "JSGlobalObject.h"
#include "Options.h"
#include <wtf/PrintStream.h>

namespace JSC {

class CodeBlock;

enum CountingVariant {
    CountingForBaseline,
    CountingForUpperTiers
};

double applyMemoryUsageHeuristics(int32_t value, CodeBlock*);
int32_t applyMemoryUsageHeuristicsAndConvertToInt(int32_t value, CodeBlock*);

inline int32_t formattedTotalExecutionCount(float value)
{
    union {
        int32_t i;
        float f;
    } u;
    u.f = value;
    return u.i;
}
    
template<CountingVariant countingVariant>
class ExecutionCounter {
public:
    ExecutionCounter();
    void forceSlowPathConcurrently(); // If you use this, checkIfThresholdCrossedAndSet() may still return false.
    bool checkIfThresholdCrossedAndSet(CodeBlock*);
    void setNewThreshold(int32_t threshold, CodeBlock*);
    void deferIndefinitely();
    double count() const { return static_cast<double>(m_totalCount) + m_counter; }
    void dump(PrintStream&) const;
    
    static int32_t maximumExecutionCountsBetweenCheckpoints()
    {
        switch (countingVariant) {
        case CountingForBaseline:
            return Options::maximumExecutionCountsBetweenCheckpointsForBaseline();
        case CountingForUpperTiers:
            return Options::maximumExecutionCountsBetweenCheckpointsForUpperTiers();
        default:
            RELEASE_ASSERT_NOT_REACHED();
            return 0;
        }
    }
    
    template<typename T>
    static T clippedThreshold(JSGlobalObject* globalObject, T threshold)
    {
        int32_t maxThreshold;
        if (Options::randomizeExecutionCountsBetweenCheckpoints())
            maxThreshold = globalObject->weakRandomInteger() % maximumExecutionCountsBetweenCheckpoints();
        else
            maxThreshold = maximumExecutionCountsBetweenCheckpoints();
        if (threshold > maxThreshold)
            threshold = maxThreshold;
        return threshold;
    }

private:
    bool hasCrossedThreshold(CodeBlock*) const;
    bool setThreshold(CodeBlock*);
    void reset();

public:
    // NB. These are intentionally public because it will be modified from machine code.
    
    // This counter is incremented by the JIT or LLInt. It starts out negative and is
    // counted up until it becomes non-negative. At the start of a counting period,
    // the threshold we wish to reach is m_totalCount + m_counter, in the sense that
    // we will add X to m_totalCount and subtract X from m_counter.
    int32_t m_counter;

    // Counts the total number of executions we have seen plus the ones we've set a
    // threshold for in m_counter. Because m_counter's threshold is negative, the
    // total number of actual executions can always be computed as m_totalCount +
    // m_counter.
    float m_totalCount;

    // This is the threshold we were originally targeting, without any correction for
    // the memory usage heuristics.
    int32_t m_activeThreshold;
};

typedef ExecutionCounter<CountingForBaseline> BaselineExecutionCounter;
typedef ExecutionCounter<CountingForUpperTiers> UpperTierExecutionCounter;

} // namespace JSC
