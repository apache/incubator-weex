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
#ifndef CORE_RENDER_ACTION_RENDER_ACTION_CREATEBODY_H_
#define CORE_RENDER_ACTION_RENDER_ACTION_CREATEBODY_H_

#include <map>
#include <set>
#include <string>

#include "core/layout/style.h"
#include "core/render/action/render_action_interface.h"

namespace WeexCore {

class RenderObject;

class RenderActionCreateBody : public RenderAction {
 public:
  explicit RenderActionCreateBody(const std::string &page_id,
                                  const RenderObject *render);

  void ExecuteAction();

 public:
  std::map<std::string, std::string> *styles_;
  std::map<std::string, std::string> *attributes_;
  std::set<std::string> *events_;
  WXCoreMargin margins_;
  WXCorePadding paddings_;
  WXCoreBorderWidth borders_;
  std::string page_id_;
  std::string component_type_;
  std::string ref_;
};
}  // namespace WeexCore

#endif  // CORE_RENDER_ACTION_RENDER_ACTION_CREATEBODY_H_
