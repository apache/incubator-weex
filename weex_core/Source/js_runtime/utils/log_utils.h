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
// Created by chenpeihan on 2019/1/17.
//

#ifndef WEEXCORE_LOG_UTILS_H
#define WEEXCORE_LOG_UTILS_H

#include "base/utils/log_utils.h"

#ifdef LOG_JS_RUNTIME_DEBUG
#define LOG_JS_RUNTIME(...) __android_log_print(ANDROID_LOG_WARN,"[JS_RUNTIME]", __VA_ARGS__)
#else
#define LOG_JS_RUNTIME(...) ((void)0)
#endif


#ifdef LOG_WEEX_RUNTIME_DEBUG
#define LOG_RUNTIME(...) __android_log_print(ANDROID_LOG_WARN,"[WEEX_RUNTIME]", __VA_ARGS__)
#else
#define LOG_RUNTIME(...) ((void)0)
#endif

#ifdef LOG_CONVERSION_DEBUG
#define LOG_CONVERSION(...)  __android_log_print(ANDROID_LOG_DEBUG,"[CONVERSION]", __VA_ARGS__)
#else
#define LOG_CONVERSION(...)  ((void)0)
#endif

#ifdef LOG_WEEX_BINDING_DEBUG
#define LOG_WEEX_BINDING(...) __android_log_print(ANDROID_LOG_WARN,"[WEEX_BINDING]", __VA_ARGS__)
#else
#define LOG_WEEX_BINDING(...) ((void)0)
#endif

#ifdef LOG_JS_DEBUG
#define LOG_JS(...)  LOGI(__VA_ARGS__)
#else
#define LOG_JS(...)  ((void)0)
#endif

#define LOG_JS_ERROR(...) LOGE(__VA_ARGS__)


#endif //WEEXCORE_LOG_UTILS_H
