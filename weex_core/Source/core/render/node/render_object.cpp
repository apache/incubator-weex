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
#include <core/manager/weex_core_manager.h>

using namespace std;
namespace WeexCore {

  RenderObject::RenderObject() {
    mStyles = new StylesMap();
    mAttributes = new AttributesMap();
    mEvents = new EventsSet();
    mIsRootRender = false;
  }

  RenderObject::~RenderObject() {

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

    if (WeexCoreManager::getInstance()->GetMeasureFunctionAdapter() == nullptr)
      return size;

    return WeexCoreManager::getInstance()->GetMeasureFunctionAdapter()->Measure(node, width, widthMeasureMode, height, heightMeasureMode);
  }

  void RenderObject::BindMeasureFunc() {
     setMeasureFunc(measureFunc_Impl);
  }

  void RenderObject::onLayoutBefore() {
    if (WeexCoreManager::getInstance()->GetMeasureFunctionAdapter() == nullptr)
      return;
    WeexCoreManager::getInstance()->GetMeasureFunctionAdapter()->LayoutBefore(this);
  }

  void RenderObject::onLayoutAfter(float width, float height) {
    if (WeexCoreManager::getInstance()->GetMeasureFunctionAdapter() == nullptr)
      return;
    WeexCoreManager::getInstance()->GetMeasureFunctionAdapter()->LayoutAfter(this, width, height);
  }
} //end WeexCore
