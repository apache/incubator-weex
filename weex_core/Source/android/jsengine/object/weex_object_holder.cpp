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
#include "android/jsengine/object/weex_object_holder.h"

#include "android/jsengine/object/weex_env.h"

void WeexObjectHolder::initFromIPCArguments(IPCArguments *arguments, size_t startCount, bool forAppContext) {
    size_t count = arguments->getCount();
    std::vector<INIT_FRAMEWORK_PARAMS *> params;

    for (size_t i = startCount; i < count; i += 2) {
        if (arguments->getType(i) != IPCType::BYTEARRAY) {
            continue;
        }
        if (arguments->getType(1 + i) != IPCType::BYTEARRAY) {
            continue;
        }
        const IPCByteArray *ba = arguments->getByteArray(1 + i);

        const IPCByteArray *ba_type = arguments->getByteArray(i);

        auto init_framework_params = (INIT_FRAMEWORK_PARAMS *) malloc(sizeof(INIT_FRAMEWORK_PARAMS));

        if (init_framework_params == nullptr) {
            return;
        }

        memset(init_framework_params, 0, sizeof(INIT_FRAMEWORK_PARAMS));

        init_framework_params->type = IPCByteArrayToWeexByteArray(ba_type);
        init_framework_params->value = IPCByteArrayToWeexByteArray(ba);

        params.push_back(init_framework_params);
    }

    initFromParams(params, forAppContext);
}

void WeexObjectHolder::initFromParams(std::vector<INIT_FRAMEWORK_PARAMS *> &params, bool forAppContext) {
    JSLockHolder locker(this->m_globalVM);
    VM& vm = *(this->m_globalVM);
    WeexGlobalObject *globalObject = WeexGlobalObject::create(vm, WeexGlobalObject::createStructure(
        vm, jsNull()));
    globalObject->initWxEnvironment(params, forAppContext, true);
    if (forAppContext)
        globalObject->initFunctionForAppContext();
    else
        globalObject->initFunction();

    globalObject->timeQueue = this->timeQueue;
    m_globalObject.set(vm, globalObject);
    vm.heap.setGarbageCollectionTimerEnabled(true);
}

WeexObjectHolder::WeexObjectHolder(VM* vm,  TimerQueue* timeQueue, bool isMultiProgress) {
    this->isMultiProgress = isMultiProgress;
    this->timeQueue = timeQueue;
    this->m_globalVM = vm;
}

WeexGlobalObject *WeexObjectHolder::cloneWeexObject(std::string page_id, bool initContext, bool forAppContext) {
    JSLockHolder locker(this->m_globalVM);
    VM& vm = *(this->m_globalVM);
    auto *temp_object = WeexGlobalObject::create(vm,
                                                 WeexGlobalObject::createStructure(vm, jsNull()));

    temp_object->initWxEnvironment(m_globalObject->m_initFrameworkParams, forAppContext, false);
    temp_object->timeQueue = this->timeQueue;
    if (forAppContext)
        temp_object->initFunctionForAppContext();
    else if (initContext)
        temp_object->initFunctionForContext();
    else
        temp_object->initFunction();

    temp_object->id = page_id;
    return temp_object;
}

WeexObjectHolder::~WeexObjectHolder() {
//    wson::destory();
    JSLockHolder locker(this->m_globalVM);
    this->m_globalVM->heap.collectAllGarbage();
}

