

#include "time_utils.h"

#if IOS
#include <cstdint>
#endif

namespace base {

    uint64_t CurrentTimeMillis() {
    timeval time;
    gettimeofday(&time, nullptr);
        uint64_t time_millis =  (time.tv_sec * 1000) + (time.tv_usec / 1000);
    return time_millis;
}

timespec ToTimeSpecFromNow(uint64_t interval_time) {
    timespec out_time;
    struct timeval now;
    uint64_t absmsec;
    gettimeofday(&now, NULL);
    absmsec = now.tv_sec * 1000ll + now.tv_usec / 1000ll;
    absmsec += interval_time;

    out_time.tv_sec = absmsec / 1000ll;
    out_time.tv_nsec = absmsec % 1000ll * 1000000ll;
    return out_time;
}
}  //namespace base