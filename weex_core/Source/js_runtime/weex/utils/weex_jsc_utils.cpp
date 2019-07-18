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
// Created by Darin on 11/02/2018.
//

#include "weex_jsc_utils.h"

#include <iostream>
#include <fstream>

#include "android/jsengine/object/weex_env.h"
#include "core/bridge/script_bridge.h"
#include "base/string_util.h"

 void printLogOnFileWithNameS(const char * name, const char *log) {
    std::string string("/data/data/com.alibaba.weex/");
    string.append(name);
    std::ofstream mcfile;
    mcfile.open(string.c_str(), std::ios::app);
    mcfile << log << std::endl;
    mcfile.close();
}

std::string jString2String(const uint16_t *str, size_t length) {
   return weex::base::to_utf8(const_cast<uint16_t *>(str), length);
}


std::string weexString2String(const WeexString *weexString) {
    if (weexString != nullptr && weexString->length > 0) {
        return jString2String(weexString->content, weexString->length);
    }

    return "";
}

std::string char2String(const char* str) {
   if(str == nullptr || strlen(str) == 0)
     return "";

   return str;
 }

WeexString *genWeexStringSS(const uint16_t *str, size_t length) {
    size_t byteSize = length * sizeof(uint16_t);
    auto *string = (WeexString *) malloc(byteSize + sizeof(WeexString));
    if (string == nullptr)
        return nullptr;

    memset(string, 0, byteSize + sizeof(WeexString));
    string->length = length;
    memcpy(string->content, str, byteSize);
    return string;
}
WeexByteArray *genWeexByteArraySS(const char *str, size_t strLen) {
    auto *ret = (WeexByteArray *) malloc(strLen + sizeof(WeexByteArray));

    if (ret == nullptr)
        return nullptr;

    memset(ret, 0, strLen + sizeof(WeexByteArray));

    ret->length = strLen;
    memcpy(ret->content, str, strLen);

    ret->content[strLen] = '\0';

    return const_cast<WeexByteArray *> (ret);
}
void freeInitFrameworkParams(std::vector<INIT_FRAMEWORK_PARAMS *> &params) {
    for (auto &param : params) {
        free(param->type);
        free(param->value);
        free(param);
    }
}

void freeParams(std::vector<VALUE_WITH_TYPE *> &params) {
    for (auto &param : params) {
        if (param->type == ParamsType::STRING ||
            param->type == ParamsType::JSONSTRING) {
            free(param->value.string);
        }
        if (param->type == ParamsType::BYTEARRAY) {
            free(param->value.byteArray);
        }
        free(param);
    }
}
void doUpdateGlobalSwitchConfig(const char *config) {
    if (!config) {
        return;
    }
    LOGE("doUpdateGlobalSwitchConfig %s", config);
    if (strstr(config, "wson_off") != NULL) {
        WeexEnv::getEnv()->setUseWson(false);
    } else {
        WeexEnv::getEnv()->setUseWson(true);
    }
}


uint64_t microTime() {
    struct timeval tv;

    gettimeofday(&tv, nullptr);

    return (((uint64_t) tv.tv_sec) * MICROSEC + tv.tv_usec);
}


int __atomic_inc(volatile int *ptr) {
    return __sync_fetch_and_add(ptr,1);
}
static int taskIdGenerator = 0;
int genTaskId(){
    return __atomic_inc(&taskIdGenerator);
}


WeexByteArray *IPCByteArrayToWeexByteArray(const IPCByteArray *byteArray) {
    return genWeexByteArraySS(byteArray->content, byteArray->length);
}


namespace WEEXICU {
    unique_fd::unique_fd(int fd)
            : m_fd(fd) {
    }

    unique_fd::~unique_fd() {
        close(m_fd);
    }

    int unique_fd::get() const {
        return m_fd;
    }
}
