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

#include "core/render/action/render_action_update_style.h"
#include "core/manager/weex_core_manager.h"

namespace WeexCore {

RenderActionUpdateStyle::RenderActionUpdateStyle(
    const std::string &page_id, const std::string &ref,
    std::vector<std::pair<std::string, std::string>> *style,
    std::vector<std::pair<std::string, std::string>> *margin,
    std::vector<std::pair<std::string, std::string>> *padding,
    std::vector<std::pair<std::string, std::string>> *border) {
  this->page_id_ = page_id;
  this->ref_ = ref;
  this->style_ = style;
  this->margin_ = margin;
  this->padding_ = padding;
  this->border_ = border;
}

void RenderActionUpdateStyle::ExecuteAction() {
  WeexCoreManager::getInstance()->getPlatformBridge()->callUpdateStyle(
      this->page_id_.c_str(), this->ref_.c_str(), this->style_, this->margin_,
      this->padding_, this->border_);
}
}  // namespace WeexCore
