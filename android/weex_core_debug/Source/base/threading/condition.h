// Copyright 2017 The Weex Authors. All rights reserved.

#ifndef WEEX_BASE_THREADING_CONDITION_H_
#define WEEX_BASE_THREADING_CONDITION_H_

#include "lock.h"
#include "../timer/time_utils.h"

namespace base {
class Condition {
 public:
    Condition() {
        pthread_cond_init(&condition_, NULL);
    }

    ~Condition() {
        pthread_cond_destroy(&condition_);
    }

    void Wait() {
        AutoLock lock(lock_);
        pthread_cond_wait(&condition_, &lock_.mutex_);
    }

    void Wait(uint64_t time) {
        AutoLock lock(lock_);
        timespec next_time = ToTimeSpecFromNow(time);
        pthread_cond_timedwait(&condition_, &lock_.mutex_, &next_time);
    }

    void Singal() {
        AutoLock lock(lock_);
        pthread_cond_signal(&condition_);
    }

 private:
    Lock lock_;
    pthread_cond_t condition_;
};
}  // namespace base

#endif  // WEEX_BASE_THREADING_CONDITION_H_
