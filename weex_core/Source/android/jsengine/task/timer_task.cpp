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

#include "timer_task.h"
#include "android/jsengine/weex_jsc_utils.h"

TimerTask::TimerTask(String id, uint32_t function, uint64_t millSecTimeout, WeexGlobalObject* global_object, bool repeat) {
    this->taskId = genTaskId();
    this->instanceID = id;
    this->m_function = function;
    this->timeout = millSecTimeout;
    this->when = microTime() + millSecTimeout * TIMESPCE;
    this->repeat = repeat;
    this->global_object_ = global_object;
}

TimerTask::TimerTask(TimerTask *timerTask) {
    this->taskId = timerTask->taskId;
    this->instanceID = timerTask->instanceID;
    this->repeat = timerTask->repeat;
    this->timeout = timerTask->timeout;
    this->m_function = timerTask->m_function;
    this->when = microTime() + timerTask->timeout * TIMESPCE;
    this->global_object_ = timerTask->global_object_;
}

TimerTask::TimerTask(TimerTask &timerTask) {
    this->taskId = timerTask.taskId;
    this->instanceID = timerTask.instanceID;
    this->repeat = timerTask.repeat;
    this->timeout = timerTask.timeout;
    this->m_function = timerTask.m_function;
    this->when = microTime() + timerTask.timeout * TIMESPCE;
    this->global_object_ = timerTask.global_object_;
}
