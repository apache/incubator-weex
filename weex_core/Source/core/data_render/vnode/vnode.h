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
#ifndef CORE_DATA_RENDER_VNODE_VNODE_
#define CORE_DATA_RENDER_VNODE_VNODE_

#include <map>
#include <string>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <memory>
#include "core/render/node/render_object.h"

namespace weex {
namespace core {
namespace data_render {
class VComponent;
class Value;

class VNode {
 public:
  typedef std::vector<Value> Params;
  typedef std::vector<Params> ParamsList;
  typedef std::unordered_map<std::string, ParamsList> EventParamsMap;

  class OnEventListener {
   public:
    virtual ~OnEventListener() {}
    virtual void OnEvent(VNode *node, const std::string &event,
                         const std::string &json_args,
                         const std::string dom_changes,
                         const Params &params) = 0;
  };

  VNode(const std::string &tag_name, const std::string &node_id,
        const std::string &ref);

  virtual ~VNode();

  void SetStyle(const std::string &key, const std::string &value);

  void SetAttribute(const std::string &key, const std::string &value);

  void AddEvent(const std::string &event,
                const std::vector<Value> &params);
  void AddEvent(const std::string &event, void *func, void *inst);

  void AddOnEventListener();

  void AddChild(VNode *child);

  void RemoveChild(VNode *child);
    
  void InsertChild(VNode *child, int index);

  VNode *FindNode(const std::string &render_object_ref);

 public:
  inline const std::string &tag_name() const { return tag_name_; }

  inline const std::string &node_id() const { return node_id_; }

  inline const std::string &ref() const { return ref_; }

  virtual inline const std::string render_object_ref() const {
    return render_object_ref_;
  }

  inline void set_render_object_ref(std::string ref) {
    render_object_ref_ = std::move(ref);
  }
  inline void *& inst() { return inst_; }
  inline VNode *parent() const { return parent_; }

  inline std::vector<VNode *> *child_list() { return &child_list_; }

  inline std::map<std::string, std::string> *styles() const { return styles_; }

  inline std::map<std::string, std::string> *attributes() const {
    return attributes_;
  }
  inline std::map<std::string, void *> *events() const {
    return events_;
  }

  inline EventParamsMap *event_params_map() const {
    return event_params_map_.get();
  }

  void OnEvent(const std::string &event, const std::string args,
               const std::string dom_changes);

  inline void set_on_event_listener(std::unique_ptr<OnEventListener> listener) {
    on_event_listener_ = std::move(listener);
  }

  inline bool HasChildren() { return !child_list_.empty(); }

  void set_component(VComponent* c) {
    component_ = c;
    for (auto child : child_list_) {
      if (!IsVirtualComponent()) {
        child->set_component(c);
      }
    }
  }

  virtual VComponent* component() {
    return component_;
  }

  virtual bool IsVirtualComponent() const { return false; }

 private:
  std::string tag_name_;
  // Not unique
  std::string node_id_;
  // Should be unique
  std::string ref_;
  // Context of vnode
  VComponent* component_ = nullptr;
  // Ref point to RenderObject is set when PatchVNode or ParseVNode2RenderObject
  std::string render_object_ref_;

  VNode *parent_ = nullptr;
  void *inst_ = nullptr;
  std::vector<VNode *> child_list_;
  std::map<std::string, std::string> *styles_;
  std::map<std::string, std::string> *attributes_;
  // This events if record event with func from vm
  std::map<std::string, void *> *events_;
  // This events if record event with value from vm
  std::unique_ptr<EventParamsMap> event_params_map_;
  std::unique_ptr<OnEventListener> on_event_listener_;

  void MapInsertOrAssign(std::map<std::string, std::string> *target_map,
                         const std::string &key, const std::string &value);
  inline virtual void DispatchAttachedToParent() {}
  inline virtual void DispatchDetachedFromParent() {}
};

}  // namespace data_render
}  // namespace core
}  // namespace weex

#endif  // CORE_DATA_RENDER_VNODE_VNODE_
