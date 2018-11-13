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
#ifndef CORE_RENDER_ACTION_RENDER_ACTION_LAYOUT_H_
#define CORE_RENDER_ACTION_RENDER_ACTION_LAYOUT_H_

#include <set>
#include <string>

#include "core/render/action/render_action_interface.h"

namespace WeexCore {

class RenderObject;

class WXCoreLayoutNode;

class RenderActionLayout : public RenderAction {
 public:
  explicit RenderActionLayout(const std::string &page_id,
                              const RenderObject *render, const int index);

  void ExecuteAction();

  void GetLayoutInfo(const WXCoreLayoutNode *node);

 public:
  std::string page_id_;
  std::string ref_;
  float width_;
  float height_;
  float top_;
  float left_;
  float bottom_;
  float right_;
  int index_;
};
}  // namespace WeexCore

#endif  // CORE_RENDER_ACTION_RENDER_ACTION_LAYOUT_H_
