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
// Created by Darin on 29/05/2018.
//

#include "init_framework_args.h"
#include "android/jsengine/weex_jsc_utils.h"

InitFrameworkArgs::InitFrameworkArgs(std::vector<INIT_FRAMEWORK_PARAMS *> &params) {
    for (auto param : params) {
        auto init_framework_params = (INIT_FRAMEWORK_PARAMS *) malloc(sizeof(INIT_FRAMEWORK_PARAMS));

        if (init_framework_params == nullptr) {
            return;
        }

        memset(init_framework_params, 0, sizeof(INIT_FRAMEWORK_PARAMS));
        init_framework_params->type = genWeexByteArraySS(param->type->content, param->type->length);
        init_framework_params->value = genWeexByteArraySS(param->value->content, param->value->length);

        this->params.push_back(init_framework_params);
    }

}

InitFrameworkArgs::InitFrameworkArgs(IPCArguments *arguments, size_t startCount) {
    size_t count = arguments->getCount();

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



}

InitFrameworkArgs::~InitFrameworkArgs() {
    for (auto param : params) {
        free(param->type);
        free(param->value);
        free(param);
    }
    params.clear();
}
