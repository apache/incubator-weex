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
