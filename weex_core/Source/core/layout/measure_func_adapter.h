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
#ifndef WEEX_CORE_MEASURE_FUNCTION_H
#define WEEX_CORE_MEASURE_FUNCTION_H


#include "layout.h"

namespace WeexCore {

  class MeasureFunctionAdapter {
  public:

    MeasureFunctionAdapter() {}

    ~MeasureFunctionAdapter() {}

    virtual WXCoreSize Measure(WXCoreLayoutNode *node, float width, MeasureMode widthMeasureMode,
                                      float height, MeasureMode heightMeasureMode) = 0;

    virtual void LayoutBefore(WXCoreLayoutNode *node) = 0;

    virtual void LayoutAfter(WXCoreLayoutNode *node, float width, float height) = 0;

  private:
  };

}

#endif //WEEX_CORE_MEASURE_FUNCTION_H
