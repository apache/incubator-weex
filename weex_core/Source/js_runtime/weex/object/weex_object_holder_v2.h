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
// Created by chenpeihan on 2019/2/22.
//

#ifndef PROJECT_WEEX_OBJECT_HOLDER_V2_H
#define PROJECT_WEEX_OBJECT_HOLDER_V2_H

#include <memory>
#include <map>
#include "js_runtime/weex/task/timer_queue.h"
#include "weex_global_object_v2.h"


class WeexObjectHolderV2 {
public:
    std::unique_ptr<WeexGlobalObjectV2> globalObject;

    std::map<std::string, WeexGlobalObjectV2*> m_jsInstanceGlobalObjectMap;

    explicit WeexObjectHolderV2(unicorn::RuntimeVM *vm, TimerQueue *timeQueue, bool isMultiProgress);

    void initFromParams(std::vector<INIT_FRAMEWORK_PARAMS *> &params, bool forAppContext);

    WeexGlobalObjectV2 *createWeexObject();

    WeexGlobalObjectV2 *createInstancecObject(const std::string &id, const std::string &name);

    WeexGlobalObjectV2 *createAppWorkerObject();

    TimerQueue *timeQueue;

private:
    bool isMultiProgress;
    unicorn::RuntimeVM *vm;

};


#endif //PROJECT_WEEX_OBJECT_HOLDER_V2_H
