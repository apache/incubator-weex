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
#include "measure_func_adapter.h"
#include <core/render/node/render_object.h>
#include <android/bridge/impl/content_box_measurement_impl_android.h>
#include <android/bridge/impl/measure_mode_impl_android.h>
#include <android/base/jni/scoped_java_ref.h>

namespace WeexCore {


  class MeasureFunctionAdapterImplIOS : public MeasureFunctionAdapter {
  public:

    MeasureFunctionAdapterImplIOS() {}

    ~MeasureFunctionAdapterImplIOS() {}

    inline WXCoreSize Measure(WXCoreLayoutNode *node, float width,
                       MeasureMode widthMeasureMode, float height,
                       MeasureMode heightMeasureMode) {
      WXCoreSize size;
      size.height = 0;
      size.width = 0;

      // TODO

      return size;
    }

    inline void LayoutBefore(WXCoreLayoutNode *node) {

    }

    inline void LayoutAfter(WXCoreLayoutNode *node, float width, float height) {

    }
  };
}
