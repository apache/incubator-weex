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

VNode::VNode(const std::string &tag_name, const std::string &node_id,
             const std::string &ref) {
  ref_ = ref;
  tag_name_ = tag_name;
  node_id_ = node_id;

  styles_ = new std::map<std::string, std::string>();
  attributes_ = new std::map<std::string, std::string>();
  events_ = new std::map<std::string, void *>();
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
  if (events_) {
      delete events_;
      events_ = nullptr;
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
    
void VNode::AddEvent(const std::string &event, void *func, void *inst) {
    std::map<std::string, void *>::iterator it = events_->find(event);
    if (it != events_->end()) {
        it->second = func;
    }
    else {
        events_->insert({event, func});
    }
    inst_ = inst;
}
    
VNode *VNode::FindNode(const std::string &render_object_ref) {
    VNode *node = nullptr;
    do {
        if (this->render_object_ref() == render_object_ref) {
            node = this;
            break;
        }
        for (int i = 0; i < child_list_.size(); i++) {
            node = child_list_[i]->FindNode(render_object_ref);
            if (node) {
                break;
            }
        }
        
    } while (0);
    
    return node;
}

void VNode::AddEvent(const std::string &event, const std::string &function,
                     const std::vector<std::string> &params) {
  //todo
}
    
void VNode::AddChild(VNode *child) {
  child->parent_ = this;
  child_list_.push_back(child);
}
    
void VNode::InsertChild(VNode *child, int index) {
    do {
        child->parent_ = this;
        if (index < child_list_.size()) {
            child_list_.insert(child_list_.begin() + index, child);
            break;
        }
        child_list_.push_back(child);
        
    } while (0);
}

void VNode::RemoveChild(VNode *child) {
  child->parent_ = nullptr;
  auto it = child_list_.begin();
  for (; it != child_list_.end(); ++it) {
    if (*it == child) {
      VNode *&reference = *it;
      child_list_.erase(it);
      if (reference != nullptr) {
          delete reference;
      }
      break;
    }
  }
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
