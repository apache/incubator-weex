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
// Created by Darin on 12/06/2018.
//

#ifndef WEEXV8_TIMERQUEUE_H
#define WEEXV8_TIMERQUEUE_H

#include <pthread.h>
#include <deque>

//#include "android/jsengine/task/timer_task.h"
#include "js_runtime/weex/task/timer_task.h"
#include "base/android/ThreadLocker.h"

class TimerTask;
class WeexTaskQueue;

class TimerQueue {

public:
    void init();

    void destroyPageTimer(std::string instanceId);

    void removeTimer(int timerId);

    int addTimerTask(TimerTask *timerTask);

    TimerTask *getTask();

    void start();

    bool isInit = false;

    explicit TimerQueue(WeexTaskQueue *taskQueue);

private:
    uint64_t nextTaskWhen;
    WeexTaskQueue *weexTaskQueue;
    std::deque<TimerTask *> timerQueue_;
    ThreadLocker threadLocker;
};


#endif //WEEXV8_TIMERQUEUE_H
