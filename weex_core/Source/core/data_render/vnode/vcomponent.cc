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

#include "core/data_render/vnode/vcomponent.h"
#include "core/data_render/exec_state.h"
namespace weex {
namespace core {
namespace data_render {

// For unique component id
static int g_component_id = 0;

VComponent::VComponent(ExecState *exec_state, int template_id,
                       const std::string &name, const std::string &tag_name,
                       const std::string &node_id, const std::string &ref)
    : VNode(tag_name, node_id, ref),
      exec_state_(exec_state),
      id_(g_component_id++),
      template_id_(template_id),
      name_(name),
      // This function name definition rule is in parser.cc
      func_name_("createComponent_" + name),
      data_(nullptr),
      props_(nullptr),
      listener_(nullptr),
      dirty_(false),
      children_(),
      parent_(nullptr) {}

VComponent::~VComponent() {}

static bool Equals(Value a, Value b) {
  if (a.type != b.type) {
    return false;
  } else if (!(a == b)) {
    switch (a.type) {
      case Value::Type::TABLE: {
        auto a_table = ValueTo<Table>(&a);
        auto b_table = ValueTo<Table>(&b);
        if (a_table->map.size() != b_table->map.size()) {
          return false;
        } else {
          for (auto it = a_table->map.begin(); it != a_table->map.end(); ++it) {
            auto a_value = it->second;
            auto b_value = b_table->map[it->first];
            if (!Equals(a_value, b_value)) {
              return false;
            }
          }
        }
      } break;
      case Value::Type::ARRAY: {
        auto a_array = ValueTo<Array>(&a);
        auto b_array = ValueTo<Array>(&b);
        if (a_array->items.size() != b_array->items.size()) {
          return false;
        } else {
          for (auto i = 0; i < a_array->items.size(); ++i) {
            if (!Equals(a_array->items[i], b_array->items[i])) {
              return false;
            }
          }
        }
      } break;
      default:
        return false;
    }
  }
  return true;
}

void VComponent::Compare(VComponent *old_component) {
  VComponent *new_component = this;
  if (old_component == new_component) return;
  // Mainly compare props
  for (auto it = old_component->props_->map.begin();
       it != old_component->props_->map.end(); ++it) {
    auto old_value = it->second;
    auto new_value = new_component->props_->map[it->first];
    if (old_value.type != new_value.type || !Equals(old_value, new_value)) {
      updated_props_[it->first] = new_value;
      dirty_ = true;
    }
  }
}

void VComponent::UpdateData(std::unordered_map<std::string, Value> datas) {
  Value new_value = exec_state_->class_factory()->CreateTable();
  TableMapAddAll(props_, ValueTo<Table>(&new_value));
  TableMapAddAll(data_, ValueTo<Table>(&new_value));
  //Value new_component = exec_state_->Call(func_name_, {new_value}).cptr;
}

void VComponent::DispatchCreated() {
  for (auto it = children_.begin(); it != children_.end(); ++it) {
    (*it)->DispatchCreated();
  }
}

void VComponent::DispatchUpdated() {
  if (dirty_) {
    for (auto it = children_.begin(); it != children_.end(); ++it) {
      if ((*it)->dirty_) {
        (*it)->DispatchUpdated();
      }
    }
    dirty_ = false;
  }
}

void VComponent::DispatchDestroyed() {
  for (auto it = children_.begin(); it != children_.end(); ++it) {
    (*it)->DispatchDestroyed();
  }
}

void VComponent::AppendChildComponent(VComponent *child) {
  children_.push_back(child);
  child->parent_ = this;
}

}  // namespace data_render
}  // namespace core
}  // namespace weex
