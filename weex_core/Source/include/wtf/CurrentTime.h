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
#ifndef CurrentTime_h
#define CurrentTime_h

#include <chrono>
#include <time.h>

namespace WTF {

// Returns the current UTC time in seconds, counted from January 1, 1970.
// Precision varies depending on platform but is usually as good or better
// than a millisecond.
// Use this function only if wall clock time is needed. For elapsed time
// measurement use monotonicallyIncreasingTime() instead.
WTF_EXPORT_PRIVATE double currentTime();

// Same thing, in milliseconds.
inline double currentTimeMS()
{
    return currentTime() * 1000.0;
}

// Provides a monotonically increasing time in seconds since an arbitrary point in the past.
// On unsupported platforms, this function only guarantees the result will be non-decreasing.
// Result of this function increases monotonically even when clock time goes back due to
// NTP or manual adjustments, so it is better suited for elapsed time measurement.
WTF_EXPORT_PRIVATE double monotonicallyIncreasingTime();

inline double monotonicallyIncreasingTimeMS()
{
    return monotonicallyIncreasingTime() * 1000.0;
}

// Returns the current CPU time of the current thread.
// Precision varies depending on platform but is usually as good or better
// than a millisecond.
WTF_EXPORT_PRIVATE std::chrono::microseconds currentCPUTime();

WTF_EXPORT_PRIVATE void sleep(double);

inline void sleepMS(double value)
{
    sleep(value / 1000.0);
}

} // namespace WTF

using WTF::currentCPUTime;
using WTF::currentTime;
using WTF::currentTimeMS;
using WTF::monotonicallyIncreasingTime;
using WTF::monotonicallyIncreasingTimeMS;
using WTF::sleep;
using WTF::sleepMS;

#endif // CurrentTime_h
