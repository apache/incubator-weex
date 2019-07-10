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

#ifndef WEEXV8_TIMERTASK_H
#define WEEXV8_TIMERTASK_H


#include <cstdint>
#include <string>
class TimerTask {

public:
    int taskId;

    std::string instanceID;
    uint32_t m_function;

    uint64_t timeout;
    uint64_t when;

    bool repeat = false;

    bool from_instance_ = true;

   // WeexGlobalObject* global_object_;

    explicit TimerTask(const std::string &id, uint32_t function, uint64_t millSecTimeout, bool repeat = false);

    explicit TimerTask(TimerTask *timerTask);

    TimerTask(TimerTask &timerTask);

};


#endif //WEEXV8_TIMERTASK_H
