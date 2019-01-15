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
#ifndef WTF_WallTime_h
#define WTF_WallTime_h

#include <wtf/ClockType.h>
#include <wtf/Seconds.h>

namespace WTF {

class MonotonicTime;
class PrintStream;

// The current time according to a wall clock (aka real time clock). This uses floating point
// internally so that you can reason about infinity and other things that arise in math. It's
// acceptable to use this to wrap NaN times, negative times, and infinite times, so long as they
// are relative to the same clock. Specifically, WallTime should be used in agreement with the
// principle that WallTime::now().secondsSinceEpoch().value() is the same as WTF::currentTime().
class WallTime {
public:
    static const ClockType clockType = ClockType::Wall;
    
    // This is the epoch. So, x.secondsSinceEpoch() should be the same as x - WallTime().
    WallTime() { }
    
    // Call this if you know for sure that the double represents time according to
    // WTF::currentTime(). It must be in seconds and it must be from the same time source.
    static WallTime fromRawSeconds(double value)
    {
        WallTime result;
        result.m_value = value;
        return result;
    }
    
    WTF_EXPORT_PRIVATE static WallTime now();
    
    static WallTime infinity() { return fromRawSeconds(std::numeric_limits<double>::infinity()); }
    
    Seconds secondsSinceEpoch() const { return Seconds(m_value); }
    
    WallTime approximateWallTime() const { return *this; }
    WTF_EXPORT_PRIVATE MonotonicTime approximateMonotonicTime() const;
    
    explicit operator bool() const { return !!m_value; }
    
    WallTime operator+(Seconds other) const
    {
        return fromRawSeconds(m_value + other.value());
    }
    
    WallTime operator-(Seconds other) const
    {
        return fromRawSeconds(m_value - other.value());
    }
    
    // Time is a scalar and scalars can be negated as this could arise from algebraic
    // transformations. So, we allow it.
    WallTime operator-() const
    {
        return fromRawSeconds(-m_value);
    }
    
    WallTime& operator+=(Seconds other)
    {
        return *this = *this + other;
    }
    
    WallTime& operator-=(Seconds other)
    {
        return *this = *this - other;
    }
    
    Seconds operator-(WallTime other) const
    {
        return Seconds(m_value - other.m_value);
    }
    
    bool operator==(WallTime other) const
    {
        return m_value == other.m_value;
    }
    
    bool operator!=(WallTime other) const
    {
        return m_value != other.m_value;
    }
    
    bool operator<(WallTime other) const
    {
        return m_value < other.m_value;
    }
    
    bool operator>(WallTime other) const
    {
        return m_value > other.m_value;
    }
    
    bool operator<=(WallTime other) const
    {
        return m_value <= other.m_value;
    }
    
    bool operator>=(WallTime other) const
    {
        return m_value >= other.m_value;
    }
    
    WTF_EXPORT_PRIVATE void dump(PrintStream&) const;
    
private:
    double m_value { 0 };
};

WTF_EXPORT_PRIVATE void sleep(WallTime);

} // namespace WTF

namespace std {

inline bool isnan(WTF::WallTime time)
{
    return std::isnan(time.secondsSinceEpoch().value());
}

inline bool isinf(WTF::WallTime time)
{
    return std::isinf(time.secondsSinceEpoch().value());
}

inline bool isfinite(WTF::WallTime time)
{
    return std::isfinite(time.secondsSinceEpoch().value());
}

} // namespace std

using WTF::WallTime;

#endif // WTF_WallTime_h
