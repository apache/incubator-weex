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

#include "init_framework_task.h"

void InitFrameworkTask::run(WeexRuntime *runtime) {
    if (instanceId.isEmpty())
        runtime->initFramework(script, args->params);
    else
        runtime->initAppFramework(instanceId, script, args->params);
}

InitFrameworkTask::InitFrameworkTask(const String &script, std::vector<INIT_FRAMEWORK_PARAMS *> &params)
        : InitFrameworkTask("", script, params) {

}

InitFrameworkTask::InitFrameworkTask(const String &script, IPCArguments *arguments, size_t startCount)
        : InitFrameworkTask("", script, arguments, startCount) {

}

InitFrameworkTask::InitFrameworkTask(const String &instanceId, const String &script,
                                     std::vector<INIT_FRAMEWORK_PARAMS *> &params) : WeexTask(instanceId) {

    this->script = script;
    args = new InitFrameworkArgs(params);
}

InitFrameworkTask::InitFrameworkTask(const String &instanceId, const String &script, IPCArguments *arguments,
                                     size_t startCount) : WeexTask(instanceId) {
    this->script = script;
    args = new InitFrameworkArgs(arguments, startCount);
}

InitFrameworkTask::~InitFrameworkTask() {
    delete args;
}
InitFrameworkTask *InitFrameworkTask::clone() {
  return new InitFrameworkTask(this->script.utf8().data(), args->params);
}
