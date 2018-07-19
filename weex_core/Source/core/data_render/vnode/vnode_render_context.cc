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

#include "core/data_render/vnode/vnode_render_context.h"

namespace weex {
namespace core {
namespace data_render {
void VNodeRenderContext::setVNodeRoot(VNode* v_node) {
  root_.reset(v_node);
}

VNode* VNodeRenderContext::find_node(const std::string& ref) {
  auto it = node_map_.find(ref);
  if (it == node_map_.end()) {
    return nullptr;
  }
  return it->second;
}
}
}
}