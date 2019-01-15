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

#ifndef WEEXV8_CALLJSONAPPCONTEXTTASK_H
#define WEEXV8_CALLJSONAPPCONTEXTTASK_H


#include "android/jsengine/task/weex_task.h"
#include "android/jsengine/object/args/exe_js_args.h"

class CallJsOnAppContextTask : public WeexTask {

public:
    CallJsOnAppContextTask(const String &instanceId, const String &func, std::vector<VALUE_WITH_TYPE *> &params);

    CallJsOnAppContextTask(const String &instanceId, const String &func, IPCArguments *arguments, size_t startCount);

    void run(WeexRuntime *runtime) override;
    std::string taskName() override { return " CallJsOnAppContextTask "; }


    ~CallJsOnAppContextTask() {
        delete exeJsArgs;
    }

private:
    String func;
    ExeJsArgs *exeJsArgs;
};


#endif //WEEXV8_CALLJSONAPPCONTEXTTASK_H
