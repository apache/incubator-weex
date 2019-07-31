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
#include <cstdint>
#include <string>
#include "render_performance.h"

namespace WeexCore {

  std::vector<int64_t> RenderPerformance::PrintPerformanceLog(PerformanceStage performanceStage) {

    std::vector<int64_t> ret(3);

    if (performanceStage == onFirstScreen) {
      firstScreenCallBridgeTime = callBridgeTime;
      firstScreenCssLayoutTime = cssLayoutTime;
      firstScreenParseJsonTime = parseJsonTime;
      ret[0] = firstScreenCallBridgeTime;
      ret[1] = firstScreenCssLayoutTime;
      ret[2] = firstScreenParseJsonTime;
    } else {
      onRenderSuccessCallBridgeTime = callBridgeTime;
      onRenderSuccessCssLayoutTime = cssLayoutTime;
      onRenderSuccessParseJsonTime = parseJsonTime;
      ret[0] = onRenderSuccessCallBridgeTime;
      ret[1] = onRenderSuccessCssLayoutTime;
      ret[2] = onRenderSuccessParseJsonTime;
    }

    return ret;
  }

    bool RenderPerformance::onInteractionTimeUpdate() {
      if (cssLayoutTimeForInteraction == cssLayoutTime){
          return false;
      }
      cssLayoutTimeForInteraction = cssLayoutTime;
        return true;
    }

    void RenderPerformance::getPerformanceStringData(std::map<std::string, std::string> &map) {
        map["wxLayoutTime"] = std::to_string(this->cssLayoutTimeForInteraction);
    }
}
