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
// Created by Darin on 23/05/2018.
//

#ifndef WEEXV8_WEEXTASKQUEUE_H
#define WEEXV8_WEEXTASKQUEUE_H


#include <deque>
#include "android/jsengine/task/weex_task.h"

class WeexTaskQueue {

public:
    ~WeexTaskQueue();
    explicit WeexTaskQueue(bool isMultiProgress = true);
    void run(WeexTask *task);

    int addTask(WeexTask *task);

    WeexTask *getTask();

    void removeTimer(int taskId);

    void removeAllTask(String id);

    int addTimerTask(String id, uint32_t function, int taskId, WeexGlobalObject* global_object, bool one_shot);

    void start();

    void init();

    bool isInitOk = false;

public:
    WeexRuntime *weexRuntime;
    bool isMultiProgress;

private:
    int _addTask(WeexTask *task, bool front);

    std::deque<WeexTask *> taskQueue_;
    ThreadLocker threadLocker;
};


#endif //WEEXV8_WEEXTASKQUEUE_H
