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

#include <jni.h>
#include "android/utils/so_utils.h"
#include "android/wrap/content_box_measurement_impl_android.h"
#include "android/wrap/hash_set.h"
#include "android/wrap/wx_debug_js_bridge.h"
#include "android/wrap/log_utils.h"
#include "android/wrap/native_render_object_utils_impl_android.h"
#include "android/wrap/wml_bridge.h"
#include "android/wrap/wx_bridge.h"
#include "android/wrap/wx_js_object.h"
#include "android/wrap/wx_map.h"
#include "base/android/jni/android_jni.h"
#include "base/android/jni_type.h"
#include "base/message_loop/message_pump_android.h"
#include "core/network/android/default_request_handler.h"

jint JNI_OnLoad(JavaVM *vm, void *reserved) {
  base::android::InitVM(vm);
  JNIEnv *env = base::android::AttachCurrentThread();
  bool result = WeexCore::RegisterJNIContentBoxMeasurement(env) &&
      WeexCore::RegisterWXDebugJsBridge(env) &&
                WeexCore::RegisterJNINativeRenderObjectUtils(env) &&
                weex::base::MessagePumpAndroid::RegisterJNIUtils(env) &&
                WeexCore::WXBridge::RegisterJNIUtils(env) &&
                base::android::JNIType::RegisterJNIUtils(env) &&
                WeexCore::WXJSObject::RegisterJNIUtils(env) &&
                WeexCore::LogUtils::RegisterJNIUtils(env) &&
                WeexCore::WXMap::RegisterJNIUtils(env) &&
                WeexCore::HashSet::RegisterJNIUtils(env) &&
                weex::core::network::DefaultRequestHandler::RegisterJNIUtils(env);
  if (result) {
    WeexCore::SoUtils::Init(env);
    WeexCore::WMLBridge::RegisterJNIUtils(env);
  }

  return result ? JNI_VERSION_1_4 : JNI_FALSE;
}

void JNI_OnUnload(JavaVM *vm, void *reserved) {}
