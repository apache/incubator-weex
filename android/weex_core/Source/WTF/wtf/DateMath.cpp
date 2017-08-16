/*
 * Copyright (C) 1999-2000 Harri Porten (porten@kde.org)
 * Copyright (C) 2006, 2007 Apple Inc. All rights reserved.
 * Copyright (C) 2009 Google Inc. All rights reserved.
 * Copyright (C) 2007-2009 Torch Mobile, Inc.
 * Copyright (C) 2010 &yet, LLC. (nate@andyet.net)
 *
 * The Original Code is Mozilla Communicator client code, released
 * March 31, 1998.
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 1998
 * the Initial Developer. All Rights Reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * Alternatively, the contents of this file may be used under the terms
 * of either the Mozilla Public License Version 1.1, found at
 * http://www.mozilla.org/MPL/ (the "MPL") or the GNU General Public
 * License Version 2.0, found at http://www.fsf.org/copyleft/gpl.html
 * (the "GPL"), in which case the provisions of the MPL or the GPL are
 * applicable instead of those above.  If you wish to allow use of your
 * version of this file only under the terms of one of those two
 * licenses (the MPL or the GPL) and not to allow others to use your
 * version of this file under the LGPL, indicate your decision by
 * deletingthe provisions above and replace them with the notice and
 * other provisions required by the MPL or the GPL, as the case may be.
 * If you do not delete the provisions above, a recipient may use your
 * version of this file under any of the LGPL, the MPL or the GPL.

 * Copyright 2006-2008 the V8 project authors. All rights reserved.
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer in the documentation and/or other materials provided
 *       with the distribution.
 *     * Neither the name of Google Inc. nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "DateMath.h"

#include "Assertions.h"
#include "ASCIICType.h"
#include "CurrentTime.h"
#include "MathExtras.h"
#include "StdLibExtras.h"
#include "StringExtras.h"

#include <algorithm>
#include <limits.h>
#include <limits>
#include <stdint.h>
#include <time.h>
#include <wtf/text/StringBuilder.h>

#if OS(WINDOWS)
#include <windows.h>
#endif

#if HAVE(ERRNO_H)
#include <errno.h>
#endif

#if HAVE(SYS_TIME_H)
#include <sys/time.h>
#endif

#if HAVE(SYS_TIMEB_H)
#include <sys/timeb.h>
#endif

using namespace WTF;

namespace WTF {

/* Constants */

static const double maxUnixTime = 2145859200.0; // 12/31/2037
// ECMAScript asks not to support for a date of which total
// millisecond value is larger than the following value.
// See 15.9.1.14 of ECMA-262 5th edition.
static const double maxECMAScriptTime = 8.64E15;

// Day of year for the first day of each month, where index 0 is January, and day 0 is January 1.
// First for non-leap years, then for leap years.
static const int firstDayOfMonth[2][12] = {
    {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334},
    {0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335}
};

static inline void getLocalTime(const time_t* localTime, struct tm* localTM)
{
#if COMPILER(MSVC)
    localtime_s(localTM, localTime);
#elif HAVE(LOCALTIME_R)
    localtime_r(localTime, localTM);
#else
    localtime_r(localTime, localTM);
#endif
}

bool isLeapYear(int year)
{
    if (year % 4 != 0)
        return false;
    if (year % 400 == 0)
        return true;
    if (year % 100 == 0)
        return false;
    return true;
}

static inline int daysInYear(int year)
{
    return 365 + isLeapYear(year);
}

static inline double daysFrom1970ToYear(int year)
{
    // The Gregorian Calendar rules for leap years:
    // Every fourth year is a leap year.  2004, 2008, and 2012 are leap years.
    // However, every hundredth year is not a leap year.  1900 and 2100 are not leap years.
    // Every four hundred years, there's a leap year after all.  2000 and 2400 are leap years.

    static const int leapDaysBefore1971By4Rule = 1970 / 4;
    static const int excludedLeapDaysBefore1971By100Rule = 1970 / 100;
    static const int leapDaysBefore1971By400Rule = 1970 / 400;

    const double yearMinusOne = year - 1;
    const double yearsToAddBy4Rule = floor(yearMinusOne / 4.0) - leapDaysBefore1971By4Rule;
    const double yearsToExcludeBy100Rule = floor(yearMinusOne / 100.0) - excludedLeapDaysBefore1971By100Rule;
    const double yearsToAddBy400Rule = floor(yearMinusOne / 400.0) - leapDaysBefore1971By400Rule;

    return 365.0 * (year - 1970.0) + yearsToAddBy4Rule - yearsToExcludeBy100Rule + yearsToAddBy400Rule;
}

double msToDays(double ms)
{
    return floor(ms / msPerDay);
}

static void appendTwoDigitNumber(StringBuilder& builder, int number)
{
    ASSERT(number >= 0);
    ASSERT(number < 100);
    builder.append(static_cast<LChar>('0' + number / 10));
    builder.append(static_cast<LChar>('0' + number % 10));
}

int msToYear(double ms)
{
    int approxYear = static_cast<int>(floor(ms / (msPerDay * 365.2425)) + 1970);
    double msFromApproxYearTo1970 = msPerDay * daysFrom1970ToYear(approxYear);
    if (msFromApproxYearTo1970 > ms)
        return approxYear - 1;
    if (msFromApproxYearTo1970 + msPerDay * daysInYear(approxYear) <= ms)
        return approxYear + 1;
    return approxYear;
}

int dayInYear(double ms, int year)
{
    return static_cast<int>(msToDays(ms) - daysFrom1970ToYear(year));
}

static inline double msToMilliseconds(double ms)
{
    double result = fmod(ms, msPerDay);
    if (result < 0)
        result += msPerDay;
    return result;
}

int msToMinutes(double ms)
{
    double result = fmod(floor(ms / msPerMinute), minutesPerHour);
    if (result < 0)
        result += minutesPerHour;
    return static_cast<int>(result);
}

int msToHours(double ms)
{
    double result = fmod(floor(ms/msPerHour), hoursPerDay);
    if (result < 0)
        result += hoursPerDay;
    return static_cast<int>(result);
}

int monthFromDayInYear(int dayInYear, bool leapYear)
{
    const int d = dayInYear;
    int step;

    if (d < (step = 31))
        return 0;
    step += (leapYear ? 29 : 28);
    if (d < step)
        return 1;
    if (d < (step += 31))
        return 2;
    if (d < (step += 30))
        return 3;
    if (d < (step += 31))
        return 4;
    if (d < (step += 30))
        return 5;
    if (d < (step += 31))
        return 6;
    if (d < (step += 31))
        return 7;
    if (d < (step += 30))
        return 8;
    if (d < (step += 31))
        return 9;
    if (d < (step += 30))
        return 10;
    return 11;
}

static inline bool checkMonth(int dayInYear, int& startDayOfThisMonth, int& startDayOfNextMonth, int daysInThisMonth)
{
    startDayOfThisMonth = startDayOfNextMonth;
    startDayOfNextMonth += daysInThisMonth;
    return (dayInYear <= startDayOfNextMonth);
}

int dayInMonthFromDayInYear(int dayInYear, bool leapYear)
{
    const int d = dayInYear;
    int step;
    int next = 30;

    if (d <= next)
        return d + 1;
    const int daysInFeb = (leapYear ? 29 : 28);
    if (checkMonth(d, step, next, daysInFeb))
        return d - step;
    if (checkMonth(d, step, next, 31))
        return d - step;
    if (checkMonth(d, step, next, 30))
        return d - step;
    if (checkMonth(d, step, next, 31))
        return d - step;
    if (checkMonth(d, step, next, 30))
        return d - step;
    if (checkMonth(d, step, next, 31))
        return d - step;
    if (checkMonth(d, step, next, 31))
        return d - step;
    if (checkMonth(d, step, next, 30))
        return d - step;
    if (checkMonth(d, step, next, 31))
        return d - step;
    if (checkMonth(d, step, next, 30))
        return d - step;
    step = next;
    return d - step;
}

int dayInYear(int year, int month, int day)
{
    return firstDayOfMonth[isLeapYear(year)][month] + day - 1;
}

double dateToDaysFrom1970(int year, int month, int day)
{
    year += month / 12;

    month %= 12;
    if (month < 0) {
        month += 12;
        --year;
    }

    double yearday = floor(daysFrom1970ToYear(year));
    ASSERT((year >= 1970 && yearday >= 0) || (year < 1970 && yearday < 0));
    return yearday + dayInYear(year, month, day);
}

// There is a hard limit at 2038 that we currently do not have a workaround
// for (rdar://problem/5052975).
static inline int maximumYearForDST()
{
    return 2037;
}

static inline int minimumYearForDST()
{
    // Because of the 2038 issue (see maximumYearForDST) if the current year is
    // greater than the max year minus 27 (2010), we want to use the max year
    // minus 27 instead, to ensure there is a range of 28 years that all years
    // can map to.
    return std::min(msToYear(jsCurrentTime()), maximumYearForDST() - 27) ;
}

/*
 * Find an equivalent year for the one given, where equivalence is deterined by
 * the two years having the same leapness and the first day of the year, falling
 * on the same day of the week.
 *
 * This function returns a year between this current year and 2037, however this
 * function will potentially return incorrect results if the current year is after
 * 2010, (rdar://problem/5052975), if the year passed in is before 1900 or after
 * 2100, (rdar://problem/5055038).
 */
int equivalentYearForDST(int year)
{
    // It is ok if the cached year is not the current year as long as the rules
    // for DST did not change between the two years; if they did the app would need
    // to be restarted.
    static int minYear = minimumYearForDST();
    int maxYear = maximumYearForDST();

    int difference;
    if (year > maxYear)
        difference = minYear - year;
    else if (year < minYear)
        difference = maxYear - year;
    else
        return year;

    int quotient = difference / 28;
    int product = (quotient) * 28;

    year += product;
    return year;
}

#if OS(WINDOWS)
typedef BOOL(WINAPI* callGetTimeZoneInformationForYear_t)(USHORT, PDYNAMIC_TIME_ZONE_INFORMATION, LPTIME_ZONE_INFORMATION);

static callGetTimeZoneInformationForYear_t timeZoneInformationForYearFunction()
{
    static callGetTimeZoneInformationForYear_t getTimeZoneInformationForYear = nullptr;

    if (getTimeZoneInformationForYear)
        return getTimeZoneInformationForYear;

    HMODULE module = ::GetModuleHandleW(L"kernel32.dll");
    if (!module)
        return nullptr;

    getTimeZoneInformationForYear = reinterpret_cast<callGetTimeZoneInformationForYear_t>(::GetProcAddress(module, "GetTimeZoneInformationForYear"));

    return getTimeZoneInformationForYear;
}
#endif

static int32_t calculateUTCOffset()
{
#if OS(WINDOWS)
    TIME_ZONE_INFORMATION timeZoneInformation;
    DWORD rc = 0;

    if (callGetTimeZoneInformationForYear_t timeZoneFunction = timeZoneInformationForYearFunction()) {
        // If available, use the Windows API call that takes into account the varying DST from
        // year to year.
        SYSTEMTIME systemTime;
        ::GetSystemTime(&systemTime);
        rc = timeZoneFunction(systemTime.wYear, nullptr, &timeZoneInformation);
        if (rc == TIME_ZONE_ID_INVALID)
            return 0;
    } else {
        rc = ::GetTimeZoneInformation(&timeZoneInformation);
        if (rc == TIME_ZONE_ID_INVALID)
            return 0;
    }

    int32_t bias = timeZoneInformation.Bias;

    if (rc == TIME_ZONE_ID_DAYLIGHT)
        bias += timeZoneInformation.DaylightBias;
    else if (rc == TIME_ZONE_ID_STANDARD || rc == TIME_ZONE_ID_UNKNOWN)
        bias += timeZoneInformation.StandardBias;

    return -bias * 60 * 1000;
#else
    time_t localTime = time(0);
    tm localt;
    getLocalTime(&localTime, &localt);

    // Get the difference between this time zone and UTC on the 1st of January of this year.
    localt.tm_sec = 0;
    localt.tm_min = 0;
    localt.tm_hour = 0;
    localt.tm_mday = 1;
    localt.tm_mon = 0;
    // Not setting localt.tm_year!
    localt.tm_wday = 0;
    localt.tm_yday = 0;
    localt.tm_isdst = 0;
#if HAVE(TM_GMTOFF)
    localt.tm_gmtoff = 0;
#endif
#if HAVE(TM_ZONE)
    localt.tm_zone = 0;
#endif

#if HAVE(TIMEGM)
    time_t utcOffset = timegm(&localt) - mktime(&localt);
#else
    // Using a canned date of 01/01/2009 on platforms with weaker date-handling foo.
    localt.tm_year = 109;
    time_t utcOffset = 1230768000 - mktime(&localt);
#endif

    return static_cast<int32_t>(utcOffset * 1000);
#endif
}

#if !HAVE(TM_GMTOFF)

#if OS(WINDOWS)
// Code taken from http://support.microsoft.com/kb/167296
static void UnixTimeToFileTime(time_t t, LPFILETIME pft)
{
    // Note that LONGLONG is a 64-bit value
    LONGLONG ll;

    ll = Int32x32To64(t, 10000000) + 116444736000000000;
    pft->dwLowDateTime = (DWORD)ll;
    pft->dwHighDateTime = ll >> 32;
}
#endif

/*
 * Get the DST offset for the time passed in.
 */
static double calculateDSTOffset(time_t localTime, double utcOffset)
{
#if OS(WINDOWS)
    FILETIME utcFileTime;
    UnixTimeToFileTime(localTime, &utcFileTime);
    SYSTEMTIME utcSystemTime, localSystemTime;
    if (!::FileTimeToSystemTime(&utcFileTime, &utcSystemTime))
        return 0;
    if (!::SystemTimeToTzSpecificLocalTime(nullptr, &utcSystemTime, &localSystemTime))
        return 0;

    double offsetTime = (localTime * msPerSecond) + utcOffset;

    // Offset from UTC but doesn't include DST obviously
    int offsetHour =  msToHours(offsetTime);
    int offsetMinute =  msToMinutes(offsetTime);

    double diff = ((localSystemTime.wHour - offsetHour) * secondsPerHour) + ((localSystemTime.wMinute - offsetMinute) * 60);

    return diff * msPerSecond;
#else
    //input is UTC so we have to shift back to local time to determine DST thus the + getUTCOffset()
    double offsetTime = (localTime * msPerSecond) + utcOffset;

    // Offset from UTC but doesn't include DST obviously
    int offsetHour =  msToHours(offsetTime);
    int offsetMinute =  msToMinutes(offsetTime);

    tm localTM;
    getLocalTime(&localTime, &localTM);

    double diff = ((localTM.tm_hour - offsetHour) * secondsPerHour) + ((localTM.tm_min - offsetMinute) * 60);

    if (diff < 0)
        diff += secondsPerDay;

    return (diff * msPerSecond);
#endif
}

#endif

// Returns combined offset in millisecond (UTC + DST).
LocalTimeOffset calculateLocalTimeOffset(double ms, TimeType inputTimeType)
{
#if HAVE(TM_GMTOFF)
    double localToUTCTimeOffset = inputTimeType == LocalTime ? calculateUTCOffset() : 0;
#else
    double localToUTCTimeOffset = calculateUTCOffset();
#endif
    if (inputTimeType == LocalTime)
        ms -= localToUTCTimeOffset;

    // On Mac OS X, the call to localtime (see calculateDSTOffset) will return historically accurate
    // DST information (e.g. New Zealand did not have DST from 1946 to 1974) however the JavaScript
    // standard explicitly dictates that historical information should not be considered when
    // determining DST. For this reason we shift away from years that localtime can handle but would
    // return historically accurate information.
    int year = msToYear(ms);
    int equivalentYear = equivalentYearForDST(year);
    if (year != equivalentYear) {
        bool leapYear = isLeapYear(year);
        int dayInYearLocal = dayInYear(ms, year);
        int dayInMonth = dayInMonthFromDayInYear(dayInYearLocal, leapYear);
        int month = monthFromDayInYear(dayInYearLocal, leapYear);
        double day = dateToDaysFrom1970(equivalentYear, month, dayInMonth);
        ms = (day * msPerDay) + msToMilliseconds(ms);
    }

    double localTimeSeconds = ms / msPerSecond;
    if (localTimeSeconds > maxUnixTime)
        localTimeSeconds = maxUnixTime;
    else if (localTimeSeconds < 0) // Go ahead a day to make localtime work (does not work with 0).
        localTimeSeconds += secondsPerDay;
    // FIXME: time_t has a potential problem in 2038.
    time_t localTime = static_cast<time_t>(localTimeSeconds);

#if HAVE(TM_GMTOFF)
    tm localTM;
    getLocalTime(&localTime, &localTM);
    return LocalTimeOffset(localTM.tm_isdst, localTM.tm_gmtoff * msPerSecond);
#else
    double dstOffset = calculateDSTOffset(localTime, localToUTCTimeOffset);
    return LocalTimeOffset(dstOffset, localToUTCTimeOffset + dstOffset);
#endif
}

void initializeDates()
{
#if !ASSERT_DISABLED
    static bool alreadyInitialized;
    ASSERT(!alreadyInitialized);
    alreadyInitialized = true;
#endif

    equivalentYearForDST(2000); // Need to call once to initialize a static used in this function.
}

static inline double ymdhmsToSeconds(int year, long mon, long day, long hour, long minute, double second)
{
    int mday = firstDayOfMonth[isLeapYear(year)][mon - 1];
    double ydays = daysFrom1970ToYear(year);

    return (second + minute * secondsPerMinute + hour * secondsPerHour + (mday + day - 1 + ydays) * secondsPerDay);
}

// We follow the recommendation of RFC 2822 to consider all
// obsolete time zones not listed here equivalent to "-0000".
static const struct KnownZone {
#if !OS(WINDOWS)
    const
#endif
        char tzName[4];
    int tzOffset;
} known_zones[] = {
    { "UT", 0 },
    { "GMT", 0 },
    { "EST", -300 },
    { "EDT", -240 },
    { "CST", -360 },
    { "CDT", -300 },
    { "MST", -420 },
    { "MDT", -360 },
    { "PST", -480 },
    { "PDT", -420 }
};

inline static void skipSpacesAndComments(const char*& s)
{
    int nesting = 0;
    char ch;
    while ((ch = *s)) {
        if (!isASCIISpace(ch)) {
            if (ch == '(')
                nesting++;
            else if (ch == ')' && nesting > 0)
                nesting--;
            else if (nesting == 0)
                break;
        }
        s++;
    }
}

// returns 0-11 (Jan-Dec); -1 on failure
static int findMonth(const char* monthStr)
{
    ASSERT(monthStr);
    char needle[4];
    for (int i = 0; i < 3; ++i) {
        if (!*monthStr)
            return -1;
        needle[i] = static_cast<char>(toASCIILower(*monthStr++));
    }
    needle[3] = '\0';
    const char *haystack = "janfebmaraprmayjunjulaugsepoctnovdec";
    const char *str = strstr(haystack, needle);
    if (str) {
        int position = static_cast<int>(str - haystack);
        if (position % 3 == 0)
            return position / 3;
    }
    return -1;
}

static bool parseInt(const char* string, char** stopPosition, int base, int* result)
{
    long longResult = strtol(string, stopPosition, base);
    // Avoid the use of errno as it is not available on Windows CE
    if (string == *stopPosition || longResult <= std::numeric_limits<int>::min() || longResult >= std::numeric_limits<int>::max())
        return false;
    *result = static_cast<int>(longResult);
    return true;
}

static bool parseLong(const char* string, char** stopPosition, int base, long* result)
{
    *result = strtol(string, stopPosition, base);
    // Avoid the use of errno as it is not available on Windows CE
    if (string == *stopPosition || *result == std::numeric_limits<long>::min() || *result == std::numeric_limits<long>::max())
        return false;
    return true;
}

// Parses a date with the format YYYY[-MM[-DD]].
// Year parsing is lenient, allows any number of digits, and +/-.
// Returns 0 if a parse error occurs, else returns the end of the parsed portion of the string.
static char* parseES5DatePortion(const char* currentPosition, int& year, long& month, long& day)
{
    char* postParsePosition;

    // This is a bit more lenient on the year string than ES5 specifies:
    // instead of restricting to 4 digits (or 6 digits with mandatory +/-),
    // it accepts any integer value. Consider this an implementation fallback.
    if (!parseInt(currentPosition, &postParsePosition, 10, &year))
        return 0;

    // Check for presence of -MM portion.
    if (*postParsePosition != '-')
        return postParsePosition;
    currentPosition = postParsePosition + 1;
    
    if (!isASCIIDigit(*currentPosition))
        return 0;
    if (!parseLong(currentPosition, &postParsePosition, 10, &month))
        return 0;
    if ((postParsePosition - currentPosition) != 2)
        return 0;

    // Check for presence of -DD portion.
    if (*postParsePosition != '-')
        return postParsePosition;
    currentPosition = postParsePosition + 1;
    
    if (!isASCIIDigit(*currentPosition))
        return 0;
    if (!parseLong(currentPosition, &postParsePosition, 10, &day))
        return 0;
    if ((postParsePosition - currentPosition) != 2)
        return 0;
    return postParsePosition;
}

// Parses a time with the format HH:mm[:ss[.sss]][Z|(+|-)00:00].
// Fractional seconds parsing is lenient, allows any number of digits.
// Returns 0 if a parse error occurs, else returns the end of the parsed portion of the string.
static char* parseES5TimePortion(char* currentPosition, long& hours, long& minutes, double& seconds, long& timeZoneSeconds)
{
    char* postParsePosition;
    if (!isASCIIDigit(*currentPosition))
        return 0;
    if (!parseLong(currentPosition, &postParsePosition, 10, &hours))
        return 0;
    if (*postParsePosition != ':' || (postParsePosition - currentPosition) != 2)
        return 0;
    currentPosition = postParsePosition + 1;
    
    if (!isASCIIDigit(*currentPosition))
        return 0;
    if (!parseLong(currentPosition, &postParsePosition, 10, &minutes))
        return 0;
    if ((postParsePosition - currentPosition) != 2)
        return 0;
    currentPosition = postParsePosition;

    // Seconds are optional.
    if (*currentPosition == ':') {
        ++currentPosition;
    
        long intSeconds;
        if (!isASCIIDigit(*currentPosition))
            return 0;
        if (!parseLong(currentPosition, &postParsePosition, 10, &intSeconds))
            return 0;
        if ((postParsePosition - currentPosition) != 2)
            return 0;
        seconds = intSeconds;
        if (*postParsePosition == '.') {
            currentPosition = postParsePosition + 1;
            
            // In ECMA-262-5 it's a bit unclear if '.' can be present without milliseconds, but
            // a reasonable interpretation guided by the given examples and RFC 3339 says "no".
            // We check the next character to avoid reading +/- timezone hours after an invalid decimal.
            if (!isASCIIDigit(*currentPosition))
                return 0;
            
            // We are more lenient than ES5 by accepting more or less than 3 fraction digits.
            long fracSeconds;
            if (!parseLong(currentPosition, &postParsePosition, 10, &fracSeconds))
                return 0;
            
            long numFracDigits = postParsePosition - currentPosition;
            seconds += fracSeconds * pow(10.0, static_cast<double>(-numFracDigits));
        }
        currentPosition = postParsePosition;
    }

    if (*currentPosition == 'Z')
        return currentPosition + 1;

    bool tzNegative;
    if (*currentPosition == '-')
        tzNegative = true;
    else if (*currentPosition == '+')
        tzNegative = false;
    else
        return currentPosition; // no timezone
    ++currentPosition;
    
    long tzHours;
    long tzHoursAbs;
    long tzMinutes;
    
    if (!isASCIIDigit(*currentPosition))
        return 0;
    if (!parseLong(currentPosition, &postParsePosition, 10, &tzHours))
        return 0;
    if (*postParsePosition != ':' || (postParsePosition - currentPosition) != 2)
        return 0;
    tzHoursAbs = labs(tzHours);
    currentPosition = postParsePosition + 1;
    
    if (!isASCIIDigit(*currentPosition))
        return 0;
    if (!parseLong(currentPosition, &postParsePosition, 10, &tzMinutes))
        return 0;
    if ((postParsePosition - currentPosition) != 2)
        return 0;
    currentPosition = postParsePosition;
    
    if (tzHoursAbs > 24)
        return 0;
    if (tzMinutes < 0 || tzMinutes > 59)
        return 0;
    
    timeZoneSeconds = 60 * (tzMinutes + (60 * tzHoursAbs));
    if (tzNegative)
        timeZoneSeconds = -timeZoneSeconds;

    return currentPosition;
}

double parseES5DateFromNullTerminatedCharacters(const char* dateString)
{
    // This parses a date of the form defined in ECMA-262-5, section 15.9.1.15
    // (similar to RFC 3339 / ISO 8601: YYYY-MM-DDTHH:mm:ss[.sss]Z).
    // In most cases it is intentionally strict (e.g. correct field widths, no stray whitespace).
    
    static const long daysPerMonth[12] = { 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    
    // The year must be present, but the other fields may be omitted - see ES5.1 15.9.1.15.
    int year = 0;
    long month = 1;
    long day = 1;
    long hours = 0;
    long minutes = 0;
    double seconds = 0;
    long timeZoneSeconds = 0;

    // Parse the date YYYY[-MM[-DD]]
    char* currentPosition = parseES5DatePortion(dateString, year, month, day);
    if (!currentPosition)
        return std::numeric_limits<double>::quiet_NaN();
    // Look for a time portion.
    if (*currentPosition == 'T') {
        // Parse the time HH:mm[:ss[.sss]][Z|(+|-)00:00]
        currentPosition = parseES5TimePortion(currentPosition + 1, hours, minutes, seconds, timeZoneSeconds);
        if (!currentPosition)
            return std::numeric_limits<double>::quiet_NaN();
    }
    // Check that we have parsed all characters in the string.
    if (*currentPosition)
        return std::numeric_limits<double>::quiet_NaN();

    // A few of these checks could be done inline above, but since many of them are interrelated
    // we would be sacrificing readability to "optimize" the (presumably less common) failure path.
    if (month < 1 || month > 12)
        return std::numeric_limits<double>::quiet_NaN();
    if (day < 1 || day > daysPerMonth[month - 1])
        return std::numeric_limits<double>::quiet_NaN();
    if (month == 2 && day > 28 && !isLeapYear(year))
        return std::numeric_limits<double>::quiet_NaN();
    if (hours < 0 || hours > 24)
        return std::numeric_limits<double>::quiet_NaN();
    if (hours == 24 && (minutes || seconds))
        return std::numeric_limits<double>::quiet_NaN();
    if (minutes < 0 || minutes > 59)
        return std::numeric_limits<double>::quiet_NaN();
    if (seconds < 0 || seconds >= 61)
        return std::numeric_limits<double>::quiet_NaN();
    if (seconds > 60) {
        // Discard leap seconds by clamping to the end of a minute.
        seconds = 60;
    }
        
    double dateSeconds = ymdhmsToSeconds(year, month, day, hours, minutes, seconds) - timeZoneSeconds;
    return dateSeconds * msPerSecond;
}

// Odd case where 'exec' is allowed to be 0, to accomodate a caller in WebCore.
double parseDateFromNullTerminatedCharacters(const char* dateString, bool& haveTZ, int& offset)
{
    haveTZ = false;
    offset = 0;

    // This parses a date in the form:
    //     Tuesday, 09-Nov-99 23:12:40 GMT
    // or
    //     Sat, 01-Jan-2000 08:00:00 GMT
    // or
    //     Sat, 01 Jan 2000 08:00:00 GMT
    // or
    //     01 Jan 99 22:00 +0100    (exceptions in rfc822/rfc2822)
    // ### non RFC formats, added for Javascript:
    //     [Wednesday] January 09 1999 23:12:40 GMT
    //     [Wednesday] January 09 23:12:40 GMT 1999
    //
    // We ignore the weekday.
     
    // Skip leading space
    skipSpacesAndComments(dateString);

    long month = -1;
    const char *wordStart = dateString;
    // Check contents of first words if not number
    while (*dateString && !isASCIIDigit(*dateString)) {
        if (isASCIISpace(*dateString) || *dateString == '(') {
            if (dateString - wordStart >= 3)
                month = findMonth(wordStart);
            skipSpacesAndComments(dateString);
            wordStart = dateString;
        } else
           dateString++;
    }

    // Missing delimiter between month and day (like "January29")?
    if (month == -1 && wordStart != dateString)
        month = findMonth(wordStart);

    skipSpacesAndComments(dateString);

    if (!*dateString)
        return std::numeric_limits<double>::quiet_NaN();

    // ' 09-Nov-99 23:12:40 GMT'
    char* newPosStr;
    long day;
    if (!parseLong(dateString, &newPosStr, 10, &day))
        return std::numeric_limits<double>::quiet_NaN();
    dateString = newPosStr;

    if (!*dateString)
        return std::numeric_limits<double>::quiet_NaN();

    if (day < 0)
        return std::numeric_limits<double>::quiet_NaN();

    int year = 0;
    if (day > 31) {
        // ### where is the boundary and what happens below?
        if (*dateString != '/')
            return std::numeric_limits<double>::quiet_NaN();
        // looks like a YYYY/MM/DD date
        if (!*++dateString)
            return std::numeric_limits<double>::quiet_NaN();
        if (day <= std::numeric_limits<int>::min() || day >= std::numeric_limits<int>::max())
            return std::numeric_limits<double>::quiet_NaN();
        year = static_cast<int>(day);
        if (!parseLong(dateString, &newPosStr, 10, &month))
            return std::numeric_limits<double>::quiet_NaN();
        month -= 1;
        dateString = newPosStr;
        if (*dateString++ != '/' || !*dateString)
            return std::numeric_limits<double>::quiet_NaN();
        if (!parseLong(dateString, &newPosStr, 10, &day))
            return std::numeric_limits<double>::quiet_NaN();
        dateString = newPosStr;
    } else if (*dateString == '/' && month == -1) {
        dateString++;
        // This looks like a MM/DD/YYYY date, not an RFC date.
        month = day - 1; // 0-based
        if (!parseLong(dateString, &newPosStr, 10, &day))
            return std::numeric_limits<double>::quiet_NaN();
        if (day < 1 || day > 31)
            return std::numeric_limits<double>::quiet_NaN();
        dateString = newPosStr;
        if (*dateString == '/')
            dateString++;
        if (!*dateString)
            return std::numeric_limits<double>::quiet_NaN();
     } else {
        if (*dateString == '-')
            dateString++;

        skipSpacesAndComments(dateString);

        if (*dateString == ',')
            dateString++;

        if (month == -1) { // not found yet
            month = findMonth(dateString);
            if (month == -1)
                return std::numeric_limits<double>::quiet_NaN();

            while (*dateString && *dateString != '-' && *dateString != ',' && !isASCIISpace(*dateString))
                dateString++;

            if (!*dateString)
                return std::numeric_limits<double>::quiet_NaN();

            // '-99 23:12:40 GMT'
            if (*dateString != '-' && *dateString != '/' && *dateString != ',' && !isASCIISpace(*dateString))
                return std::numeric_limits<double>::quiet_NaN();
            dateString++;
        }
    }

    if (month < 0 || month > 11)
        return std::numeric_limits<double>::quiet_NaN();

    // '99 23:12:40 GMT'
    if (year <= 0 && *dateString) {
        if (!parseInt(dateString, &newPosStr, 10, &year))
            return std::numeric_limits<double>::quiet_NaN();
    }

    // Don't fail if the time is missing.
    long hour = 0;
    long minute = 0;
    long second = 0;
    if (!*newPosStr)
        dateString = newPosStr;
    else {
        // ' 23:12:40 GMT'
        if (!(isASCIISpace(*newPosStr) || *newPosStr == ',')) {
            if (*newPosStr != ':')
                return std::numeric_limits<double>::quiet_NaN();
            // There was no year; the number was the hour.
            year = -1;
        } else {
            // in the normal case (we parsed the year), advance to the next number
            dateString = ++newPosStr;
            skipSpacesAndComments(dateString);
        }

        parseLong(dateString, &newPosStr, 10, &hour);
        // Do not check for errno here since we want to continue
        // even if errno was set becasue we are still looking
        // for the timezone!

        // Read a number? If not, this might be a timezone name.
        if (newPosStr != dateString) {
            dateString = newPosStr;

            if (hour < 0 || hour > 23)
                return std::numeric_limits<double>::quiet_NaN();

            if (!*dateString)
                return std::numeric_limits<double>::quiet_NaN();

            // ':12:40 GMT'
            if (*dateString++ != ':')
                return std::numeric_limits<double>::quiet_NaN();

            if (!parseLong(dateString, &newPosStr, 10, &minute))
                return std::numeric_limits<double>::quiet_NaN();
            dateString = newPosStr;

            if (minute < 0 || minute > 59)
                return std::numeric_limits<double>::quiet_NaN();

            // ':40 GMT'
            if (*dateString && *dateString != ':' && !isASCIISpace(*dateString))
                return std::numeric_limits<double>::quiet_NaN();

            // seconds are optional in rfc822 + rfc2822
            if (*dateString ==':') {
                dateString++;

                if (!parseLong(dateString, &newPosStr, 10, &second))
                    return std::numeric_limits<double>::quiet_NaN();
                dateString = newPosStr;

                if (second < 0 || second > 59)
                    return std::numeric_limits<double>::quiet_NaN();
            }

            skipSpacesAndComments(dateString);

            if (strncasecmp(dateString, "AM", 2) == 0) {
                if (hour > 12)
                    return std::numeric_limits<double>::quiet_NaN();
                if (hour == 12)
                    hour = 0;
                dateString += 2;
                skipSpacesAndComments(dateString);
            } else if (strncasecmp(dateString, "PM", 2) == 0) {
                if (hour > 12)
                    return std::numeric_limits<double>::quiet_NaN();
                if (hour != 12)
                    hour += 12;
                dateString += 2;
                skipSpacesAndComments(dateString);
            }
        }
    }
    
    // The year may be after the time but before the time zone.
    if (isASCIIDigit(*dateString) && year == -1) {
        if (!parseInt(dateString, &newPosStr, 10, &year))
            return std::numeric_limits<double>::quiet_NaN();
        dateString = newPosStr;
        skipSpacesAndComments(dateString);
    }

    // Don't fail if the time zone is missing. 
    // Some websites omit the time zone (4275206).
    if (*dateString) {
        if (strncasecmp(dateString, "GMT", 3) == 0 || strncasecmp(dateString, "UTC", 3) == 0) {
            dateString += 3;
            haveTZ = true;
        }

        if (*dateString == '+' || *dateString == '-') {
            int o;
            if (!parseInt(dateString, &newPosStr, 10, &o))
                return std::numeric_limits<double>::quiet_NaN();
            dateString = newPosStr;

            if (o < -9959 || o > 9959)
                return std::numeric_limits<double>::quiet_NaN();

            int sgn = (o < 0) ? -1 : 1;
            o = abs(o);
            if (*dateString != ':') {
                if (o >= 24)
                    offset = ((o / 100) * 60 + (o % 100)) * sgn;
                else
                    offset = o * 60 * sgn;
            } else { // GMT+05:00
                ++dateString; // skip the ':'
                int o2;
                if (!parseInt(dateString, &newPosStr, 10, &o2))
                    return std::numeric_limits<double>::quiet_NaN();
                dateString = newPosStr;
                offset = (o * 60 + o2) * sgn;
            }
            haveTZ = true;
        } else {
            for (size_t i = 0; i < WTF_ARRAY_LENGTH(known_zones); ++i) {
                if (0 == strncasecmp(dateString, known_zones[i].tzName, strlen(known_zones[i].tzName))) {
                    offset = known_zones[i].tzOffset;
                    dateString += strlen(known_zones[i].tzName);
                    haveTZ = true;
                    break;
                }
            }
        }
    }

    skipSpacesAndComments(dateString);

    if (*dateString && year == -1) {
        if (!parseInt(dateString, &newPosStr, 10, &year))
            return std::numeric_limits<double>::quiet_NaN();
        dateString = newPosStr;
        skipSpacesAndComments(dateString);
    }

    // Trailing garbage
    if (*dateString)
        return std::numeric_limits<double>::quiet_NaN();

    // Y2K: Handle 2 digit years.
    if (year >= 0 && year < 100) {
        if (year < 50)
            year += 2000;
        else
            year += 1900;
    }
    
    return ymdhmsToSeconds(year, month + 1, day, hour, minute, second) * msPerSecond;
}

double parseDateFromNullTerminatedCharacters(const char* dateString)
{
    bool haveTZ;
    int offset;
    double ms = parseDateFromNullTerminatedCharacters(dateString, haveTZ, offset);
    if (std::isnan(ms))
        return std::numeric_limits<double>::quiet_NaN();

    // fall back to local timezone
    if (!haveTZ)
        offset = calculateLocalTimeOffset(ms, LocalTime).offset / msPerMinute; // ms value is in local time milliseconds.

    return ms - (offset * msPerMinute);
}

double timeClip(double t)
{
    if (!std::isfinite(t))
        return std::numeric_limits<double>::quiet_NaN();
    if (fabs(t) > maxECMAScriptTime)
        return std::numeric_limits<double>::quiet_NaN();
    return trunc(t);
}

// See http://tools.ietf.org/html/rfc2822#section-3.3 for more information.
String makeRFC2822DateString(unsigned dayOfWeek, unsigned day, unsigned month, unsigned year, unsigned hours, unsigned minutes, unsigned seconds, int utcOffset)
{
    StringBuilder stringBuilder;
    stringBuilder.append(weekdayName[dayOfWeek]);
    stringBuilder.appendLiteral(", ");
    stringBuilder.appendNumber(day);
    stringBuilder.append(' ');
    stringBuilder.append(monthName[month]);
    stringBuilder.append(' ');
    stringBuilder.appendNumber(year);
    stringBuilder.append(' ');

    appendTwoDigitNumber(stringBuilder, hours);
    stringBuilder.append(':');
    appendTwoDigitNumber(stringBuilder, minutes);
    stringBuilder.append(':');
    appendTwoDigitNumber(stringBuilder, seconds);
    stringBuilder.append(' ');

    stringBuilder.append(utcOffset > 0 ? '+' : '-');
    int absoluteUTCOffset = abs(utcOffset);
    appendTwoDigitNumber(stringBuilder, absoluteUTCOffset / 60);
    appendTwoDigitNumber(stringBuilder, absoluteUTCOffset % 60);

    return stringBuilder.toString();
}

} // namespace WTF
