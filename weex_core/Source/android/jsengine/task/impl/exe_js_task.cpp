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

#include "exe_js_task.h"

void ExeJsTask::run(WeexRuntime *runtime) {
    if (extraArgs.size() < 2)
        return;

    if (callbackId >= 0) {
        runtime->exeJSWithCallback(instanceId, extraArgs.at(0), extraArgs.at(1),
                                 exeJsArgs->params, callbackId);
    } else if (!withResult) {
        runtime->exeJS(instanceId, extraArgs.at(0), extraArgs.at(1), exeJsArgs->params);
    } else {
        std::unique_ptr<WeexJSResult>  jsResult = runtime->exeJSWithResult(instanceId, extraArgs.at(0), extraArgs.at(1),
                                                                           exeJsArgs->params);

        if (future() != nullptr) {
            future()->setResult(jsResult);
        }
    }
}

void ExeJsTask::addExtraArg(String arg) {
    this->extraArgs.push_back(arg);
}

ExeJsTask::ExeJsTask(const String &instanceId, std::vector<VALUE_WITH_TYPE *> &params, bool withResult) : WeexTask(
        instanceId) {
    this->withResult = withResult;
    callbackId = -1;
    exeJsArgs = new ExeJsArgs(params);
}

ExeJsTask::ExeJsTask(const String &instanceId, std::vector<VALUE_WITH_TYPE *> &params, long callback_id) : WeexTask(
        instanceId) {
    this->withResult = true;
    callbackId = callback_id;
    exeJsArgs = new ExeJsArgs(params);
}

ExeJsTask::ExeJsTask(const String &instanceId, IPCArguments *arguments, size_t startCount, bool withResult) : WeexTask(
        instanceId) {
    this->withResult = withResult;
    callbackId = -1;
    this->exeJsArgs = new ExeJsArgs(arguments, startCount);
}

ExeJsTask::~ExeJsTask() {
    delete exeJsArgs;
}

ExeJsTask *ExeJsTask::clone() {
    ExeJsTask *task = new ExeJsTask(instanceId, this->exeJsArgs->params);
    for (int i = 0; i < this->extraArgs.size(); ++i) {
        task->addExtraArg(this->extraArgs[i]);
    }
    return task;
}
