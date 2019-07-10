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

#include "native_timer_task.h"
#include "android/jsengine/object/weex_env.h"
#include "js_runtime/weex/object/weex_runtime.h"

NativeTimerTask::NativeTimerTask(const std::string &instanceId,
                                 uint32_t function,
                                 int taskId,
                                 bool one_shot)
    : WeexTask(instanceId, taskId) {
  this->timerFunction = function;
  repeatTimer = !one_shot;
}

void NativeTimerTask::run(WeexRuntime *runtime) {
  runtime->exeTimerFunctionForRunTimeApi(instanceId, timerFunction, is_from_instance);
  if (!repeatTimer)
    runtime->removeTimerFunctionForRunTimeApi(instanceId, timerFunction, is_from_instance);

}
