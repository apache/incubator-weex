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

#ifndef WEEXV8_INITFRAMEWORKTASK_H
#define WEEXV8_INITFRAMEWORKTASK_H


#include "android/jsengine/task/weex_task.h"
#include "android/jsengine/object/args/init_framework_args.h"

class InitFrameworkTask : public WeexTask {
public:


    explicit InitFrameworkTask(const String &instanceId, const String &script,
                               std::vector<INIT_FRAMEWORK_PARAMS *> &params);

    explicit InitFrameworkTask(const String &script, std::vector<INIT_FRAMEWORK_PARAMS *> &params);


    explicit InitFrameworkTask(const String &instanceId, const String &script, IPCArguments *arguments,
                               size_t startCount);

    explicit InitFrameworkTask(const String &script, IPCArguments *arguments, size_t startCount);

    void run(WeexRuntime *runtime) override;

    std::string taskName() override { return "InitFrameworkTask"; }

    InitFrameworkTask *clone();

    ~InitFrameworkTask();

private:
    InitFrameworkArgs *args;
    String script;
};


#endif //WEEXV8_INITFRAMEWORKTASK_H
