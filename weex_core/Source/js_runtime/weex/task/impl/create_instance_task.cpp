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

#include "create_instance_task.h"

void CreateInstanceTask::addExtraArg(std::string arg) {
    extraArgs.push_back(arg);
}

CreateInstanceTask::CreateInstanceTask(const std::string &instanceId, const std::string &script, std::vector<INIT_FRAMEWORK_PARAMS*>& params) : WeexTask(instanceId) {
    this->script = script;
    initExtraArgs.reset(new InitFrameworkArgs(params));
}

void CreateInstanceTask::run(WeexRuntime *runtime) {
    if (extraArgs.size() < 4)
        return;

    runtime->createInstance(instanceId, extraArgs.at(0), this->script, extraArgs.at(1), extraArgs.at(2),
                            extraArgs.at(3), initExtraArgs->params);
}

