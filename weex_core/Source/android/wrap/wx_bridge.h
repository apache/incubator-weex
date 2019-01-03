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

#ifndef WEEX_PROJECT_WX_BRIDGE_H
#define WEEX_PROJECT_WX_BRIDGE_H

#include <jni.h>
#include <map>
#include <set>
#include <string>
#include <vector>
#include "base/android/jni/scoped_java_ref.h"
#include "base/android/jni/jni_object_wrap.h"
#include "base/common.h"
#include "include/WeexApiHeader.h"

namespace WeexCore {
class WXCoreMargin;
class WXCorePadding;
class WXCoreBorderWidth;
class WXCoreSize;

class WXBridge : public JNIObjectWrap {
 public:
  static WXBridge *Instance() {
    if (g_instance == nullptr) {
      g_instance = new WXBridge();
    }
    return g_instance;
  }
  static bool RegisterJNIUtils(JNIEnv *env);
  base::android::ScopedLocalJavaRef<jobject> GetMeasureFunc(
      JNIEnv *env, const char *page_id, jlong render_object_ptr);
  int HasTransitionPros(
      JNIEnv *env, const char *page_id, const char *ref,
      const std::vector<std::pair<std::string, std::string>> &styles);
  int AppendTreeCreateFinish(JNIEnv *env, const char *page_id, const char *ref);
  int MoveElement(JNIEnv *env, const char *page_id, const char *ref,
                  const char *parent_ref, int index);
  int RemoveElement(JNIEnv *env, const char *page_id, const char *ref);
  int CreateFinish(JNIEnv *env, const char *page_id);
  int UpdateAttr(JNIEnv *env, const char *page_id, const char *ref,
                 std::vector<std::pair<std::string, std::string>> *attrs);
  int UpdateStyle(JNIEnv *env, const char *page_id, const char *ref,
                  std::vector<std::pair<std::string, std::string>> *style,
                  std::vector<std::pair<std::string, std::string>> *margin,
                  std::vector<std::pair<std::string, std::string>> *padding,
                  std::vector<std::pair<std::string, std::string>> *border);
  int Layout(JNIEnv *env, const char *page_id, const char *ref, int top,
             int bottom, int left, int right, int height, int width,
             bool isRTL, int index);
  int AddElement(JNIEnv *env, const char *page_id, const char *component_type,
                 const char *ref, int &index, const char *parentRef,
                 std::map<std::string, std::string> *styles,
                 std::map<std::string, std::string> *attributes,
                 std::set<std::string> *events, const WXCoreMargin &margins,
                 const WXCorePadding &paddings,
                 const WXCoreBorderWidth &borders, bool willLayout);
  int CreateBody(JNIEnv *env, const char *page_id, const char *component_type,
                 const char *ref, std::map<std::string, std::string> *styles,
                 std::map<std::string, std::string> *attributes,
                 std::set<std::string> *events, const WXCoreMargin &margins,
                 const WXCorePadding &paddings,
                 const WXCoreBorderWidth &borders);
  int RemoveEvent(JNIEnv *env, const char *page_id, const char *ref,
                  const char *event);
  int AddEvent(JNIEnv *env, const char *page_id, const char *ref,
               const char *event);
  int RefreshFinish(JNIEnv *env, const char *page_id, const char *task,
                    const char *callback);
  int RenderSuccess(JNIEnv *env, const char *page_id);
  int UpdateFinish(JNIEnv *env, const char *page_id, const char *task,
                   const char *callback);
  void SetTimeout(JNIEnv *env, const char *callback_id, const char *time);
  void CallNativeComponent(JNIEnv *env, const char *page_id, const char *ref,
                           const char *method, const char *arguments,
                           int arguments_length, const char *options,
                           int options_length);
  base::android::ScopedLocalJavaRef<jobject> CallNativeModule(
      JNIEnv *env, const char *page_id, const char *module, const char *method,
      const char *arguments, int arguments_length, const char *options,
      int options_length);
  int CallNative(JNIEnv *env, const char *page_id, const char *task,
                 const char *callback);
  void ReportException(JNIEnv *env, const char *page_id, const char *func,
                       const char *exception_string);
  void SetJSFrmVersion(JNIEnv *env, const char *version);
  void ReportServerCrash(JNIEnv *env, const char *instance_id,
                         const char *info);
  void ReportNativeInitStatus(JNIEnv *env, const char *statusCode,
                              const char *errorMsg);
  void OnReceivedResult(JNIEnv *env, long callback_id,
                        std::unique_ptr<WeexJSResult> &result);

  void reset_clazz(JNIEnv* env, const char* className);
 private:
  WXBridge() {}
  virtual ~WXBridge() {}
  static WXBridge *g_instance;
  DISALLOW_COPY_AND_ASSIGN(WXBridge);
};
}  // namespace WeexCore

#endif  // WEEX_PROJECT_WX_BRIDGE_H
