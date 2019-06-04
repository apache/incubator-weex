/*
 * Copyright (C) 2014 University of Washington. All rights reserved.
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
