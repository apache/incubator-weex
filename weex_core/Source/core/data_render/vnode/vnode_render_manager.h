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

#ifndef CORE_DATA_RENDER_VNODE_VNODE_RENDER_MANAGER_
#define CORE_DATA_RENDER_VNODE_VNODE_RENDER_MANAGER_

#include <map>
#include <string>

#include "core/data_render/vm.h"
#include "core/data_render/vnode/vnode.h"
#include "core/data_render/vnode/vnode_exec_env.h"
#include "core/render/manager/render_manager.h"
#include "core/render/node/render_object.h"

namespace weex {
namespace core {
namespace data_render {

class VNodeRenderManager {
 private:
  VNodeRenderManager() {}

  ~VNodeRenderManager() {}

 public:
  void CreatePage(const std::string &input, const std::string &page_id,
                  const std::string &init_data);
  bool RefreshPage(const std::string &page_id, const std::string &init_data);
  bool ClosePage(const std::string &page_id);

  static VNodeRenderManager *GetInstance() {
    if (!g_instance) {
      g_instance = new VNodeRenderManager();
    }
    return g_instance;
  }

 private:
  void InitVM();
  bool CreatePageInternal(const std::string &page_id, VNode *v_node);
  bool RefreshPageInternal(const std::string &page_id, VNode *new_node);
  bool ClosePageInternal(const std::string &page_id);

  static VM *g_vm;
  static VNodeRenderManager *g_instance;

  std::map<std::string, VNode *> vnode_trees_;
  std::map<std::string, ExecState *> exec_states_;
};
}  // namespace data_render
}  // namespace core
}  // namespace weex
#endif  // CORE_DATA_RENDER_VNODE_VNODE_RENDER_MANAGER_