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

#include "core/render/action/render_action_remove_child_from_richtext.h"
#include "core/manager/weex_core_manager.h"
#include "core/render/node/render_object.h"

namespace WeexCore {

RenderActionRemoveChildFromRichtext::RenderActionRemoveChildFromRichtext(const std::string &page_id, const std::string &ref, const RenderObject* parent, RenderObject* richtext) {
  this->page_id_ = page_id;
  this->ref_ = ref;
  this->parent_ref_ = parent ? parent->ref() : "";
  this->richtext_ref_ = richtext->ref();
}

void RenderActionRemoveChildFromRichtext::ExecuteAction() {
  WeexCoreManager::Instance()
      ->getPlatformBridge()
      ->platform_side()
      ->RemoveChildFromRichtext(this->page_id_.c_str(), this->ref_.c_str(), this->parent_ref_.c_str(), this->richtext_ref_.c_str());
}
}  // namespace WeexCore
