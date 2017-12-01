#ifndef WEEX_PROJECT_TIMEUTILS_H
#define WEEX_PROJECT_TIMEUTILS_H

#include <sys/time.h>

static long long getCurrentTime() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return (long long) tv.tv_sec * 1000 + (long long) tv.tv_usec / 1000;
}

#endif //WEEX_PROJECT_TIMEUTILS_H
