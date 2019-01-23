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

#include <cmath>
#include <wtf/CurrentTime.h>
#include <wtf/MonotonicTime.h>
#include <wtf/RefCounted.h>

namespace WTF {

class Stopwatch : public RefCounted<Stopwatch> {
public:
    static Ref<Stopwatch> create()
    {
        return adoptRef(*new Stopwatch());
    }

    void reset();
    void start();
    void stop();

    double elapsedTime();
    double elapsedTimeSince(MonotonicTime);

    bool isActive() const { return !std::isnan(m_lastStartTime); }
private:
    Stopwatch() { reset(); }

    double m_elapsedTime;
    double m_lastStartTime;
};

inline void Stopwatch::reset()
{
    m_elapsedTime = 0.0;
    m_lastStartTime = NAN;
}

inline void Stopwatch::start()
{
    ASSERT_WITH_MESSAGE(std::isnan(m_lastStartTime), "Tried to start the stopwatch, but it is already running.");

    m_lastStartTime = monotonicallyIncreasingTime();
}

inline void Stopwatch::stop()
{
    ASSERT_WITH_MESSAGE(!std::isnan(m_lastStartTime), "Tried to stop the stopwatch, but it is not running.");

    m_elapsedTime += monotonicallyIncreasingTime() - m_lastStartTime;
    m_lastStartTime = NAN;
}

inline double Stopwatch::elapsedTime()
{
    if (!isActive())
        return m_elapsedTime;

    return m_elapsedTime + (monotonicallyIncreasingTime() - m_lastStartTime);
}

inline double Stopwatch::elapsedTimeSince(MonotonicTime timeStamp)
{
    if (!isActive())
        return m_elapsedTime;

    return m_elapsedTime + (timeStamp.secondsSinceEpoch().seconds() - m_lastStartTime);
}

} // namespace WTF

using WTF::Stopwatch;
