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

#include "android/bridge/platform/android_side.h"
#include "android/wrap/content_box_measurement_impl_android.h"
#include "base/android/jni/scoped_java_ref.h"
#include "core/manager/weex_core_manager.h"
#include "core/render/node/render_object.h"

namespace WeexCore {

  class MeasureFunctionAdapterImplAndroid : public MeasureFunctionAdapter {
  public:

    MeasureFunctionAdapterImplAndroid() {}

    ~MeasureFunctionAdapterImplAndroid() {}

    WXCoreSize Measure(const char* page_id, long render_ptr, float width,
                   MeasureMode widthMeasureMode, float height,
                   MeasureMode heightMeasureMode) {
      WXCoreSize size;
      size.height = 0;
      size.width = 0;

      jobject measureFunc = GetMeasureFuncFromComponent(page_id, render_ptr);

      if (render_ptr == 0 || measureFunc == nullptr) {
        return size;
      }

      JNIEnv* env = base::android::AttachCurrentThread();
      if (!env) return size;

      cumsmeasure_Imple_Android(env, measureFunc,
                                width, height,
                                widthMeasureMode, heightMeasureMode);
      size.width = GetLayoutWidth(env, measureFunc);
      size.height = GetLayoutHeight(env, measureFunc);

      env->DeleteLocalRef(measureFunc);

      return size;
    }

    void LayoutBefore(const char* page_id, long render_ptr) {
      jobject measureFunc = GetMeasureFuncFromComponent(page_id, render_ptr);
      if(nullptr == measureFunc) {
        return;
      }

        JNIEnv *env = base::android::AttachCurrentThread();
      if (!env) return;
      LayoutBeforeImplAndroid(env, measureFunc);
      env->DeleteLocalRef(measureFunc);
    }

    void LayoutAfter(const char* page_id, long render_ptr, float width, float height) {
      jobject measureFunc = GetMeasureFuncFromComponent(page_id, render_ptr);
      if(nullptr == measureFunc) {
        return;
      }
        JNIEnv *env = base::android::AttachCurrentThread();
      if (!env) return;
      LayoutAfterImplAndroid(env, measureFunc, width, height);
      env->DeleteLocalRef(measureFunc);
    }

   jobject GetMeasureFuncFromComponent(const char* page_id, long render_ptr) {
      return static_cast<AndroidSide*>(WeexCoreManager::Instance()->getPlatformBridge()->platform_side())->getMeasureFunc(page_id, render_ptr);
    }
  };
}
