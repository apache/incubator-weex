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

#include "config.h"
#include "MediaTime.h"

#include <algorithm>
#include <cstdlib>
#include <wtf/CheckedArithmetic.h>
#include <wtf/MathExtras.h>
#include <wtf/PrintStream.h>

namespace WTF {

static uint32_t greatestCommonDivisor(uint32_t a, uint32_t b)
{
    // Euclid's Algorithm
    uint32_t temp = 0;
    while (b) {
        temp = b;
        b = a % b;
        a = temp;
    }
    return a;
}

static uint32_t leastCommonMultiple(uint32_t a, uint32_t b, uint32_t &result)
{
    return safeMultiply(a, b / greatestCommonDivisor(a, b), result);
}

static int64_t signum(int64_t val)
{
    return (0 < val) - (val < 0);
}

const uint32_t MediaTime::MaximumTimeScale = 0x7fffffffL;

MediaTime::MediaTime()
    : m_timeValue(0)
    , m_timeScale(DefaultTimeScale)
    , m_timeFlags(Valid)
{
}

MediaTime::MediaTime(int64_t value, uint32_t scale, uint8_t flags)
    : m_timeValue(value)
    , m_timeScale(scale)
    , m_timeFlags(flags)
{
}

MediaTime::~MediaTime()
{
}

MediaTime::MediaTime(const MediaTime& rhs)
{
    *this = rhs;
}

MediaTime MediaTime::createWithFloat(float floatTime)
{
    if (floatTime != floatTime)
        return invalidTime();
    if (std::isinf(floatTime))
        return std::signbit(floatTime) ? negativeInfiniteTime() : positiveInfiniteTime();
    if (floatTime > std::numeric_limits<int64_t>::max())
        return positiveInfiniteTime();
    if (floatTime < std::numeric_limits<int64_t>::min())
        return negativeInfiniteTime();

    MediaTime value(0, DefaultTimeScale, Valid | DoubleValue);
    value.m_timeValueAsDouble = floatTime;
    return value;
}

MediaTime MediaTime::createWithFloat(float floatTime, uint32_t timeScale)
{
    if (floatTime != floatTime)
        return invalidTime();
    if (std::isinf(floatTime))
        return std::signbit(floatTime) ? negativeInfiniteTime() : positiveInfiniteTime();
    if (floatTime > std::numeric_limits<int64_t>::max())
        return positiveInfiniteTime();
    if (floatTime < std::numeric_limits<int64_t>::min())
        return negativeInfiniteTime();

    while (floatTime * timeScale > std::numeric_limits<int64_t>::max())
        timeScale /= 2;
    return MediaTime(static_cast<int64_t>(floatTime * timeScale), timeScale, Valid);
}

MediaTime MediaTime::createWithDouble(double doubleTime)
{
    if (doubleTime != doubleTime)
        return invalidTime();
    if (std::isinf(doubleTime))
        return std::signbit(doubleTime) ? negativeInfiniteTime() : positiveInfiniteTime();
    if (doubleTime > std::numeric_limits<int64_t>::max())
        return positiveInfiniteTime();
    if (doubleTime < std::numeric_limits<int64_t>::min())
        return negativeInfiniteTime();

    MediaTime value(0, DefaultTimeScale, Valid | DoubleValue);
    value.m_timeValueAsDouble = doubleTime;
    return value;
}

MediaTime MediaTime::createWithDouble(double doubleTime, uint32_t timeScale)
{
    if (doubleTime != doubleTime)
        return invalidTime();
    if (std::isinf(doubleTime))
        return std::signbit(doubleTime) ? negativeInfiniteTime() : positiveInfiniteTime();
    if (doubleTime > std::numeric_limits<int64_t>::max())
        return positiveInfiniteTime();
    if (doubleTime < std::numeric_limits<int64_t>::min())
        return negativeInfiniteTime();

    while (doubleTime * timeScale > std::numeric_limits<int64_t>::max())
        timeScale /= 2;
    return MediaTime(static_cast<int64_t>(doubleTime * timeScale), timeScale, Valid);
}

float MediaTime::toFloat() const
{
    if (isInvalid() || isIndefinite())
        return std::numeric_limits<float>::quiet_NaN();
    if (isPositiveInfinite())
        return std::numeric_limits<float>::infinity();
    if (isNegativeInfinite())
        return -std::numeric_limits<float>::infinity();
    if (hasDoubleValue())
        return m_timeValueAsDouble;
    return static_cast<float>(m_timeValue) / m_timeScale;
}

double MediaTime::toDouble() const
{
    if (isInvalid() || isIndefinite())
        return std::numeric_limits<double>::quiet_NaN();
    if (isPositiveInfinite())
        return std::numeric_limits<double>::infinity();
    if (isNegativeInfinite())
        return -std::numeric_limits<double>::infinity();
    if (hasDoubleValue())
        return m_timeValueAsDouble;
    return static_cast<double>(m_timeValue) / m_timeScale;
}

MediaTime& MediaTime::operator=(const MediaTime& rhs)
{
    m_timeValue = rhs.m_timeValue;
    m_timeScale = rhs.m_timeScale;
    m_timeFlags = rhs.m_timeFlags;
    return *this;
}

MediaTime MediaTime::operator+(const MediaTime& rhs) const
{
    if (rhs.isInvalid() || isInvalid())
        return invalidTime();

    if (rhs.isIndefinite() || isIndefinite())
        return indefiniteTime();

    if (isPositiveInfinite() && rhs.isNegativeInfinite())
        return invalidTime();

    if (isNegativeInfinite() && rhs.isPositiveInfinite())
        return invalidTime();

    if (isPositiveInfinite() || rhs.isPositiveInfinite())
        return positiveInfiniteTime();

    if (isNegativeInfinite() || rhs.isNegativeInfinite())
        return negativeInfiniteTime();

    if (hasDoubleValue() && rhs.hasDoubleValue())
        return MediaTime::createWithDouble(m_timeValueAsDouble + rhs.m_timeValueAsDouble);

    MediaTime a = *this;
    MediaTime b = rhs;

    if (a.hasDoubleValue())
        a.setTimeScale(DefaultTimeScale);
    else if (b.hasDoubleValue())
        b.setTimeScale(DefaultTimeScale);

    uint32_t commonTimeScale;
    if (!leastCommonMultiple(a.m_timeScale, b.m_timeScale, commonTimeScale) || commonTimeScale > MaximumTimeScale)
        commonTimeScale = MaximumTimeScale;
    a.setTimeScale(commonTimeScale);
    b.setTimeScale(commonTimeScale);
    while (!safeAdd(a.m_timeValue, b.m_timeValue, a.m_timeValue)) {
        if (commonTimeScale == 1)
            return a.m_timeValue > 0 ? positiveInfiniteTime() : negativeInfiniteTime();
        commonTimeScale /= 2;
        a.setTimeScale(commonTimeScale);
        b.setTimeScale(commonTimeScale);
    }
    return a;
}

MediaTime MediaTime::operator-(const MediaTime& rhs) const
{
    if (rhs.isInvalid() || isInvalid())
        return invalidTime();

    if (rhs.isIndefinite() || isIndefinite())
        return indefiniteTime();

    if (isPositiveInfinite() && rhs.isPositiveInfinite())
        return invalidTime();

    if (isNegativeInfinite() && rhs.isNegativeInfinite())
        return invalidTime();

    if (isPositiveInfinite() || rhs.isNegativeInfinite())
        return positiveInfiniteTime();

    if (isNegativeInfinite() || rhs.isPositiveInfinite())
        return negativeInfiniteTime();

    if (hasDoubleValue() && rhs.hasDoubleValue())
        return MediaTime::createWithDouble(m_timeValueAsDouble - rhs.m_timeValueAsDouble);

    MediaTime a = *this;
    MediaTime b = rhs;

    if (a.hasDoubleValue())
        a.setTimeScale(DefaultTimeScale);
    else if (b.hasDoubleValue())
        b.setTimeScale(DefaultTimeScale);

    uint32_t commonTimeScale;
    if (!leastCommonMultiple(this->m_timeScale, rhs.m_timeScale, commonTimeScale) || commonTimeScale > MaximumTimeScale)
        commonTimeScale = MaximumTimeScale;
    a.setTimeScale(commonTimeScale);
    b.setTimeScale(commonTimeScale);
    while (!safeSub(a.m_timeValue, b.m_timeValue, a.m_timeValue)) {
        if (commonTimeScale == 1)
            return a.m_timeValue > 0 ? positiveInfiniteTime() : negativeInfiniteTime();
        commonTimeScale /= 2;
        a.setTimeScale(commonTimeScale);
        b.setTimeScale(commonTimeScale);
    }
    return a;
}

MediaTime MediaTime::operator-() const
{
    if (isInvalid())
        return invalidTime();

    if (isIndefinite())
        return indefiniteTime();

    if (isPositiveInfinite())
        return negativeInfiniteTime();

    if (isNegativeInfinite())
        return positiveInfiniteTime();

    MediaTime negativeTime = *this;
    if (negativeTime.hasDoubleValue())
        negativeTime.m_timeValueAsDouble = -negativeTime.m_timeValueAsDouble;
    else
        negativeTime.m_timeValue = -negativeTime.m_timeValue;
    return negativeTime;
}

MediaTime MediaTime::operator*(int32_t rhs) const
{
    if (isInvalid())
        return invalidTime();

    if (isIndefinite())
        return indefiniteTime();

    if (!rhs)
        return zeroTime();

    if (isPositiveInfinite()) {
        if (rhs > 0)
            return positiveInfiniteTime();
        return negativeInfiniteTime();
    }

    if (isNegativeInfinite()) {
        if (rhs > 0)
            return negativeInfiniteTime();
        return positiveInfiniteTime();
    }

    MediaTime a = *this;

    if (a.hasDoubleValue()) {
        a.m_timeValueAsDouble *= rhs;
        return a;
    }

    while (!safeMultiply(a.m_timeValue, rhs, a.m_timeValue)) {
        if (a.m_timeScale == 1)
            return signum(a.m_timeValue) == signum(rhs) ? positiveInfiniteTime() : negativeInfiniteTime();
        a.setTimeScale(a.m_timeScale / 2);
    }

    return a;
}

bool MediaTime::operator!() const
{
    return (m_timeFlags == Valid && !m_timeValue)
        || (m_timeFlags == (Valid | DoubleValue) && !m_timeValueAsDouble);
}

MediaTime::operator bool() const
{
    return !(m_timeFlags == Valid && !m_timeValue)
        && !(m_timeFlags == (Valid | DoubleValue) && !m_timeValueAsDouble);
}

MediaTime::ComparisonFlags MediaTime::compare(const MediaTime& rhs) const
{
    auto andFlags = m_timeFlags & rhs.m_timeFlags;
    if (andFlags & (PositiveInfinite | NegativeInfinite | Indefinite))
        return EqualTo;

    auto orFlags = m_timeFlags | rhs.m_timeFlags;
    if (!(orFlags & Valid))
        return EqualTo;

    if (!(andFlags & Valid))
        return isInvalid() ? GreaterThan : LessThan;

    if (orFlags & NegativeInfinite)
        return isNegativeInfinite() ? LessThan : GreaterThan;

    if (orFlags & PositiveInfinite)
        return isPositiveInfinite() ? GreaterThan : LessThan;

    if (orFlags & Indefinite)
        return isIndefinite() ? GreaterThan : LessThan;

    if (andFlags & DoubleValue) {
        if (m_timeValueAsDouble == rhs.m_timeValueAsDouble)
            return EqualTo;

        return m_timeValueAsDouble < rhs.m_timeValueAsDouble ? LessThan : GreaterThan;
    }

    if (orFlags & DoubleValue) {
        double a = toDouble();
        double b = rhs.toDouble();
        if (a > b)
            return GreaterThan;
        if (a < b)
            return LessThan;
        return EqualTo;
    }

    if ((m_timeValue < 0) != (rhs.m_timeValue < 0))
        return m_timeValue < 0 ? LessThan : GreaterThan;

    if (!m_timeValue && !rhs.m_timeValue)
        return EqualTo;

    if (m_timeScale == rhs.m_timeScale) {
        if (m_timeValue == rhs.m_timeValue)
            return EqualTo;
        return m_timeValue < rhs.m_timeValue ? LessThan : GreaterThan;
    }

    if (m_timeValue == rhs.m_timeValue)
        return m_timeScale < rhs.m_timeScale ? GreaterThan : LessThan;

    if (m_timeValue < rhs.m_timeValue && m_timeScale > rhs.m_timeScale)
        return LessThan;

    if (m_timeValue > rhs.m_timeValue && m_timeScale < rhs.m_timeScale)
        return GreaterThan;

    int64_t lhsFactor;
    int64_t rhsFactor;
    if (safeMultiply(m_timeValue, static_cast<int64_t>(rhs.m_timeScale), lhsFactor)
        && safeMultiply(rhs.m_timeValue, static_cast<int64_t>(m_timeScale), rhsFactor)) {
        if (lhsFactor == rhsFactor)
            return EqualTo;
        return lhsFactor < rhsFactor ? LessThan : GreaterThan;
    }

    int64_t rhsWhole = rhs.m_timeValue / rhs.m_timeScale;
    int64_t lhsWhole = m_timeValue / m_timeScale;
    if (lhsWhole > rhsWhole)
        return GreaterThan;
    if (lhsWhole < rhsWhole)
        return LessThan;

    int64_t rhsRemain = rhs.m_timeValue % rhs.m_timeScale;
    int64_t lhsRemain = m_timeValue % m_timeScale;
    lhsFactor = lhsRemain * rhs.m_timeScale;
    rhsFactor = rhsRemain * m_timeScale;

    if (lhsFactor == rhsFactor)
        return EqualTo;
    return lhsFactor > rhsFactor ? GreaterThan : LessThan;
}

bool MediaTime::isBetween(const MediaTime& a, const MediaTime& b) const
{
    if (a > b)
        return *this > b && *this < a;
    return *this > a && *this < b;
}

const MediaTime& MediaTime::zeroTime()
{
    static const MediaTime* time = new MediaTime(0, 1, Valid);
    return *time;
}

const MediaTime& MediaTime::invalidTime()
{
    static const MediaTime* time = new MediaTime(-1, 1, 0);
    return *time;
}

const MediaTime& MediaTime::positiveInfiniteTime()
{
    static const MediaTime* time = new MediaTime(0, 1, PositiveInfinite | Valid);
    return *time;
}

const MediaTime& MediaTime::negativeInfiniteTime()
{
    static const MediaTime* time = new MediaTime(-1, 1, NegativeInfinite | Valid);
    return *time;
}

const MediaTime& MediaTime::indefiniteTime()
{
    static const MediaTime* time = new MediaTime(0, 1, Indefinite | Valid);
    return *time;
}

MediaTime MediaTime::toTimeScale(uint32_t timeScale, RoundingFlags flags) const
{
    MediaTime result = *this;
    result.setTimeScale(timeScale, flags);
    return result;
}

void MediaTime::setTimeScale(uint32_t timeScale, RoundingFlags flags)
{
    if (hasDoubleValue()) {
        *this = MediaTime::createWithDouble(m_timeValueAsDouble, timeScale);
        return;
    }

    if (timeScale == m_timeScale)
        return;

    timeScale = std::min(MaximumTimeScale, timeScale);

#if !PLATFORM(WIN) && (CPU(X86_64) || CPU(ARM64))
    __int128_t newValue = static_cast<__int128_t>(m_timeValue) * timeScale;
    int64_t remainder = newValue % m_timeScale;
    newValue = newValue / m_timeScale;

    if (newValue < std::numeric_limits<int64_t>::min()) {
        *this = negativeInfiniteTime();
        return;
    }

    if (newValue > std::numeric_limits<int64_t>::max()) {
        *this = positiveInfiniteTime();
        return;
    }
#else
    int64_t newValue = m_timeValue / m_timeScale;
    int64_t partialRemainder = (m_timeValue % m_timeScale) * timeScale;
    int64_t remainder = partialRemainder % m_timeScale;

    if (!safeMultiply<int64_t>(newValue, static_cast<int64_t>(timeScale), newValue)
        || !safeAdd(newValue, partialRemainder / m_timeScale, newValue)) {
        *this = newValue < 0 ? negativeInfiniteTime() : positiveInfiniteTime();
        return;
    }
#endif

    m_timeValue = newValue;
    std::swap(m_timeScale, timeScale);

    if (!remainder)
        return;

    m_timeFlags |= HasBeenRounded;
    switch (flags) {
    case RoundingFlags::HalfAwayFromZero:
        if (static_cast<int64_t>(llabs(remainder)) * 2 >= static_cast<int64_t>(timeScale)) {
            // round up (away from zero)
            if (remainder < 0)
                m_timeValue--;
            else
                m_timeValue++;
        }
        break;

    case RoundingFlags::TowardZero:
        break;

    case RoundingFlags::AwayFromZero:
        if (remainder < 0)
            m_timeValue--;
        else
            m_timeValue++;
        break;

    case RoundingFlags::TowardPositiveInfinity:
        if (remainder > 0)
            m_timeValue++;
        break;
        
    case RoundingFlags::TowardNegativeInfinity:
        if (remainder < 0)
            m_timeValue--;
        break;
    }
}

void MediaTime::dump(PrintStream &out) const
{
    out.print("{", m_timeValue, "/", m_timeScale, ", ", toDouble(), "}");
}

MediaTime abs(const MediaTime& rhs)
{
    if (rhs.isInvalid())
        return MediaTime::invalidTime();
    if (rhs.isNegativeInfinite() || rhs.isPositiveInfinite())
        return MediaTime::positiveInfiniteTime();
    if (rhs.hasDoubleValue())
        return MediaTime::createWithDouble(fabs(rhs.m_timeValueAsDouble));

    MediaTime val = rhs;
    val.m_timeValue = std::abs(rhs.m_timeValue);
    return val;
}

}

