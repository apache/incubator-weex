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

#ifndef WEEXV8_THREADLOCKER_H
#define WEEXV8_THREADLOCKER_H


#include <pthread.h>

#define MICROSEC  ((uint64_t) 1e6)
#define TIMESPCE 1000

class ThreadLocker {


public:
    explicit ThreadLocker();

    ~ThreadLocker();

    int lock();

    int unlock();

    int wait();

    int signal();

    int waitTimeout(const uint64_t microSecWhen);

    int waitTimeout(const struct timeval *);

    int waitTimeout(const struct timespec *);


private:
    pthread_mutex_t mutex_;
    pthread_cond_t condition_;
};


#endif //WEEXV8_THREADLOCKER_H
