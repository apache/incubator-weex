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

#ifndef CORE_BRIDGE_EAGLE_BRIDGE_EXT_H
#define CORE_BRIDGE_EAGLE_BRIDGE_EXT_H

#if defined __cplusplus

#include <string>
#include <map>
#include <functional>
#include <set>
#include <mutex>

namespace WeexCore {
namespace EagleExt {

//this must stay in weex because weex_core_manager.h has inline static which can't be compiled into Eagle so.

std::function<void(const char*, const char*)> CreatePageDownloadExec(const char* instanceId,
                                         const char* func,
                                         const char* script,
                                         int script_length,
                                         const char* opts,
                                         const char* initData,
                                         const char* extendsApi);

#ifdef OS_ANDROID
void RefreshPageEagle(const char* page_id, const char* init_data);
#endif

}
}

#endif
#endif
