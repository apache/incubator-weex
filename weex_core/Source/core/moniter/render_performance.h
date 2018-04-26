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

namespace WeexCore {

  typedef enum PerformanceStage {
    onFirstScreen, onRenderSuccess,
  } PerformanceStage;

  class RenderPerformance {

  public:

    long long jniCallTime;

    long long jniCallBridgeTime;

    long long cssLayoutTime;

    long long addElementActionJNITime;

    long long addEventActionJNITime;

    long long removeEventActionJNITime;

    long long layoutActionJniTime;

    long long parseJsonTime;

    long long buildRenderObjectTime;

    long long createJMapJNITime;

    RenderPerformance() : jniCallTime(0), jniCallBridgeTime(0), cssLayoutTime(0),
                          addElementActionJNITime(0),
                          layoutActionJniTime(0), parseJsonTime(0), buildRenderObjectTime(0),
                          createJMapJNITime(0) {}

    inline int PrintPerformanceLog(PerformanceStage performanceStage) {
      return PrintPerformanceLogImplAndroid(performanceStage);
    }

  private:

    int PrintPerformanceLogImplAndroid(const PerformanceStage &performanceStage);

    int PrintPerformanceLogImplIOS(const PerformanceStage &performanceStage);
  };
}

#endif //WEEX_PROJECT_WXPERFORMANCE_H
