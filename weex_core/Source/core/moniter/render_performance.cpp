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
#include "render_performance.h"
#include <android/base/log_utils.h>

namespace WeexCore {

  int
  RenderPerformance::PrintPerformanceLogImplAndroid(const PerformanceStage &performanceStage) {

#if PERFORMANCE_LOG

    if (performanceStage == onFirstScreen) {
      LOGD("[WeexCore render time]onFirstScreen");
    } else {
      LOGD("[WeexCore render time]onRenderSuccess");
    }
    LOGD("[WeexCore render time]      jniCallTime: %lld", jniCallTime);
    LOGD("[WeexCore render time]        -addElementActionJNITime: %lld", addElementActionJNITime);
    LOGD("[WeexCore render time]         layoutActionJNITime: %lld", layoutActionJniTime);
    LOGD("[WeexCore render time]        -jniCallBridgeTime: %lld", jniCallBridgeTime);
    LOGD("[WeexCore render time]         createJMapJNITime: %lld", createJMapJNITime);
    LOGD("[WeexCore render time]      cssLayoutTime: %lld", cssLayoutTime);
    LOGD("[WeexCore render time]      parseJsonTime: %lld", parseJsonTime);
    LOGD("[WeexCore render time]      buildRenderObjectTime: %lld", buildRenderObjectTime);
#endif

    return cssLayoutTime;
  }

  int RenderPerformance::PrintPerformanceLogImplIOS(const PerformanceStage &performanceStage) {
    return 0;
  }
}
