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
#ifndef CORE_RENDER_ACTION_RENDER_ACTION_ADD_CHILD_TO_RICHTEXT_H_
#define CORE_RENDER_ACTION_RENDER_ACTION_ADD_CHILD_TO_RICHTEXT_H_

#include <map>
#include <set>
#include <string>

#include "core/layout/style.h"
#include "core/render/action/render_action_interface.h"

namespace WeexCore {

class RenderObject;

class RenderActionAddChildToRichtext : public RenderAction {
 public:
  explicit RenderActionAddChildToRichtext(const std::string &page_id,
                                  const RenderObject *render,
                                  const RenderObject *parent,
                                  const RenderObject *richtext);

  void ExecuteAction();

 public:
  std::map<std::string, std::string> *styles_;
  std::map<std::string, std::string> *attributes_;
  std::string page_id_;
  std::string parent_ref_;
  std::string node_type_;
  std::string ref_;
  std::string richtext_ref_;
};
}  // namespace WeexCore

#endif  // CORE_RENDER_ACTION_RENDER_ACTION_ADD_CHILD_TO_RICHTEXT_H_
