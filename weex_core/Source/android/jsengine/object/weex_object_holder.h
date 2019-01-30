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
// Created by Darin on 10/04/2018.
//

#ifndef WEEXV8_WEEXOBJECTHOLDER_H
#define WEEXV8_WEEXOBJECTHOLDER_H

#include "android/jsengine/task/timer_queue.h"
#include "android/jsengine/object/weex_global_object.h"
#include "android/jsengine/weex_jsc_utils.h"

class WeexObjectHolder {

public:
    Strong<WeexGlobalObject> m_globalObject;

    VM* m_globalVM;

    std::map<std::string, WeexGlobalObject *> m_jsInstanceGlobalObjectMap;

    ~WeexObjectHolder();


    explicit WeexObjectHolder(VM* vm, TimerQueue* timeQueue, bool isMultiProgress);

    void initFromIPCArguments(IPCArguments *arguments, size_t startCount, bool forAppContext);

    void initFromParams(std::vector<INIT_FRAMEWORK_PARAMS *> &params, bool forAppContext);

    WeexGlobalObject *cloneWeexObject(std::string page_id, bool initContext, bool forAppContext);

    TimerQueue* timeQueue;

private:
    bool isMultiProgress;

};


#endif //WEEXV8_WEEXOBJECTHOLDER_H
