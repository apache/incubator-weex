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
#include "core/data_render/vnode/vcomponent.h"
#include "core/data_render/vnode/vnode_exec_env.h"
#include "core/render/manager/render_manager.h"
#include "core/render/node/render_object.h"

namespace weex {
namespace core {
namespace data_render {

class HttpModule;
class VNodeRenderManager {
 friend class VNode;
 private:
  VNodeRenderManager() {}

  ~VNodeRenderManager() {}

 public:
  void CreatePage(const std::string &input, const std::string &page_id, const std::string &options, const std::string &init_data, std::function<void(const char*)> exec_js);

  void CreatePage(const char *contents, size_t length, const std::string& page_id, const std::string& options, const std::string& init_data, std::function<void(const char*)> exec_js);

  bool RefreshPage(const std::string &page_id, const std::string &init_data);
  bool ClosePage(const std::string &page_id);
  void FireEvent(const std::string &page_id, const std::string &ref, const std::string &event,const std::string &args,const std::string &dom_changes);
  void ExecuteRegisterModules(ExecState *exec_state, std::vector<std::string>& registers);
  void RegisterModules(const std::string &modules) { modules_.push_back(modules); }
  bool RequireModule(ExecState *exec_state, std::string &name, std::string &result);
  void PatchVNode(ExecState *exec_state, VNode *v_node, VNode *new_node);
  void CallNativeModule(ExecState *exec_state, const std::string &module, const std::string &method, const std::string &args, int argc = 0);
  void UpdateComponentData(const std::string& page_id, const char* cid, const std::string& json_data);
  void WXLogNative(ExecState *exec_state, const std::string &info);
  static VNodeRenderManager *GetInstance() {
    if (!g_instance) {
      g_instance = new VNodeRenderManager();
    }
    return g_instance;
  }

  inline ExecState *GetExecState(const std::string &instance_id) {
    auto it = exec_states_.find(instance_id);
    return it != exec_states_.end() ? it->second : nullptr;
  }

  inline VNode *GetRootVNode(const std::string &instance_id) {
    auto it = vnode_trees_.find(instance_id);
    return it != vnode_trees_.end() ? it->second : nullptr;
  }

 private:
  void InitVM();
  bool CreatePageInternal(const std::string &page_id, VNode *v_node);
  bool RefreshPageInternal(const std::string &page_id, VNode *new_node);
  bool ClosePageInternal(const std::string &page_id);
  void DownloadAndExecScript(ExecState *exec_state, const std::string &page_id,
                             std::function<void(const char *)> exec_js);

  std::string CreatePageWithContent(const uint8_t *contents, size_t length,
                                    const std::string &page_id,
                                    const std::string &options,
                                    const std::string &init_data,
                                    std::function<void(const char*)> exec_js);
  std::string CreatePageWithContent(const std::string &input,
                                    const std::string &page_id,
                                    const std::string &options,
                                    const std::string &init_data,
                                    std::function<void(const char *)> exec_js);

  static VM *g_vm;
  static VNodeRenderManager *g_instance;

  std::map<std::string, VNode *> vnode_trees_;
  std::map<std::string, ExecState *> exec_states_;
  std::vector<std::string> modules_;
};
}  // namespace data_render
}  // namespace core
}  // namespace weex
#endif  // CORE_DATA_RENDER_VNODE_VNODE_RENDER_MANAGER_
