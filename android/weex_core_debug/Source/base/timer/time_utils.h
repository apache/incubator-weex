//
//  timer_utils.h
//  samples
//
//  Created by dli on 11/2/16.
//
//

#ifndef WEEX_BASE_TIMER_TIME_UTILS_H_
#define WEEX_BASE_TIMER_TIME_UTILS_H_

#include <sys/time.h>

#if OS_IOS
#include <cstdint>
#endif

namespace base {
uint64_t CurrentTimeMillis();
timespec ToTimeSpecFromNow(uint64_t interval_time);
}  // namespace base

#endif  // WEEX_BASE_TIMER_TIME_UTILS_H_
