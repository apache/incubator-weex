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
#include "android/weex_extend_js_api.h"

#include "android/base/string/string_utils.h"
#include "base/android/log_utils.h"
#include "core/common/view_utils.h"

using namespace WeexCore;

WeexCore::FunType gCanvasFunc = nullptr;

WeexCore::FunTypeT3d t3dFunc = nullptr;

namespace WeexCore {

  extern "C" void Inject_GCanvasFunc(FunType fp) {
    gCanvasFunc = fp;
    LOGE("weexjsc injectGCanvasFunc gCanvasFunc");
  }

  const char *CallGCanvasFun(const char *conextId, int x, const char *args) {
    if(gCanvasFunc != nullptr){
      LOGE("CallGCanvasFun");
      return gCanvasFunc(conextId, x, args);
    } else {
      return nullptr;
    }
  }

  extern "C" void Inject_T3dFunc(FunTypeT3d fp) {
    t3dFunc = fp;
    LOGE("weexjsc Inject_T3dFunc t3d Func");
  }

const char* CallT3dFunc(int x, const char* args) {
 if(t3dFunc != nullptr) {
    return t3dFunc(x, args);
 } else {
    return nullptr;
 }
}

}
