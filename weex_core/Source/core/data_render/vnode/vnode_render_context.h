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
#ifndef CORE_DATA_RENDER_VNODE_VNODE_RENDER_CONTEXT_
#define CORE_DATA_RENDER_VNODE_VNODE_RENDER_CONTEXT_

#include <memory>
#include <string>
#include "core/data_render/vnode/vnode.h"
#include "third_party/json11/json11.hpp"

namespace weex {
namespace core {
namespace data_render {
class VNodeRenderContext {
 public:
  VNodeRenderContext();
  ~VNodeRenderContext();
  void Reset();

  inline void page_id(const std::string& page_id) { page_id_ = page_id; }
  inline const std::string& page_id() const { return page_id_; }
  inline VNode* root() const { return root_; }
  inline void set_root(VNode* root) { root_ = root; }

  inline json11::Json& raw_json() { return raw_json_; }
  inline void set_script(const std::string& script) { script_ = script; }
  inline const std::string& script() { return script_; }
  inline bool HasScript() { return !script_.empty(); }
  inline std::string& raw_source() { return raw_source_; }
  inline std::map<std::string, json11::Json>& style_json() {
    return style_json_;
  }
  inline const json11::Json& script_json() {
    return script_json_;
  }
  inline void set_script_json(const json11::Json& script_json) {
      script_json_ = script_json;
  }

  inline void AddComponent(int ref, VComponent *component) {
    vcomponent_trees_.insert({ref, component});
  }

  inline void RemoveComponent(int ref) {
    vcomponent_trees_.erase(ref);
  }

  inline VComponent* GetComponent(int ref) {
    auto it = vcomponent_trees_.find(ref);
    return it != vcomponent_trees_.end() ? it->second : nullptr;
  }

  inline void RemoveVNode(const std::string& ref) {
    auto it = vnode_trees_.find(ref);
    if (it != vnode_trees_.end()) {
      vnode_trees_.erase(it);
    }
  }

  inline void AddVNode(const std::string& ref, VNode* node) {
    if (node->IsVirtualComponent()) {
      return;
    }
    auto it = vnode_trees_.find(ref);
    if (it != vnode_trees_.end()) {
      vnode_trees_[ref] = node;
    } else {
      vnode_trees_.insert({ref, node});
    }
  }

  inline VNode* GetVNode(const std::string &ref) {
    auto it = vnode_trees_.find(ref);
    return it != vnode_trees_.end() ? it->second : nullptr;
  }

 private:
  // node context
  std::string page_id_;
  VNode *root_;
  json11::Json raw_json_;
  std::string raw_source_;
  // script to execute
  std::string script_;
  std::map<std::string, json11::Json> style_json_;
  std::unordered_map<int, VComponent *> vcomponent_trees_;
  std::unordered_map<std::string, VNode*> vnode_trees_;
  json11::Json script_json_;
};
}  // namespace data_render
}  // namespace core
}  // namespace weex
#endif  // CORE_DATA_RENDER_VNODE_VNODE_RENDER_CONTEXT_
