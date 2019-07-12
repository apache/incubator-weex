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
//
// Created by Darin on 14/06/2018.
//

#include "ThreadLocker.h"

int ThreadLocker::lock() {
    return pthread_mutex_lock(&mutex_);
}

int ThreadLocker::unlock() {
    return pthread_mutex_unlock(&mutex_);
}

int ThreadLocker::wait() {
    return pthread_cond_wait(&condition_, &mutex_);
}


int ThreadLocker::waitTimeout(const uint64_t microSecWhen) {
    const struct timeval timeout = {static_cast<long>(microSecWhen / MICROSEC),
                                    static_cast<int>(microSecWhen % MICROSEC)};
    return waitTimeout(&timeout);
}

int ThreadLocker::waitTimeout(const struct timeval *tv) {
    struct timespec ts{tv->tv_sec, tv->tv_usec * TIMESPCE};
    return waitTimeout(&ts);
}

int ThreadLocker::waitTimeout(const struct timespec *timeout) {
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    return pthread_cond_timedwait(&condition_, &mutex_, timeout);
}

ThreadLocker::ThreadLocker() {
    pthread_mutex_init(&mutex_, nullptr);
    pthread_cond_init(&condition_, nullptr);
}

ThreadLocker::~ThreadLocker() {
    pthread_mutex_destroy(&mutex_);
    pthread_cond_destroy(&condition_);
}

int ThreadLocker::signal() {
    return pthread_cond_signal(&condition_);
}




