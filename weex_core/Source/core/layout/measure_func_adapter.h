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
#ifdef __cplusplus

#ifndef WEEX_CORE_MEASURE_FUNCTION_ADAPTER_H
#define WEEX_CORE_MEASURE_FUNCTION_ADAPTER_H


#include "layout.h"

namespace WeexCore {

  class MeasureFunctionAdapter {
  public:

    MeasureFunctionAdapter() {}

    ~MeasureFunctionAdapter() {}

    virtual WXCoreSize Measure(const char* page_id, long render_ptr, float width, MeasureMode widthMeasureMode,
                               float height, MeasureMode heightMeasureMode) = 0;
    virtual void LayoutBefore(const char* page_id, long render_ptr) = 0;

    virtual void LayoutAfter(const char* page_id, long render_ptr, float width, float height) = 0;

  };

}

#endif //WEEX_CORE_MEASURE_FUNCTION_ADAPTER_H
#endif //__cplusplus
