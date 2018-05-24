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
#include <core/render/node/render_object.h>
#include <android/bridge/impl/content_box_measurement_impl_android.h>
#include <android/bridge/impl/measure_mode_impl_android.h>
#include <android/base/jni/scoped_java_ref.h>
#include <android/bridge/impl/weexcore_impl_android.h>
#include <android/base/string/string_utils.h>
#include <string.h>

using namespace std;
namespace WeexCore {

  RenderObject::RenderObject() {
    mStyles = new StylesMap();
    mAttributes = new AttributesMap();
    mEvents = new EventsSet();
    mIsRootRender = false;
  }

  RenderObject::~RenderObject() {

    JNIEnv *env = getJNIEnv();

    mParentRender = nullptr;

    if (mStyles != nullptr) {
      delete mStyles;
      mStyles = nullptr;
    }
    if (mAttributes != nullptr) {
      delete mAttributes;
      mAttributes = nullptr;
    }
    if (mEvents != nullptr) {
      delete mEvents;
      mEvents = nullptr;
    }

    for(auto it = ChildListIterBegin(); it != ChildListIterEnd(); it++) {
      RenderObject* child = static_cast<RenderObject*>(*it);
      if (child != nullptr) {
        delete child;
        child = nullptr;
      }
    }
  }

  void RenderObject::ApplyDefaultStyle() {
    std::map<std::string, std::string> *style = GetDefaultStyle();

    if (style == nullptr)
      return;

    for (auto iter = style->cbegin(); iter != style->cend(); iter++)
      AddStyle(iter->first, iter->second);

    if (style != nullptr) {
      delete style;
      style = nullptr;
    }
  }

  void RenderObject::ApplyDefaultAttr() {
    std::map<std::string, std::string> *attrs = GetDefaultAttr();

    if (attrs == nullptr)
      return;

    for (auto iter = attrs->cbegin(); iter != attrs->cend(); iter++) {
        UpdateAttr(iter->first, iter->second);
    }

    if (attrs != nullptr) {
      delete attrs;
      attrs = nullptr;
    }
  }

  WXCoreSize measureFunc_Impl(WXCoreLayoutNode *node, float width, MeasureMode widthMeasureMode,
                              float height, MeasureMode heightMeasureMode) {
    WXCoreSize size;
    size.height = 0;
    size.width = 0;

    jobject measureFunc = ((RenderObject *) node)->GetMeasureFuncImplAndroid();

    if (node == nullptr || measureFunc == nullptr) {
      return size;
    }

    JNIEnv *env = getJNIEnv();

    int widthMode = Unspecified(env);
    int heightMode = Unspecified(env);
    if (widthMeasureMode == kExactly)
      widthMode = Exactly(env);
    if (heightMeasureMode == kExactly)
      heightMode = Exactly(env);
    cumsmeasure_Imple_Android(env, measureFunc,
                              width, height,
                              widthMode, heightMode);
    size.width = GetLayoutWidth(env, measureFunc);
    size.height = GetLayoutHeight(env, measureFunc);

    env->DeleteLocalRef(measureFunc);

    return size;
  }

  void RenderObject::BindMeasureFuncImplAndroid() {
     setMeasureFunc(measureFunc_Impl);
  }

  void RenderObject::BindMeasureFuncImplIOS(WXCoreMeasureFunc measureFunc_impl_ios) {
    setMeasureFunc(measureFunc_impl_ios);
  }

  void RenderObject::onLayoutBefore() {
    jobject measureFunc = this->GetMeasureFuncImplAndroid();
    if(nullptr == measureFunc) {
       return;
    }

    JNIEnv *env = getJNIEnv();
    LayoutBeforeImplAndroid(env, measureFunc);
    env->DeleteLocalRef(measureFunc);
  }

  void RenderObject::onLayoutAfter(float width, float height) {
    jobject measureFunc = this->GetMeasureFuncImplAndroid();
    if(nullptr == measureFunc) {
       return;
    }
    JNIEnv *env = getJNIEnv();
    LayoutAfterImplAndroid(env, measureFunc, width, height);
    env->DeleteLocalRef(measureFunc);
  }
} //end WeexCore
