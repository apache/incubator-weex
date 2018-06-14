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
#include "core/render/node/render_object.h"
#include "base/ViewUtils.h"
#include "core/css/constants_name.h"
#include "core/css/constants_value.h"
#include "core/css/css_value_getter.h"
#include "core/layout/layout.h"
#include "core/manager/weex_core_manager.h"
#include "core/render/manager/render_manager.h"
#include "core/render/page/render_page.h"

namespace WeexCore {

RenderObject::RenderObject() {
  this->styles_ = new std::map<std::string, std::string>();
  this->attributes_ = new std::map<std::string, std::string>();
  this->events_ = new std::set<std::string>();
  this->is_root_render_ = false;
}

RenderObject::~RenderObject() {
  this->parent_render_ = nullptr;

  if (this->styles_ != nullptr) {
    delete this->styles_;
    this->styles_ = nullptr;
  }

  if (this->attributes_ != nullptr) {
    delete this->attributes_;
    this->attributes_ = nullptr;
  }

  if (this->events_ != nullptr) {
    delete this->events_;
    this->events_ = nullptr;
  }

  for (auto it = ChildListIterBegin(); it != ChildListIterEnd(); it++) {
    RenderObject *child = static_cast<RenderObject *>(*it);
    if (child != nullptr) {
      delete child;
      child = nullptr;
    }
  }
}

void RenderObject::ApplyDefaultStyle() {
  std::map<std::string, std::string> *style = GetDefaultStyle();

  if (style == nullptr) return;

  for (auto iter = style->cbegin(); iter != style->cend(); iter++)
    AddStyle(iter->first, iter->second);

  if (style != nullptr) {
    delete style;
    style = nullptr;
  }
}

void RenderObject::ApplyDefaultAttr() {
  std::map<std::string, std::string> *attrs = GetDefaultAttr();

  if (attrs == nullptr) return;

  for (auto iter = attrs->cbegin(); iter != attrs->cend(); iter++) {
    UpdateAttr(iter->first, iter->second);
  }

  if (attrs != nullptr) {
    delete attrs;
    attrs = nullptr;
  }
}

WXCoreSize measureFunc_Impl(WXCoreLayoutNode *node, float width,
                            MeasureMode widthMeasureMode, float height,
                            MeasureMode heightMeasureMode) {
  WXCoreSize size;
  size.height = 0;
  size.width = 0;

  if (WeexCoreManager::getInstance()->GetMeasureFunctionAdapter() == nullptr)
    return size;

  return WeexCoreManager::getInstance()->GetMeasureFunctionAdapter()->Measure(
      node, width, widthMeasureMode, height, heightMeasureMode);
}

void RenderObject::BindMeasureFunc() { setMeasureFunc(measureFunc_Impl); }

void RenderObject::OnLayoutBefore() {
  if (WeexCoreManager::getInstance()->GetMeasureFunctionAdapter() == nullptr)
    return;
  WeexCoreManager::getInstance()->GetMeasureFunctionAdapter()->LayoutBefore(
      this);
}

void RenderObject::OnLayoutAfter(float width, float height) {
  if (WeexCoreManager::getInstance()->GetMeasureFunctionAdapter() == nullptr)
    return;
  WeexCoreManager::getInstance()->GetMeasureFunctionAdapter()->LayoutAfter(
      this, width, height);
}

StyleType RenderObject::ApplyStyle(const std::string &key,
                                   const std::string &value,
                                   const bool updating) {
  bool insert = false;
  if (value.length() > 0 && (value.at(0) == JSON_OBJECT_MARK_CHAR ||
                             value.at(0) == JSON_ARRAY_MARK_CHAR)) {
    MapInsertOrAssign(this->styles_, key, value);
    insert = true;
  }

  if (key == ALIGN_ITEMS) {
    setAlignItems(GetWXCoreAlignItem(value));
    return kTypeLayout;
  } else if (key == ALIGN_SELF) {
    setAlignSelf(GetWXCoreAlignSelf(value));
    return kTypeLayout;
  } else if (key == FLEX) {
    if (value.empty()) {
      set_flex(0);
    } else {
      float ret = getFloat(value.c_str());
      if (!isnan(ret)) {
        set_flex(ret);
      }
    }
    return kTypeLayout;
  } else if (key == FLEX_DIRECTION) {
    setFlexDirection(GetWXCoreFlexDirection(value), updating);
    return kTypeLayout;
  } else if (key == JUSTIFY_CONTENT) {
    setJustifyContent(GetWXCoreJustifyContent(value));
    return kTypeLayout;
  } else if (key == FLEX_WRAP) {
    setFlexWrap(GetWXCoreFlexWrap(value));
    return kTypeLayout;
  } else if (key == MIN_WIDTH) {
    UpdateStyleInternal(key, value, NAN,
                        [=](float foo) { setMinWidth(foo, updating); });
    return kTypeLayout;
  } else if (key == MIN_HEIGHT) {
    UpdateStyleInternal(key, value, NAN, [=](float foo) { setMinHeight(foo); });
    return kTypeLayout;
  } else if (key == MAX_WIDTH) {
    UpdateStyleInternal(key, value, NAN,
                        [=](float foo) { setMaxWidth(foo, updating); });
    return kTypeLayout;
  } else if (key == MAX_HEIGHT) {
    UpdateStyleInternal(key, value, NAN, [=](float foo) { setMaxHeight(foo); });
    return kTypeLayout;
  } else if (key == HEIGHT) {
    if (UpdateStyleInternal(key, value, NAN,
                            [=](float foo) { setStyleHeight(foo); })) {
      setStyleHeightLevel(CSS_STYLE);
    }
    return kTypeLayout;
  } else if (key == WIDTH) {
    if (UpdateStyleInternal(key, value, NAN,
                            [=](float foo) { setStyleWidth(foo, updating); })) {
      setStyleWidthLevel(CSS_STYLE);
    }
    return kTypeLayout;
  } else if (key == POSITION) {
    setStylePositionType(GetWXCorePositionType(value));
    if (value == STICKY) {
      this->is_sticky_ = true;
    }
    MapInsertOrAssign(this->styles_, key, value);
    return kTypeStyle;
  } else if (key == LEFT) {
    UpdateStyleInternal(key, value, NAN, [=](float foo) {
      setStylePosition(kPositionEdgeLeft, foo);
    });
    return kTypeLayout;
  } else if (key == TOP) {
    UpdateStyleInternal(key, value, NAN, [=](float foo) {
      setStylePosition(kPositionEdgeTop, foo);
    });
    return kTypeLayout;
  } else if (key == RIGHT) {
    UpdateStyleInternal(key, value, NAN, [=](float foo) {
      setStylePosition(kPositionEdgeRight, foo);
    });
    return kTypeLayout;
  } else if (key == BOTTOM) {
    UpdateStyleInternal(key, value, NAN, [=](float foo) {
      setStylePosition(kPositionEdgeBottom, foo);
    });
    return kTypeLayout;
  } else if (key == MARGIN) {
    UpdateStyleInternal(key, value, 0,
                        [=](float foo) { setMargin(kMarginALL, foo); });
    return kTypeMargin;
  } else if (key == MARGIN_LEFT) {
    UpdateStyleInternal(key, value, 0,
                        [=](float foo) { setMargin(kMarginLeft, foo); });
    return kTypeMargin;
  } else if (key == MARGIN_TOP) {
    UpdateStyleInternal(key, value, 0,
                        [=](float foo) { setMargin(kMarginTop, foo); });
    return kTypeMargin;
  } else if (key == MARGIN_RIGHT) {
    UpdateStyleInternal(key, value, 0,
                        [=](float foo) { setMargin(kMarginRight, foo); });
    return kTypeMargin;
  } else if (key == MARGIN_BOTTOM) {
    UpdateStyleInternal(key, value, 0,
                        [=](float foo) { setMargin(kMarginBottom, foo); });
    return kTypeMargin;
  } else if (key == BORDER_WIDTH) {
    UpdateStyleInternal(key, value, 0, [=](float foo) {
      setBorderWidth(kBorderWidthALL, foo);
    });
    return kTypeBorder;
  } else if (key == BORDER_TOP_WIDTH) {
    UpdateStyleInternal(key, value, 0, [=](float foo) {
      setBorderWidth(kBorderWidthTop, foo);
    });
    return kTypeBorder;
  } else if (key == BORDER_RIGHT_WIDTH) {
    UpdateStyleInternal(key, value, 0, [=](float foo) {
      setBorderWidth(kBorderWidthRight, foo);
    });
    return kTypeBorder;
  } else if (key == BORDER_BOTTOM_WIDTH) {
    UpdateStyleInternal(key, value, 0, [=](float foo) {
      setBorderWidth(kBorderWidthBottom, foo);
    });
    return kTypeBorder;
  } else if (key == BORDER_LEFT_WIDTH) {
    UpdateStyleInternal(key, value, 0, [=](float foo) {
      setBorderWidth(kBorderWidthLeft, foo);
    });
    return kTypeBorder;
  } else if (key == PADDING) {
    UpdateStyleInternal(key, value, 0,
                        [=](float foo) { setPadding(kPaddingALL, foo); });
    return kTypePadding;
  } else if (key == PADDING_LEFT) {
    UpdateStyleInternal(key, value, 0,
                        [=](float foo) { setPadding(kPaddingLeft, foo); });
    return kTypePadding;
  } else if (key == PADDING_TOP) {
    UpdateStyleInternal(key, value, 0,
                        [=](float foo) { setPadding(kPaddingTop, foo); });
    return kTypePadding;
  } else if (key == PADDING_RIGHT) {
    UpdateStyleInternal(key, value, 0,
                        [=](float foo) { setPadding(kPaddingRight, foo); });
    return kTypePadding;
  } else if (key == PADDING_BOTTOM) {
    UpdateStyleInternal(key, value, 0,
                        [=](float foo) { setPadding(kPaddingBottom, foo); });
    return kTypePadding;
  } else {
    if (!insert) {
      MapInsertOrAssign(this->styles_, key, value);
    }
    return kTypeStyle;
  }
}

const std::string RenderObject::GetStyle(const std::string &key) {
  if (this->styles_ == nullptr) return "";

  std::map<std::string, std::string>::iterator iter = this->styles_->find(key);
  if (iter != this->styles_->end()) {
    return iter->second;
  } else {
    return "";
  }
}

const std::string RenderObject::GetAttr(const std::string &key) {
  if (this->attributes_ == nullptr) return "";

  std::map<std::string, std::string>::iterator iter =
      this->attributes_->find(key);
  if (iter != this->attributes_->end()) {
    return iter->second;
  } else {
    return "";
  }
}

int RenderObject::AddRenderObject(int index, RenderObject *child) {
  if (child == nullptr || index < -1) {
    return index;
  }

  Index count = getChildCount();
  index = index >= count ? -1 : index;
  if (index == -1) {
    addChildAt(child, getChildCount());
  } else {
    addChildAt(child, index);
  }

  child->set_parent_render(this);

  return index;
}

Index RenderObject::IndexOf(const RenderObject *render) {
  if (render == nullptr) {
    return -1;
  } else {
    int i = 0;
    for (auto it = ChildListIterBegin(); it != ChildListIterEnd(); it++) {
      RenderObject *child = static_cast<RenderObject *>(*it);
      if (child != nullptr) {
        if (render->ref() == child->ref()) return i;
      }
      ++i;
    }
  }
  return -1;
}

bool RenderObject::UpdateStyleInternal(const std::string key,
                                       const std::string value, float fallback,
                                       std::function<void(float)> functor) {
  bool ret = false;
  if (value.empty()) {
    functor(fallback);
    ret = true;
  } else {
    float fvalue = getFloatByViewport(value, RenderManager::GetInstance()->viewport_width());
    if (!isnan(fvalue)) {
      functor(fvalue);
      ret = true;
    }
  }
  return ret;
}

void RenderObject::LayoutBeforeImpl() {
  if (isDirty()) {
    OnLayoutBefore();
  }

  for (auto it = ChildListIterBegin(); it != ChildListIterEnd(); it++) {
    RenderObject *child = static_cast<RenderObject *>(*it);
    if (child != nullptr) {
      child->LayoutBeforeImpl();
    }
  }
}

void RenderObject::LayoutAfterImpl() {
  if (hasNewLayout()) {
    OnLayoutAfter(getLayoutWidth(), getLayoutHeight());
  }

  for (auto it = ChildListIterBegin(); it != ChildListIterEnd(); it++) {
    RenderObject *child = static_cast<RenderObject *>(*it);
    if (child != nullptr) {
      child->LayoutAfterImpl();
    }
  }
}

void RenderObject::CopyFrom(RenderObject *src) {
  IRenderObject::CopyFrom(src);
  this->styles_->insert(src->styles_->begin(), src->styles_->end());
  this->attributes_->insert(src->attributes_->begin(), src->attributes_->end());
  this->events_->insert(src->events_->begin(), src->events_->end());
}

void RenderObject::MapInsertOrAssign(
    std::map<std::string, std::string> *targetMap, const std::string &key,
    const std::string &value) {
  std::map<std::string, std::string>::iterator it = targetMap->find(key);
  if (it != targetMap->end()) {
    it->second = value;
  } else {
    targetMap->insert({key, value});
  }
}

bool RenderObject::ViewInit() {
  return (!isnan(getStyleWidth()) && getStyleWidth() > 0) ||
         (is_root_render() && GetRenderPage() != nullptr &&
             GetRenderPage()->is_render_container_width_wrap_content());
}

RenderPage *RenderObject::GetRenderPage() {
  return RenderManager::GetInstance()->GetPage(page_id());
}

bool RenderObject::IsAppendTree() {
  std::string append = GetAttr(APPEND);
  if (append == "tree") {
    return true;
  }
  return false;
}

void RenderObject::UpdateAttr(std::string key, std::string value) {
  MapInsertOrAssign(this->attributes_, key, value);
}

StyleType RenderObject::UpdateStyle(std::string key, std::string value) {
  return ApplyStyle(key, value, true);
}

RenderObject *RenderObject::GetChild(const Index &index) {
  return static_cast<RenderObject *>(getChildAt(index));
}

void RenderObject::RemoveRenderObject(RenderObject *child) {
  removeChild(child);
}

void RenderObject::AddAttr(std::string key, std::string value) {
  MapInsertOrAssign(this->attributes_, key, value);
}

StyleType RenderObject::AddStyle(std::string key, std::string value) {
  return ApplyStyle(key, value, false);
}

void RenderObject::AddEvent(std::string event) {
  if (this->events_ == nullptr || this->events_->empty()) {
    this->events_ = new std::set<std::string>();
  }
  this->events_->insert(event);
}

void RenderObject::RemoveEvent(const std::string &event) {
  this->events_->erase(event);
}
}  // namespace WeexCore
