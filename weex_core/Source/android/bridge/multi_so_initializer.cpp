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

#include "multi_so_initializer.h"

#include <dlfcn.h>

#include "android/utils/so_utils.h"
#include "base/android/log_utils.h"

namespace WeexCore {

bool MultiSoInitializer::Init(
    const std::function<bool(void *)> &OnInitFinished,
    const std::function<void(const char *, const char *)>
        &ReportNativeInitStatus) {
  std::string soPath = "";

  // -----------------------------------------------
  // use find path to get lib path
  // use set path is better idea, should change in future
  if (SoUtils::jss_so_path() != nullptr) {
    soPath = SoUtils::jss_so_path();
  }

  if (soPath.empty()) {
    soPath = SoUtils::FindLibJssSoPath();
  }

  LOGD("final executablePath:%s", soPath.c_str());
  SoUtils::updateSoLinkPath(SoUtils::lib_ld_path());
  void *handle = dlopen(soPath.c_str(), RTLD_NOW);
  if (!handle) {
    const char *error = dlerror();
    LOGE("load %s failed,error=%s\n", SoUtils::jss_so_name(), error);
    ReportNativeInitStatus("-1005", error);
    // try again use current path
    //dlclose(handle);
    return false;
  }

  // clear dlopen error message
  dlerror();

  if (!OnInitFinished(handle)) {
    const char *error = dlerror();
    LOGE("load External_InitFrameWork failed,error=%s\n", error);
    ReportNativeInitStatus("-1006", error);
    dlclose(handle);
    ReportNativeInitStatus("-1007", "Init Functions failed");
    return false;
  }
  return true;
}
}  // namespace WeexCore
