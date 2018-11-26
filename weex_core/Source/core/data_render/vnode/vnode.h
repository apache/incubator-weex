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
#include "core/render/node/render_object.h"

namespace weex {
namespace core {
namespace data_render {
class VNode;

class VNode {
 public:
  VNode(const std::string &tag_name, const std::string &node_id,
        const std::string &ref);

  virtual ~VNode();

  void SetStyle(const std::string &key, const std::string &value);

  void SetAttribute(const std::string &key, const std::string &value);

  void AddEvent(const std::string &event, const std::string &function,
                const std::vector<std::string> &params);
  void AddEvent(const std::string &event, void *func, void *inst);

  void AddChild(VNode *child);

  void RemoveChild(VNode *child);
    
  void InsertChild(VNode *child, int index);

  VNode *FindNode(const std::string &render_object_ref);

 public:
  inline const std::string &tag_name() const { return tag_name_; }

  inline const std::string &node_id() const { return node_id_; }

  inline const std::string &ref() const { return ref_; }

  inline const std::string &render_object_ref() const {
    return render_object_ref_;
  }

  inline void set_render_object_ref(std::string ref) {
    render_object_ref_ = std::move(ref);
  }
  inline void *& inst() { return inst_; }
  inline const VNode *parent() const { return parent_; }

  inline std::vector<VNode *> *child_list() { return &child_list_; }

  inline std::map<std::string, std::string> *styles() const { return styles_; }

  inline std::map<std::string, std::string> *attributes() const {
    return attributes_;
  }
  inline std::map<std::string, void *> *events() const {
    return events_;
  }

  inline bool HasChildren() { return !child_list_.empty(); }

 private:
  std::string tag_name_;
  // Not unique
  std::string node_id_;
  // Should be unique
  std::string ref_;
  // Ref point to RenderObject is set when PatchVNode or ParseVNode2RenderObject
  std::string render_object_ref_;

  VNode *parent_ = nullptr;
  void *inst_ = nullptr;
  std::vector<VNode *> child_list_;
  std::map<std::string, std::string> *styles_;
  std::map<std::string, std::string> *attributes_;
  std::map<std::string, void *> *events_;
  void MapInsertOrAssign(std::map<std::string, std::string> *target_map,
                         const std::string &key, const std::string &value);
};

}  // namespace data_render
}  // namespace core
}  // namespace weex

#endif  // CORE_DATA_RENDER_VNODE_VNODE_
