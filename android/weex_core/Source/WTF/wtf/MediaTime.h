/*
 * Copyright (C) 2012 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer. 
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution. 
 * 3.  Neither the name of Apple Inc. ("Apple") nor the names of
 *     its contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission. 
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

#ifndef WTF_MediaTime_h
#define WTF_MediaTime_h

#include <wtf/FastMalloc.h>

#include <cmath>
#include <limits>
#include <math.h>
#include <stdint.h>

namespace WTF {

class PrintStream;

class WTF_EXPORT_PRIVATE MediaTime {
    WTF_MAKE_FAST_ALLOCATED;
public:
    enum {
        Valid = 1 << 0,
        HasBeenRounded = 1 << 1,
        PositiveInfinite = 1 << 2,
        NegativeInfinite = 1 << 3,
        Indefinite = 1 << 4,
        DoubleValue = 1 << 5,
    };

    MediaTime();
    MediaTime(int64_t value, uint32_t scale, uint8_t flags = Valid);
    MediaTime(const MediaTime& rhs);
    ~MediaTime();

    static MediaTime createWithFloat(float floatTime);
    static MediaTime createWithFloat(float floatTime, uint32_t timeScale);
    static MediaTime createWithDouble(double doubleTime);
    static MediaTime createWithDouble(double doubleTime, uint32_t timeScale);

    float toFloat() const;
    double toDouble() const;

    MediaTime& operator=(const MediaTime& rhs);
    MediaTime& operator+=(const MediaTime& rhs) { return *this = *this + rhs; }
    MediaTime& operator-=(const MediaTime& rhs) { return *this = *this - rhs; }
    MediaTime operator+(const MediaTime& rhs) const;
    MediaTime operator-(const MediaTime& rhs) const;
    MediaTime operator-() const;
    MediaTime operator*(int32_t) const;
    bool operator<(const MediaTime& rhs) const { return compare(rhs) == LessThan; }
    bool operator>(const MediaTime& rhs) const { return compare(rhs) == GreaterThan; }
    bool operator!=(const MediaTime& rhs) const { return compare(rhs) != EqualTo; }
    bool operator==(const MediaTime& rhs) const { return compare(rhs) == EqualTo; }
    bool operator>=(const MediaTime& rhs) const { return compare(rhs) >= EqualTo; }
    bool operator<=(const MediaTime& rhs) const { return compare(rhs) <= EqualTo; }
    bool operator!() const;
    explicit operator bool() const;

    typedef enum {
        LessThan = -1,
        EqualTo = 0,
        GreaterThan = 1,
    } ComparisonFlags;

    ComparisonFlags compare(const MediaTime& rhs) const;
    bool isBetween(const MediaTime&, const MediaTime&) const;

    bool isValid() const { return m_timeFlags & Valid; }
    bool isInvalid() const { return !isValid(); }
    bool hasBeenRounded() const { return m_timeFlags & HasBeenRounded; }
    bool isPositiveInfinite() const { return m_timeFlags & PositiveInfinite; }
    bool isNegativeInfinite() const { return m_timeFlags & NegativeInfinite; }
    bool isIndefinite() const { return m_timeFlags & Indefinite; }
    bool hasDoubleValue() const { return m_timeFlags & DoubleValue; }

    static const MediaTime& zeroTime();
    static const MediaTime& invalidTime();
    static const MediaTime& positiveInfiniteTime();
    static const MediaTime& negativeInfiniteTime();
    static const MediaTime& indefiniteTime();

    const int64_t& timeValue() const { return m_timeValue; }
    const uint32_t& timeScale() const { return m_timeScale; }

    void dump(PrintStream& out) const;

    // Make the following casts errors:
    operator double() const = delete;
    MediaTime(double) = delete;
    operator int() const = delete;
    MediaTime(int) = delete;

    friend WTF_EXPORT_PRIVATE MediaTime abs(const MediaTime& rhs);

    static const uint32_t DefaultTimeScale = 10000000;
    static const uint32_t MaximumTimeScale;

    enum class RoundingFlags {
        HalfAwayFromZero = 0,
        TowardZero,
        AwayFromZero,
        TowardPositiveInfinity,
        TowardNegativeInfinity,
    };

    MediaTime toTimeScale(uint32_t, RoundingFlags = RoundingFlags::HalfAwayFromZero) const;

    template<class Encoder> void encode(Encoder&) const;
    template<class Decoder> static bool decode(Decoder&, MediaTime&);

private:
    void setTimeScale(uint32_t, RoundingFlags = RoundingFlags::HalfAwayFromZero);

    union {
        int64_t m_timeValue;
        double m_timeValueAsDouble;
    };
    uint32_t m_timeScale;
    uint8_t m_timeFlags;
};

inline MediaTime operator*(int32_t lhs, const MediaTime& rhs) { return rhs.operator*(lhs); }

WTF_EXPORT_PRIVATE extern MediaTime abs(const MediaTime& rhs);

template<class Encoder>
void MediaTime::encode(Encoder& encoder) const
{
    encoder << m_timeValue << m_timeScale << m_timeFlags;
}

template<class Decoder>
bool MediaTime::decode(Decoder& decoder, MediaTime& time)
{
    return decoder.decode(time.m_timeValue)
        && decoder.decode(time.m_timeScale)
        && decoder.decode(time.m_timeFlags);
}

}

using WTF::MediaTime;
using WTF::abs;

#endif
