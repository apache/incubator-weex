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
#pragma once

#include <string>
#include <memory>
#include "core/data_render/json/json11.hpp"
#include "core/data_render/vnode/vnode.h"

namespace weex {
namespace core {
namespace data_render {
class VNodeRenderContext {
 public:
  void setVNodeRoot(VNode* v_node);
  VNode* find_node(const std::string& ref);

  inline void page_id(const std::string& page_id) { page_id_ = page_id; }
  inline const std::string& page_id() const { return page_id_; }
  inline VNode* root() const { return root_.get(); }
  inline void set_root(VNode* root) { return root_.reset(root); }
  inline void insert_node(VNode* node) {
    node_map_.insert({node->ref(), node});
  }
  inline json11::Json& raw_json() { return raw_json_; }
 private:
  // node context
  std::string page_id_;
  std::unique_ptr<VNode> root_;
  std::map<std::string, VNode*> node_map_;
  json11::Json raw_json_;
};
}
}
}
