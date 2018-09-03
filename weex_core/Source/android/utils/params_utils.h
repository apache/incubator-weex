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

#ifndef WEEX_PROJECT_PARAMS_UTILS_H
#define WEEX_PROJECT_PARAMS_UTILS_H

#include <jni.h>
#include <Serializing/IPCSerializer.h>
#include "android/wrap/wx_js_object.h"
#include "include/WeexApiHeader.h"

namespace WeexCore {

WeexByteArray* genWeexByteArray(const char* str, size_t strLen);

INIT_FRAMEWORK_PARAMS* genInitFrameworkParams(const char* type,
                                              const char* value);

WeexString* genWeexString(const uint16_t* str, size_t length);

WeexString* jstring2WeexString(JNIEnv* env, jstring fromJString);

VALUE_WITH_TYPE* getValueWithTypePtr();
void addParamsToIPCSerializer(IPCSerializer *serializer, VALUE_WITH_TYPE* param);
std::vector<INIT_FRAMEWORK_PARAMS*> initFromParam(
    JNIEnv* env, jobject params,
    const std::function<void(const char*, const char*)>&
        ReportNativeInitStatus);

bool isSingleProcess();

jstring getJsonData(JNIEnv* env, jobjectArray jargs, int index);

void addParamsFromJArgs(std::vector<VALUE_WITH_TYPE*>& params,
                        VALUE_WITH_TYPE* param, JNIEnv* env,
                        std::unique_ptr<WXJSObject>& wx_js_object);
void freeValueWithType(VALUE_WITH_TYPE* params);
void freeParams(std::vector<VALUE_WITH_TYPE*>& params);
void freeParams(std::vector<InitFrameworkParams*>& params);
}  // namespace WeexCore

#endif  // WEEX_PROJECT_PARAMS_UTILS_H
