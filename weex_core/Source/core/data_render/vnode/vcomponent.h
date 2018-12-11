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
// VComponent is a VNode
class VComponent : public VNode {
 public:
  class LifecycleListener {
    virtual void OnCreated(VComponent* component,
                           std::unordered_map<std::string, Value> datas,
                           std::unordered_map<std::string, Value> props) = 0;
    virtual void OnUpdated(VComponent* component,
                           std::unordered_map<std::string, Value> props) = 0;
    virtual void OnDestroyed(VComponent* component) = 0;
  };
  VComponent(ExecState* exec_state, int template_id, const std::string& name,
             const std::string& tag_name, const std::string& node_id,
             const std::string& ref);
  ~VComponent();
  void UpdateData(std::unordered_map<std::string, Value> datas);
  void DispatchCreated();
  void DispatchUpdated();
  void DispatchDestroyed();
  void Compare(VComponent* component);

  void AppendChildComponent(VComponent* child);

  inline void set_lifecycle_listener(LifecycleListener* listener) {
    listener_ = listener;
  }

  inline int id() { return id_; }
  inline int template_id() { return template_id_; }
  VComponent* parent() { return parent_; }
  inline void set_data(Table* data) { data_ = data; }
  inline void set_props(Table* props) { props = props_; }

 private:
  int id_;
  int template_id_;
  std::string name_;
  // For invoke function in vm to create component
  std::string func_name_;
  // Save data and props for diff and update
  Table* data_;
  Table* props_;
  std::unordered_map<std::string, Value> updated_props_;
  // Ptr should handle by outside
  LifecycleListener* listener_;
  // Mark if this component is a new one after diff & patch
  bool dirty_;
  std::vector<VComponent*> children_;
  VComponent* parent_;
  ExecState* exec_state_;
};
}  // namespace data_render
}  // namespace core
}  // namespace weex

#endif  // CORE_DATA_RENDER_VNODE_VCOMPONENT_H_
