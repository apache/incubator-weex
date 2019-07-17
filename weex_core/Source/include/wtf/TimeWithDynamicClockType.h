/*
 * Copyright (C) 2016 Apple Inc. All rights reserved.
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

#ifndef WTF_TimeWithDynamicClockType_h
#define WTF_TimeWithDynamicClockType_h

#include <wtf/ClockType.h>
#include <wtf/MonotonicTime.h>
#include <wtf/WallTime.h>

namespace WTF {

class PrintStream;

class TimeWithDynamicClockType {
public:
    TimeWithDynamicClockType() { }
    
    TimeWithDynamicClockType(WallTime time)
        : m_value(time.secondsSinceEpoch().value())
        , m_type(ClockType::Wall)
    {
    }
    
    TimeWithDynamicClockType(MonotonicTime time)
        : m_value(time.secondsSinceEpoch().value())
        , m_type(ClockType::Monotonic)
    {
    }
    
    static TimeWithDynamicClockType fromRawSeconds(double value, ClockType type)
    {
        TimeWithDynamicClockType result;
        result.m_value = value;
        result.m_type = type;
        return result;
    }
    
    Seconds secondsSinceEpoch() const { return Seconds(m_value); }
    ClockType clockType() const { return m_type; }
    
    WTF_EXPORT_PRIVATE static TimeWithDynamicClockType now(ClockType);
    
    WTF_EXPORT_PRIVATE TimeWithDynamicClockType nowWithSameClock() const;
    
    TimeWithDynamicClockType withSameClockAndRawSeconds(double value) const
    {
        return TimeWithDynamicClockType::fromRawSeconds(value, clockType());
    }
    
    // Asserts that the time is of the type you want.
    WTF_EXPORT_PRIVATE WallTime wallTime() const;
    WTF_EXPORT_PRIVATE MonotonicTime monotonicTime() const;
    
    WTF_EXPORT_PRIVATE WallTime approximateWallTime() const;
    WTF_EXPORT_PRIVATE MonotonicTime approximateMonotonicTime() const;
    
    explicit operator bool() const { return !!m_value; }
    
    TimeWithDynamicClockType operator+(Seconds other) const
    {
        return withSameClockAndRawSeconds(m_value + other.value());
    }
    
    TimeWithDynamicClockType operator-(Seconds other) const
    {
        return withSameClockAndRawSeconds(m_value - other.value());
    }
    
    // Time is a scalar and scalars can be negated as this could arise from algebraic
    // transformations. So, we allow it.
    TimeWithDynamicClockType operator-() const
    {
        return withSameClockAndRawSeconds(-m_value);
    }
    
    TimeWithDynamicClockType operator+=(Seconds other)
    {
        return *this = *this + other;
    }
    
    TimeWithDynamicClockType operator-=(Seconds other)
    {
        return *this = *this - other;
    }
    
    WTF_EXPORT_PRIVATE Seconds operator-(const TimeWithDynamicClockType&) const;
    
    bool operator==(const TimeWithDynamicClockType& other) const
    {
        return m_value == other.m_value
            && m_type == other.m_type;
    }
    
    bool operator!=(const TimeWithDynamicClockType& other) const
    {
        return !(*this == other);
    }
    
    // To do relative comparisons, you must be using times with the same clock type.
    WTF_EXPORT_PRIVATE bool operator<(const TimeWithDynamicClockType&) const;
    WTF_EXPORT_PRIVATE bool operator>(const TimeWithDynamicClockType&) const;
    WTF_EXPORT_PRIVATE bool operator<=(const TimeWithDynamicClockType&) const;
    WTF_EXPORT_PRIVATE bool operator>=(const TimeWithDynamicClockType&) const;
    
    WTF_EXPORT_PRIVATE void dump(PrintStream&) const;
    
private:
    double m_value { 0 };
    ClockType m_type { ClockType::Wall };
};

WTF_EXPORT_PRIVATE void sleep(const TimeWithDynamicClockType&);

WTF_EXPORT_PRIVATE bool hasElapsed(const TimeWithDynamicClockType&);

} // namespace WTF

namespace std {

inline bool isnan(WTF::TimeWithDynamicClockType time)
{
    return std::isnan(time.secondsSinceEpoch().value());
}

inline bool isinf(WTF::TimeWithDynamicClockType time)
{
    return std::isinf(time.secondsSinceEpoch().value());
}

inline bool isfinite(WTF::TimeWithDynamicClockType time)
{
    return std::isfinite(time.secondsSinceEpoch().value());
}

} // namespace std

using WTF::TimeWithDynamicClockType;
using WTF::hasElapsed;
using WTF::sleep;

#endif // WTF_TimeWithDynamicClockType_h
