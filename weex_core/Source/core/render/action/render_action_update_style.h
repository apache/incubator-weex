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
#ifndef CORE_RENDER_ACTION_RENDER_ACTION_UPDATE_STYLE_H_
#define CORE_RENDER_ACTION_RENDER_ACTION_UPDATE_STYLE_H_

#include <string>
#include <utility>
#include <vector>

#include "core/render/action/render_action_interface.h"

namespace WeexCore {

class RenderActionUpdateStyle : public RenderAction {
 public:
  explicit RenderActionUpdateStyle(
      const std::string &page_id, const std::string &ref,
      std::vector<std::pair<std::string, std::string>> *style,
      std::vector<std::pair<std::string, std::string>> *margin,
      std::vector<std::pair<std::string, std::string>> *padding,
      std::vector<std::pair<std::string, std::string>> *border);

  void ExecuteAction();

 public:
  std::string page_id_;
  std::string ref_;
  std::vector<std::pair<std::string, std::string>> *style_;
  std::vector<std::pair<std::string, std::string>> *margin_;
  std::vector<std::pair<std::string, std::string>> *padding_;
  std::vector<std::pair<std::string, std::string>> *border_;
};
}  // namespace WeexCore

#endif  // CORE_RENDER_ACTION_RENDER_ACTION_UPDATE_STYLE_H_
