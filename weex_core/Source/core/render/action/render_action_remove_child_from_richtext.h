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
#ifndef CORE_RENDER_ACTION_RENDER_ACTION_REMOVE_CHILD_FROM_RICHTEXT_H_
#define CORE_RENDER_ACTION_RENDER_ACTION_REMOVE_CHILD_FROM_RICHTEXT_H_

#include <string>
#include "core/render/action/render_action_interface.h"

namespace WeexCore {
class RenderObject;
class RenderActionRemoveChildFromRichtext : public RenderAction {
 public:
  explicit RenderActionRemoveChildFromRichtext(const std::string &page_id, const std::string &ref, const RenderObject* parent, RenderObject* richtext);

  void ExecuteAction();

 public:
  std::string page_id_;
  std::string ref_;
  std::string parent_ref_;
  std::string richtext_ref_;
};
}  // namespace WeexCore

#endif  // CORE_RENDER_ACTION_RENDER_ACTION_REMOVE_CHILD_FROM_RICHTEXT_H_
