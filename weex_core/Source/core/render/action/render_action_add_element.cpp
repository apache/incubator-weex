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

#include "core/render/action/render_action_add_element.h"
#include "core/manager/weex_core_manager.h"
#include "core/render/node/render_object.h"

namespace WeexCore {

RenderActionAddElement::RenderActionAddElement(const std::string &page_id,
                                               const RenderObject *render,
                                               const RenderObject *parent,
                                               int index, bool will_layout) {
  this->attributes_ = render->attributes();
  this->styles_ = render->styles();
  this->events_ = render->events();
  this->margins_ = render->GetMargins();
  this->paddings_ = render->GetPaddings();
  this->borders_ = render->GetBorders();
  this->page_id_ = page_id;
  this->component_type_ = render->type();
  this->ref_ = render->ref();
  this->parent_ref_ = parent->ref();
  this->index_ = index;
  this->will_layout_ = will_layout;
}

void RenderActionAddElement::ExecuteAction() {
  WeexCoreManager::Instance()->getPlatformBridge()->platform_side()->AddElement(
      this->page_id_.c_str(), this->component_type_.c_str(), this->ref_.c_str(),
      this->index_, this->parent_ref_.c_str(), this->styles_, this->attributes_,
      this->events_, this->margins_, this->paddings_, this->borders_,
      this->will_layout_);
}
}  // namespace WeexCore
