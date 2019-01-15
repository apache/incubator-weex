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
#include "vnode_render_manager.h"

namespace weex {
namespace core {
namespace data_render {

// For unique component id
static int g_component_id = 0;

VComponent::VComponent(ExecState *exec_state, int template_id,
                       const std::string &name, const std::string &func_name)
    : VNode("", "", ""),
      is_dirty_(false),
      has_dispatch_created_(false),
      has_moved_(false),
      id_(g_component_id++),
      template_id_(template_id),
      name_(name),
      func_name_(func_name),
      data_(exec_state->class_factory()->CreateTable()),
      props_(exec_state->class_factory()->CreateTable()),
      listener_(nullptr),
      old_root_vnode_(nullptr),
      root_vnode_(nullptr),
      exec_state_(exec_state) {}

VComponent::~VComponent() {
  if (listener_ && !has_moved_) {
    listener_->OnDestroyed(this);
  }
}

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

static void BuildRefsInner(
    std::unordered_map<std::string, VComponent::VNodeRefs> &ref_map,
    VNode *node, bool in_for_loop) {
  if (!node) {
    return;
  }
  if (node->attributes()->find("[[repeat]]") != node->attributes()->end()) {
    in_for_loop = true;
  }
  if (!node->ref().empty()) {
    VComponent::VNodeRef ref;
    ref.insert({"ref", node->render_object_ref()});
    auto it = ref_map.find(node->ref());
    if (it != ref_map.end()) {
      if (!in_for_loop) {
        return;
      }
      it->second.push_back(ref);
    } else {
      VComponent::VNodeRefs refs;
      refs.push_back(ref);
      ref_map.insert({node->ref(), refs});
    }
  }
  // Record { ref : node } in context
  node->component()->exec_state()->context()->AddVNode(node->render_object_ref(), node);
  if (node->child_list()->size() > 0 && !node->IsVirtualComponent()) {
    for (auto it = node->child_list()->begin(); it != node->child_list()->end();
         it++) {
      BuildRefsInner(ref_map, *it, in_for_loop);
    }
  }
}

static void DetachVNodesInContext(VNode *node) {
  // Remove { ref : node } in context
  if (!node || node->IsVirtualComponent()) return;;
  auto context = node->component()->exec_state()->context();
  auto record = context->GetVNode(node->render_object_ref());
  if (record != node) {
    context->RemoveVNode(node->render_object_ref());
  }
  if (node->child_list()->size() > 0 && !node->IsVirtualComponent()) {
    for (auto it = node->child_list()->begin(); it != node->child_list()->end();
         it++) {
      DetachVNodesInContext(*it);
    }
  }
}

void VComponent::BuildRefMap() {
  ref_map_.clear();
  BuildRefsInner(ref_map_, root_vnode(), false);
}

bool VComponent::Equal(VComponent *old_component) {
  VComponent *new_component = this;
  if (old_component == new_component) return true;
  // Mainly compare props
  Table *old_table = ValueTo<Table>(&old_component->props_);
  Table *new_table = ValueTo<Table>(&new_component->props_);
  if (updated_props_.type == Value::Type::NIL) {
    updated_props_ = exec_state_->class_factory()->CreateTable();
  } else {
    ValueTo<Table>(&updated_props_)->map.clear();
  }
  bool equal = true;
  for (auto it = old_table->map.begin(); it != old_table->map.end(); ++it) {
    auto old_value = it->second;
    auto new_value = new_table->map[it->first];
    if (old_value.type != new_value.type || !Equals(old_value, new_value)) {
      ValueTo<Table>(&updated_props_)->map.insert({it->first, new_value});
      equal = false;
    }
  }
  return equal;
}

void VComponent::MoveTo(VComponent *new_component) {
  new_component->has_dispatch_created_ = has_dispatch_created_;
  new_component->id_ = id_;
  new_component->SetRootNode(root_vnode_.release());
  new_component->data_ = data_;
  if (!new_component->Equal(this)) {
    new_component->UpdateData();
  }
  exec_state_->context()->RemoveComponent(id_);
  exec_state_->context()->AddComponent(new_component->id(), new_component);
  has_moved_ = true;
}

void VComponent::UpdateData() {
  // Merge data and props
  Value new_value = exec_state_->class_factory()->CreateTable();
  TableAddAll(props_, new_value);
  TableAddAll(data_, new_value);

  Value component;
  component.cptr = this;
  component.type = Value::CPTR;
  exec_state_->Call(func_name_, {new_value, component, Value(false)});
  VNodeRenderManager::GetInstance()->PatchVNode(
      exec_state_, old_root_vnode_.get(), root_vnode_.get());

  TravelVComponentsWithFunc(&VComponent::DispatchDestroyed,
                            old_root_vnode_.get());
  old_root_vnode_.reset(nullptr);
  if (has_dispatch_created_) {
    is_dirty_ = true;
    DispatchUpdated();
  }
}

void VComponent::UpdateData(Value *data) {
  if (data->type == Value::Type::TABLE && TableAddAll(*data, data_)) {
    UpdateData();
  }
}

// TODO Depth-first traversal
void VComponent::TravelVComponentsWithFunc(TravelTreeFunc func, VNode *root) {
  if (!root) return;
  std::vector<VNode *> list;
  list.push_back(root);
  VNode *node = nullptr;
  while (!list.empty()) {
    node = list.front();
    list.erase(list.begin());
    if (node->IsVirtualComponent()) {
      (static_cast<VComponent *>(node)->*func)();
      continue;
    }
    for (auto it = node->child_list()->begin(); it != node->child_list()->end();
         it++) {
      list.push_back(*it);
    }
  }
}

void VComponent::DispatchCreated() {
  if (has_dispatch_created_) return;
  BuildRefMap();
  if (listener_) {
    listener_->OnCreated(this, ValueTo<Table>(&data_), ValueTo<Table>(&props_),
                         ref_map_);
  }
  TravelVComponentsWithFunc(&VComponent::DispatchCreated, root_vnode());
  has_dispatch_created_ = true;
}

void VComponent::DispatchUpdated() {
  if (!has_dispatch_created_) {
    DispatchCreated();
    return;
  }
  if (!is_dirty_) return;
  BuildRefMap();
  if (listener_) {
    if (updated_props_.type == Value::Type::NIL) {
      updated_props_ = exec_state_->class_factory()->CreateTable();
    }
    listener_->OnUpdated(this, ValueTo<Table>(&updated_props_), ref_map_);
  }
  TravelVComponentsWithFunc(&VComponent::DispatchUpdated, root_vnode());
  is_dirty_ = false;
}

void VComponent::DispatchDestroyed() {
  if (listener_ && !has_moved_) {
    listener_->OnDestroyed(this);
  }
  TravelVComponentsWithFunc(&VComponent::DispatchDestroyed, root_vnode());
  if (!has_moved_) {
    exec_state_->context()->RemoveComponent(id_);
  }
  DetachVNodesInContext(root_vnode());
}

void VComponent::SetRootNode(VNode *node) {
  if (root_vnode_) {
    old_root_vnode_ = std::move(root_vnode_);
  }
  root_vnode_.reset(node);
  if (root_vnode_) {
    root_vnode_->set_component(this);
  }
}

void VComponent::SetData(Value *data) { TableCopy(*data, data_); }

void VComponent::SetProps(Value *props) { TableCopy(*props, props_); }

void VComponent::DispatchAttachedToParent() {}

void VComponent::DispatchDetachedFromParent() { DispatchDestroyed(); }

}  // namespace data_render
}  // namespace core
}  // namespace weex
