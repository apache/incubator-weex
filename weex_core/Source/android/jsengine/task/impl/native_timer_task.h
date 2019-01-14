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
// Created by Darin on 20/07/2018.
//

#ifndef WEEXV8_NATIVE_TIMERTASK_H
#define WEEXV8_NATIVE_TIMERTASK_H

#include "android/jsengine/task/weex_task.h"

class NativeTimerTask : public WeexTask {
public:
    explicit NativeTimerTask(const String &instanceId, uint32_t function, int taskId, bool one_shot);
    ~NativeTimerTask() override {}

    void run(WeexRuntime *runtime) override;
    std::string taskName() override { return "NativeTimerTask"; }
    uint32_t timerFunctionID() const { return timerFunction; }

private:
    uint32_t timerFunction;
    bool repeatTimer;
};


#endif //WEEXV8_TIMERTASK_H
