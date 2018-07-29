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
#include <android/base/string/string_utils.h>
#include <base/ViewUtils.h>
#include <android/jsengine/api/WeexJSCoreApi.h>
#include "android/utils/IPCStringResult.h"
#include "ExtendJSApi.h"

using namespace WeexCore;

WeexCore::FunType gCanvasFunc = nullptr;

WeexCore::FunTypeT3d t3dFunc = nullptr;

/**
* This class aim to extend JS Api
**/
ExtendJSApi::ExtendJSApi() {
}

namespace WeexCore {

  extern "C" void Inject_GCanvasFunc(FunType fp) {
      // TODO
    gCanvasFunc = fp;
    LOGE("weexjsc injectGCanvasFunc gCanvasFunc");
  }

  const char *callGCanvasFun(FunType fp, const char *conextId, int x, const char *args) {
    return fp(conextId, x, args);
  }

  extern "C" void Inject_T3dFunc(FunTypeT3d fp) {
    t3dFunc = fp;
    LOGE("weexjsc Inject_T3dFunc t3d Func");
  }

const char* weexCallT3dFunc(FunTypeT3d fp, int x, const char* args) {
  return fp(x, args);
}

}
