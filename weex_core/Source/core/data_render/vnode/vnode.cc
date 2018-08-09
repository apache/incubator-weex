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
#include "core/render/node/factory/render_creator.h"
#include "core/data_render/vnode/vnode.h"

namespace weex {
namespace core {
namespace data_render {

VNode::VNode(const std::string &ref, const std::string &tag_name) {
  ref_ = ref;
  tag_name_ = tag_name;

  styles_ = new std::map<std::string, std::string>();
  attributes_ = new std::map<std::string, std::string>();
}

VNode::~VNode() {
  parent_ = nullptr;

  if (styles_ != nullptr) {
    delete styles_;
    styles_ = nullptr;
  }

  if (attributes_ != nullptr) {
    delete attributes_;
    attributes_ = nullptr;
  }

  for (auto it = child_list_.begin(); it != child_list_.end(); it++) {
    VNode *&reference = *it;
    if (reference != nullptr) {
      delete reference;
      *it = nullptr;
    }
  }
}

void VNode::SetStyle(const std::string &key, const std::string &value) {
  MapInsertOrAssign(styles_, key, value);
}

void VNode::SetAttribute(const std::string &key, const std::string &value) {
  MapInsertOrAssign(attributes_, key, value);
}

void VNode::AddEvent(const std::string &event, const std::string &function,
                     const std::vector<std::string> &params) {
  //todo
}

void VNode::AddChild(VNode *child) {
  child->parent_ = this;
  child_list_.push_back(child);
}

void VNode::MapInsertOrAssign(std::map<std::string, std::string> *target_map, const std::string &key,
                              const std::string &value) {
  std::map<std::string, std::string>::iterator it = target_map->find(key);
  if (it != target_map->end()) {
    it->second = value;
  } else {
    target_map->insert({key, value});
  }
}
}  // namespace data_render
}  // namespace core
}  // namespace weex