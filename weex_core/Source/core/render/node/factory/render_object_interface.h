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
#ifndef CORE_RENDER_NODE_FACTORY_RENDER_OBJECT_INTERFACE_H_
#define CORE_RENDER_NODE_FACTORY_RENDER_OBJECT_INTERFACE_H_

#include <string>

#include "core/layout/layout.h"

namespace WeexCore {

class IRenderObject : public WXCoreLayoutNode {
 public:
  virtual ~IRenderObject() {}
  inline void set_ref(const std::string& ref) { this->ref_ = ref; }

  inline const std::string &ref() const { return ref_; }

  inline void set_page_id(const std::string& page_id) { this->page_id_ = page_id; }

  inline const std::string &page_id() const { return page_id_; }

  inline void set_type(const std::string& type) { this->tyle_ = type; }

  inline const std::string &type() const { return tyle_; }

  inline void CopyFrom(IRenderObject *src) {
    WXCoreLayoutNode::copyFrom(src);
    set_ref(src->ref());
    set_page_id(src->page_id());
    set_type(src->type());
  }

 private:
  std::string page_id_ = "";
  std::string ref_ = "";
  std::string tyle_ = "";
};
}  // namespace WeexCore

#endif  // CORE_RENDER_NODE_FACTORY_RENDER_OBJECT_INTERFACE_H_
