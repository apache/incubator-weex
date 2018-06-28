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
#ifndef WEEX_PROJECT_WXPERFORMANCE_H
#define WEEX_PROJECT_WXPERFORMANCE_H

#include <vector>

namespace WeexCore {

  typedef enum PerformanceStage {
    onFirstScreen, onRenderSuccess,
  } PerformanceStage;

  class RenderPerformance {

  public:

    int64_t callBridgeTime;

    int64_t cssLayoutTime;

    int64_t parseJsonTime;

    int64_t firstScreenCallBridgeTime;

    int64_t firstScreenCssLayoutTime;

    int64_t firstScreenParseJsonTime;

    int64_t onRenderSuccessCallBridgeTime;

    int64_t onRenderSuccessCssLayoutTime;

    int64_t onRenderSuccessParseJsonTime;

    RenderPerformance() : callBridgeTime(0), cssLayoutTime(0), parseJsonTime(0),
                          firstScreenCallBridgeTime(0), firstScreenCssLayoutTime(0),
                          firstScreenParseJsonTime(0), onRenderSuccessCallBridgeTime(0),
                          onRenderSuccessCssLayoutTime(0), onRenderSuccessParseJsonTime(0) {}

    std::vector<int64_t> PrintPerformanceLog(PerformanceStage performanceStage);
  };
}

#endif //WEEX_PROJECT_WXPERFORMANCE_H
