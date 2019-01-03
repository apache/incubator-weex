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

#include <utility>
#include <math.h>
#include "core/css/constants_name.h"
#include "core/render/node/render_scroller.h"

namespace WeexCore {

std::map<std::string, std::string> *RenderScroller::GetDefaultStyle() {
  std::map<std::string, std::string> *style =
      new std::map<std::string, std::string>();

  bool is_vertical = true;
  RenderObject *parent = static_cast<RenderObject *>(getParent());

  if (parent != nullptr) {
    if (parent->GetAttr(SCROLL_DIRECTION) == HORIZONTAL) {
      is_vertical = false;
    }
  }

  std::string prop = is_vertical ? HEIGHT : WIDTH;

  if (prop == HEIGHT && isnan(getStyleHeight()) && !this->is_set_flex_) {
    style->insert(std::pair<std::string, std::string>(FLEX, "1"));
  } else if (prop == WIDTH && isnan(getStyleWidth()) && !this->is_set_flex_) {
    style->insert(std::pair<std::string, std::string>(FLEX, "1"));
  }

  return style;
}

void RenderScroller::set_flex(const float flex) {
  this->is_set_flex_ = true;
  WXCoreLayoutNode::set_flex(flex);
}

void RenderScroller::onLayout(const float left, const float top, const float right, const float bottom,
                                  WXCoreLayoutNode *const absoulteItem, WXCoreFlexLine *const flexLine) {
  // In scroller only use left to right direction to caculate children frame
  this->setLayoutDirection(kDirectionLTR);
  RenderObject::onLayout(left, top, right, bottom, absoulteItem, flexLine);
}

}  // namespace WeexCore
