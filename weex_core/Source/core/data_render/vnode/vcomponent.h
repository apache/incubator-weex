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
#ifndef CORE_DATA_RENDER_VNODE_VCOMPONENT_H_
#define CORE_DATA_RENDER_VNODE_VCOMPONENT_H_

#include "core/data_render/object.h"
#include <unordered_map>
#include "core/data_render/vnode/vnode.h"

namespace weex {
namespace core {
namespace data_render {
class VComponent : public VNode {
 public:
  typedef std::unordered_map<std::string, std::string> VNodeRef;
  typedef std::vector<VNodeRef> VNodeRefs;
  class LifecycleListener {
   public:
    virtual ~LifecycleListener() {}
    virtual void OnCreated(
        VComponent* component, Table* data, Table* props,
        const std::unordered_map<std::string, VNodeRefs>& ref_map) = 0;
    virtual void OnUpdated(
        VComponent* component, Table* props,
        const std::unordered_map<std::string, VNodeRefs>& ref_map) = 0;
    virtual void OnDestroyed(VComponent* component) = 0;
  };
  VComponent(ExecState* exec_state, int template_id, const std::string& name,
             const std::string& func_name);
  ~VComponent();
  void UpdateData();
  void UpdateData(Value* datas);
  inline bool IsSameNode(VComponent* old_component) {
    return template_id_ == old_component->template_id_;
  }
  void MoveTo(VComponent* new_component);

  inline const std::string render_object_ref() const override {
    if (root_vnode_.get()) {
      return root_vnode_->render_object_ref();
    }
    return "";
  }

  inline int id() { return id_; }
  inline int template_id() { return template_id_; }
  inline ExecState* exec_state() { return exec_state_; }
  void SetData(Value* data);
  void SetProps(Value* props);
  inline VNode* root_vnode() { return root_vnode_.get(); }
  void SetRootNode(VNode* node);
  bool IsVirtualComponent() const override { return true; }
  VComponent* component() override {return this;}

  void DispatchCreated();
  void DispatchUpdated();
  void DispatchDestroyed();

  inline void set_life_cycle_listener(
      std::unique_ptr<LifecycleListener> listener) {
    listener_ = std::move(listener);
  }

 private:
  typedef void (VComponent::*TravelTreeFunc)();
  void TravelVComponentsWithFunc(TravelTreeFunc func, VNode* root);

  void BuildRefMap();
  bool Equal(VComponent* old_component);
  void DispatchAttachedToParent() override;
  void DispatchDetachedFromParent() override;

  bool is_dirty_;
  bool has_dispatch_created_;
  bool has_moved_;
  // unique id for unique component
  int id_;
  // Unique id for unique template, a template to multiple component
  int template_id_;
  // Unique template name
  std::string name_;
  // For invoke function in vm to create nodes in component
  std::string func_name_;
  // Save data and props for diff and update
  Value data_;
  Value props_;
  Value updated_props_;
  std::unique_ptr<LifecycleListener> listener_;
  std::unordered_map<std::string, VNodeRefs> ref_map_;
  std::unique_ptr<VNode> old_root_vnode_;
  std::unique_ptr<VNode> root_vnode_;
  ExecState* exec_state_;
};
}  // namespace data_render
}  // namespace core
}  // namespace weex

#endif  // CORE_DATA_RENDER_VNODE_VCOMPONENT_H_
