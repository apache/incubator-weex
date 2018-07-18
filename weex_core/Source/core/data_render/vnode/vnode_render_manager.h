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

#ifndef DATA_RENDER_VNODE_VNODE_RENDER_MANAGER_
#define DATA_RENDER_VNODE_VNODE_RENDER_MANAGER_

#include <map>
#include <string>

#include "core/render/node/render_object.h"
#include "core/render/manager/render_manager.h"
#include "vnode.h"
#include "core/data_render/vm.h"

namespace weex {
namespace core {
namespace data_render {

WeexCore::RenderObject* VNode2RenderObject(const VNode* root, const std::string& page_id);

class VNodeRenderManager {
 private:
  VNodeRenderManager() {}

  ~VNodeRenderManager() {}

  class Garbo {
   public:
    ~Garbo() {
      if (VNodeRenderManager::g_pInstance) {
        delete VNodeRenderManager::g_pInstance;
      }
    }
  };

  static Garbo g_garbo;

 public:
  void InitVM();

  void TestProcess(const std::string& input, const std::string& page_id);

  bool CreatePage(const std::string& page_id, VNode* vNode);

  bool RefreshPage(const std::string& page_id, VNode* new_node);

  bool ClosePage(const std::string& page_id);

  static VNodeRenderManager* GetInstance() {
    if (!g_pInstance) {
      g_pInstance = new VNodeRenderManager();
    }
    return g_pInstance;
  }

 private:
  static VM* g_vm_;
  static VNodeRenderManager* g_pInstance;
  std::map<std::string, VNode*> vnode_trees_;
};
}  // namespace data_render
}  // namespace core
}  // namespace weex
#endif