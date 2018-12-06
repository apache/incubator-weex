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

#include <chrono>
#include <sstream>
#include "core/data_render/vnode/vnode_render_manager.h"
#include "core/data_render/common_error.h"
#include "core/data_render/exec_state_binary.h"
#include "core/data_render/exec_state.h"
#include "core/data_render/string_table.h"
#include "core/data_render/vnode/vnode.h"
#include "core/data_render/vnode/vnode_exec_env.h"
#include "core/manager/weex_core_manager.h"
#include "core/render/manager/render_manager.h"
#include "core/render/node/factory/render_creator.h"
#include "core/bridge/platform_bridge.h"

#define VRENDER_LOG true

#if VRENDER_LOG

#include "base/LogDefines.h"

#endif

namespace weex {
namespace core {
namespace data_render {

using std::map;
using std::pair;
using std::string;
using std::vector;
using WeexCore::RenderManager;

void PatchVNode(const string& page_id, VNode* old_node, VNode* new_node);

VNodeRenderManager* VNodeRenderManager::g_instance = nullptr;
VM* VNodeRenderManager::g_vm = nullptr;
// TODO establish linkages between page ref_id
int ref_id = 0;

WeexCore::RenderObject* ParseVNode2RenderObject(VNode* vnode,
                                                WeexCore::RenderObject* parent, bool isRoot,
                                                int index,
                                                const string& pageId) {
  std::string ref_str;
  if (isRoot) {
    ref_str = "_root";
  } else if (!vnode->node_id().empty()) {
    ref_str = vnode->node_id();
  } else {
    ref_str = base::to_string(ref_id++);
  }

  WeexCore::RenderObject* render_object = static_cast<WeexCore::RenderObject*>(
      WeexCore::RenderCreator::GetInstance()->CreateRender(vnode->tag_name(),
                                                           ref_str));
  // Link RenderObject and VNode
  vnode->set_render_object_ref(std::move(ref_str));

  // style
  map<string, string>* style = vnode->styles();
  for (auto it = style->begin(); it != style->end(); it++) {
    render_object->AddStyle(it->first, it->second);
  }

  // attr
  map<string, string>* attr = vnode->attributes();
  for (auto it = attr->begin(); it != attr->end(); it++) {
    render_object->AddAttr(it->first, it->second);
  }

  // event,todo
  std::map<std::string, void *> *events = vnode->events();
  for (auto iter = events->begin(); iter != events->end(); iter++) {
      render_object->events()->insert(iter->first);
  }

  // child
  vector<VNode*>* children = (const_cast<VNode*>(vnode))->child_list();
  for (int i = 0; i < children->size(); i++) {
    ParseVNode2RenderObject((*children)[i], render_object, false, i, pageId);
  }

  render_object->set_page_id(pageId);

  render_object->ApplyDefaultStyle();
  render_object->ApplyDefaultAttr();

  // parent
  if (parent != nullptr) {
    parent->AddRenderObject(index, render_object);
  }

  return render_object;
}

WeexCore::RenderObject *VNode2RenderObject(VNode *root, const string& page_id) {
  return ParseVNode2RenderObject(root, nullptr, true, 0, page_id);
}

void Patch(const string& page_id, VNode* old_root, VNode* new_root);

bool VNodeRenderManager::CreatePageInternal(const string& page_id,
                                            VNode* vNode) {
  auto node = vnode_trees_.find(page_id);
  if (node != vnode_trees_.end()) {
    delete node->second;
    node->second = nullptr;
  }
  vnode_trees_[page_id] = vNode;

  auto render_root = VNode2RenderObject(vNode, page_id);

  RenderManager::GetInstance()->CreatePage(page_id, render_root);
  RenderManager::GetInstance()->CreateFinish(page_id);
  return true;
}

bool VNodeRenderManager::RefreshPageInternal(const string& page_id,
                                             VNode* new_node) {
  auto node = vnode_trees_.find(page_id);
  if (node == vnode_trees_.end()) {
    return false;
  }

  auto oldNode = node->second;
  Patch(page_id, oldNode, new_node);
  node->second = new_node;
  delete oldNode;
  return true;
}

bool VNodeRenderManager::ClosePageInternal(const string& page_id) {
  auto node = vnode_trees_.find(page_id);
  if (node == vnode_trees_.end()) {
    return false;
  }
  RenderManager::GetInstance()->ClosePage(page_id);
  delete node->second;
  vnode_trees_.erase(node);
  return true;
}

void VNodeRenderManager::InitVM() {
  if (g_vm == nullptr) {
    g_vm = new VM();
  }
}

void VNodeRenderManager::CreatePage(const std::string &input, const std::string &page_id, const  std::string &options, const std::string &init_data) {
    std::string err = CreatePageWithContent(input, page_id, options, init_data);
    if (!err.empty()) {
        WeexCore::WeexCoreManager::Instance()->getPlatformBridge()->platform_side()->ReportException(page_id.c_str(), nullptr, err.c_str());
    }
}

std::string VNodeRenderManager::CreatePageWithContent(const std::string &input, const std::string &page_id, const std::string &options, const std::string &init_data) {
    InitVM();
#ifdef DEBUG
    auto start = std::chrono::steady_clock::now();
#endif
    ExecState *exec_state = new ExecState(g_vm);
    exec_states_.insert({page_id, exec_state});
    VNodeExecEnv::ImportExecEnv(exec_state);
    std::string err;
    json11::Json json = json11::Json::parse(input, err);
    if (!err.empty() || json.is_null()) {
        exec_state->context()->raw_source() = input;
    }
    else {
        exec_state->context()->raw_json() = json;
        VNodeExecEnv::ParseData(exec_state);
        VNodeExecEnv::ParseStyle(exec_state);
        VNodeExecEnv::ParseScript(exec_state);
    }
    if (init_data.length() > 0) {
        VNodeExecEnv::ImportExecData(exec_state, init_data);
    }
    exec_state->context()->page_id(page_id);
    //auto compile_start = std::chrono::steady_clock::now();
    exec_state->Compile(err);
    if (!err.empty()) {
        LOGE("DATA_RENDER, compile err: %s",err.c_str());
        return err;
    }
#ifdef DEBUG
    auto compile_post = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start);
    LOGD("[DATA_RENDER], Compile time:[%lld]\n", compile_post.count());
#endif
    //auto exec_start = std::chrono::steady_clock::now();
    exec_state->Execute(err);
    if (!err.empty()) {
        LOGE("DATA_RENDER, exec err: %s",err.c_str());
        return err;
    }
    if (exec_state->context()->root() == NULL) {
        return err;
    }
    CreatePageInternal(page_id, exec_state->context()->root());
#ifdef DEBUG
    auto duration_post = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start);
    LOGD("DATA_RENDER, All time %lld\n", duration_post.count());
#endif
    return err;
}
    
bool VNodeRenderManager::RequireModule(ExecState *exec_state, std::string &name, std::string &result)
{
    bool finished = false;
    do {
        if (!modules_.size()) {
            break;
        }
        for (auto iter = modules_.begin(); iter != modules_.end(); iter++) {
            if ((*iter).find(name) <= 10) {
                result = *iter;
                finished = true;
                break;
            }
        }
        
    } while (0);
    
    return finished;
}
    
void VNodeRenderManager::ExecuteRegisterModules(ExecState *exec_state, std::vector<std::string>& registers) {
    do {
        if (!modules_.size()) {
            break;
        }
        const std::string func_name = "registerModule";
        for (auto iter = modules_.begin(); iter != modules_.end(); iter++) {
            for (int j = 0; j < registers.size(); j++) {
                std::string prefix = registers[j];
                if ((*iter).find(prefix) <= 10) {
                    Value arg = StringToValue(exec_state, *iter);
                    exec_state->Call(func_name, {arg});
                    break;
                }
            }
        }
        
    } while (0);
}

std::string VNodeRenderManager::CreatePageWithContent(const uint8_t *contents, size_t length, const std::string &page_id, const std::string &options, const std::string &init_data) {
    InitVM();
#ifdef DEBUG
    auto start = std::chrono::steady_clock::now();
#endif
    ExecState *exec_state = new ExecState(g_vm);
    exec_states_.insert({page_id, exec_state});
    VNodeExecEnv::ImportExecEnv(exec_state);
    exec_state->context()->page_id(page_id);
    std::string err;
    if (!weex::core::data_render::WXExecDecoder(exec_state, (uint8_t *)contents, length, err)) {
        return err;
    }
    if (init_data.length() > 0) {
        VNodeExecEnv::ImportExecData(exec_state, init_data);
    }
#ifdef DEBUG
    auto decoder_post = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start);
    LOGD("[DATA_RENDER], Decoder time:[%lld]\n", decoder_post.count());
#endif
    exec_state->Execute(err);
    if (!err.empty()) {
        return err;
    }
    if (exec_state->context()->root() == NULL) {
        err = "Root vonde is null";
        return err;
    }
    CreatePageInternal(page_id, exec_state->context()->root());
#ifdef DEBUG
    auto duration_post = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start);
    LOGD("[DATA_RENDER], All time:[%lld]\n", duration_post.count());
#endif
    return err;
}

void VNodeRenderManager::CreatePage(const char *contents, size_t length, const std::string& page_id, const std::string& options, const std::string& init_data) {
    string err = CreatePageWithContent((const uint8_t *)contents, length, page_id, options, init_data);
    if (!err.empty()) {
        WeexCore::WeexCoreManager::Instance()->getPlatformBridge()->platform_side()->ReportException(page_id.c_str(), nullptr, err.c_str());
    }
}

bool VNodeRenderManager::RefreshPage(const std::string& page_id,
                                     const std::string& init_data) {
    do {
        auto it = exec_states_.find(page_id);
        if (it == exec_states_.end()) {
            break;
        }
        ExecState *exec_state = it->second;
        VNodeExecEnv::ImportExecData(exec_state, init_data);
        std::string err;
        exec_state->context()->Reset();
        exec_state->Execute(err);  // refresh root
        if (!err.empty()) {
            break;
        }
        if (exec_state->context()->root() == NULL) {
            break;
        }
        RefreshPageInternal(page_id, exec_state->context()->root());
        WeexCore::WeexCoreManager::Instance()
        ->getPlatformBridge()
        ->platform_side()
        ->RefreshFinish(page_id.c_str(), nullptr, "");
        return true;
        
    } while (0);
    
    return false;
}

bool VNodeRenderManager::ClosePage(const std::string& page_id) {
  auto it = exec_states_.find(page_id);
  if (it == exec_states_.end()) {
    return false;
  }
  ExecState *exec_state = it->second;
  ClosePageInternal(page_id);
  delete exec_state;
  exec_states_.erase(it);
  return true;
}
        
void VNodeRenderManager::FireEvent(const std::string &page_id, const std::string &ref, const std::string &event,const std::string &args) {
    do {
        auto iter = exec_states_.find(page_id);
        if (iter == exec_states_.end()) {
            break;
        }
        auto node = vnode_trees_.find(page_id);
        if (node == vnode_trees_.end()) {
            break;
        }
        auto vnode = node->second->FindNode(ref);
        if (!vnode) {
            break;
        }
        auto iter_event = vnode->events()->find(event);
        if (iter_event == vnode->events()->end()) {
            break;
        }
        if (!iter_event->second) {
            break;
        }
        FuncState *func_state = nullptr;
        FuncInstance *func_inst = nullptr;
        ExecState *exec_state = iter->second;
        bool finder = false;
        for (auto iter : exec_state->class_factory()->stores()) {
            if (iter.first == iter_event->second) {
                if (iter.second == Value::Type::FUNC_INST) {
                    func_inst = reinterpret_cast<FuncInstance *>(iter.first);
                }
                finder = true;
            }
        }
        if (!finder) {
            func_state = reinterpret_cast<FuncState *>(iter_event->second);
        }
        if (!func_state && !func_inst) {
            break;
        }
        std::vector<Value> caller_args;
        if (func_inst) {
            func_state = func_inst->func_;
        }
        if (func_state->is_class_func() && vnode->inst()) {
            Value inst;
            SetCIValue(&inst, reinterpret_cast<GCObject *>(vnode->inst()));
            caller_args.push_back(inst);
        }
        caller_args.push_back(StringToValue(exec_state, args));
        if (func_inst) {
            exec_state->Call(func_inst, caller_args);
        }
        else {
            exec_state->Call(func_state, caller_args);
        }
        
    } while (0);
}

void VNodeRenderManager::CallNativeModule(ExecState *exec_state,
                                          const std::string& module,
                                          const std::string& method,
                                          const std::string& args, int argc) {
  for (auto iter = exec_states_.begin(); iter != exec_states_.end(); iter++) {
    if (iter->second == exec_state) {
      WeexCoreManager::Instance()
          ->getPlatformBridge()
          ->platform_side()
          ->CallNativeModule(iter->first.c_str(), module.c_str(),
                             method.c_str(),
                             args.length() > 0 ? args.c_str() : nullptr,
                             static_cast<int>(args.length()), nullptr, 0);
      break;
    }
  }
}
    
void VNodeRenderManager::WXLogNative(ExecState *exec_state, const std::string &info) {
    for (auto iter = exec_states_.begin(); iter != exec_states_.end(); iter++) {
        if (iter->second == exec_state) {
            WeexCoreManager::Instance()
            ->getPlatformBridge()
            ->platform_side()
            ->NativeLog(info.c_str());
            break;
        }
    }
}

void VNodeRenderManager::PatchVNode(ExecState *exec_state, VNode *v_node, VNode *new_node) {
    do {
        for (auto iter = exec_states_.begin(); iter != exec_states_.end(); iter++) {
            if (iter->second == exec_state) {
                Patch(iter->first, v_node, new_node);
                break;
            }
        }
        
    } while (0);
}
    
bool SameNode(VNode* a, VNode* b) {
  return a->tag_name() == b->tag_name() &&
         a->ref() == b->ref();  // todo to be more accurate
}

inline VNode* GetOrNull(vector<VNode*>& vec, int index) {
  if (index < 0 || index >= vec.size()) {
    return nullptr;
  }
  return vec[index];
}

void RemoveNodes(const string& pageId, vector<VNode*>& vec,
                 vector<VNode*>& ref_list, unsigned int start,
                 unsigned int end);

void AddNodes(const string& pageId, vector<VNode*>& vec,
              vector<VNode*>& ref_list, unsigned int start, unsigned int end);

inline vector<VNode*>::iterator IndexOf(vector<VNode*>& vec,
                                        const VNode* value) {
  return std::find(vec.begin(), vec.end(), value);
}

int MoveToBackOfRef(vector<VNode*>& ref_list, const VNode* move_ref,
                    const VNode* anchor_ref);

int MoveToFrontOfRef(vector<VNode*>& ref_list, const VNode* move_ref,
                     const VNode* anchor_ref);

int MoveElmToFrontOfNode(const string& page_id, vector<VNode*>& ref_list,
                         VNode* move_node, VNode* anchor_node);

int MoveElmToBackOfNode(const string& page_id, vector<VNode*>& ref_list,
                        VNode* move_node, VNode* anchor_node);

void CreateAndInsertElm(const string& page_id, VNode* node,
                        vector<VNode*>& ref_list, const VNode* ref);

void UpdateChildren(const string& page_id, VNode* old_node, VNode* new_node) {
  vector<VNode*>& old_children = *old_node->child_list();
  vector<VNode*>& new_children = *new_node->child_list();
  vector<VNode*> ref_list;
  map<string, unsigned int>* ref_to_index = nullptr;

  // ref
  for (auto begin = old_children.begin(); begin < old_children.end(); begin++) {
    ref_list.push_back((*begin));
  }

  unsigned int old_start = 0;
  unsigned int old_end = static_cast<unsigned int >(old_children.size()) - 1;
  unsigned int new_start = 0;
  unsigned int new_end = static_cast<unsigned int >(new_children.size()) - 1;
  VNode* old_start_node = GetOrNull(old_children, old_start);
  VNode* old_end_node = GetOrNull(old_children, old_end);
  VNode* new_start_node = GetOrNull(new_children, new_start);
  VNode* new_end_node = GetOrNull(new_children, new_end);

  while (old_start <= old_end && new_start <= new_end) {
    if (old_start_node == nullptr) {
      //++oldStart might larger than children size;
      old_start_node = GetOrNull(old_children, ++old_start);

    } else if (old_end_node == nullptr) {
      old_end_node = GetOrNull(old_children, --old_end);

    } else if (SameNode(old_start_node, new_start_node)) {
      PatchVNode(page_id, old_start_node, new_start_node);
      old_start_node = GetOrNull(old_children, ++old_start);
      new_start_node = GetOrNull(new_children, ++new_start);

    } else if (SameNode(old_end_node, new_end_node)) {
      PatchVNode(page_id, old_end_node, new_end_node);
      old_end_node = GetOrNull(old_children, --old_end);
      new_end_node = GetOrNull(new_children, --new_end);

    } else if (SameNode(old_start_node, new_end_node)) {
      PatchVNode(page_id, old_start_node, new_end_node);
      MoveElmToBackOfNode(page_id, ref_list, old_start_node, old_end_node);
      old_start_node = GetOrNull(old_children, ++old_start);
      new_end_node = GetOrNull(new_children, --new_end);

    } else if (SameNode(old_end_node, new_start_node)) {
      PatchVNode(page_id, old_end_node, new_start_node);
      MoveElmToFrontOfNode(page_id, ref_list, old_end_node, old_start_node);
      old_end_node = GetOrNull(old_children, --old_end);
      new_start_node = GetOrNull(new_children, ++new_start);
    } else {
      // create on first time
      if (ref_to_index == nullptr) {
        ref_to_index = new map<string, unsigned int>();
        for (unsigned int i = old_start; i <= old_end; ++i) {
          auto vnode = GetOrNull(old_children, i);
          if (vnode == nullptr) {
            continue;
          }
          ref_to_index->insert({vnode->ref(), i});
        }
      }

      auto pos = ref_to_index->find(new_start_node->ref());
      if (pos == ref_to_index->end()) {
        // no node found, create new
        CreateAndInsertElm(page_id, new_start_node, ref_list, old_start_node);
        new_start_node = GetOrNull(new_children, ++new_start);
      } else {
        auto node_to_move = GetOrNull(old_children, pos->second);
        if (node_to_move == nullptr) {
          // wtf!
#if VRENDER_LOG
          LOGE("[VRenderManager] already moved, has duplicated ref: %s",
               new_start_node->ref().c_str());
#endif
          continue;
        }
        if (SameNode(node_to_move, new_start_node)) {
          PatchVNode(page_id, node_to_move, new_start_node);

          // delete old node.
          *IndexOf(ref_list, node_to_move) = new_start_node;
          delete old_children[pos->second];
          old_children[pos->second] = nullptr;

          MoveElmToFrontOfNode(page_id, ref_list, new_start_node,
                               old_start_node);
          new_start_node = GetOrNull(new_children, ++new_start);
        } else {
          // same ref exist, after insert there will be duplicated ref.
          // so must remove it in real dom first.
          RenderManager::GetInstance()->RemoveRenderObject(
              page_id, node_to_move->render_object_ref());
          ref_list.erase(IndexOf(ref_list, node_to_move));
          delete old_children[pos->second];
          old_children[pos->second] = nullptr;
          // no need to remove from keyToIndex, will go into wtf above;

          CreateAndInsertElm(page_id, new_start_node, ref_list, old_start_node);
          new_start_node = GetOrNull(new_children, ++new_start);
        }
      }
    }
  }

  if (old_start > old_end) {
    AddNodes(page_id, new_children, ref_list, new_start,
             new_end);  // end is include
  } else if (new_start > new_end) {
    RemoveNodes(page_id, old_children, ref_list, old_start, old_end);
  }

  if (ref_to_index != nullptr) {
    delete ref_to_index;
    ref_to_index = nullptr;
  }
}

void CreateAndInsertElm(const string& page_id, VNode* node,
                        vector<VNode*>& ref_list, const VNode* ref) {
  auto insert_pos = IndexOf(ref_list, ref);
  int index = static_cast<int>(std::distance(ref_list.begin(), insert_pos));
  ref_list.insert(insert_pos, node);

  WeexCore::RenderObject* root = ParseVNode2RenderObject(node, nullptr, false, 0, page_id);
  RenderManager::GetInstance()->AddRenderObject(
      page_id, node->parent()->render_object_ref(), index, root);
}

int MoveToBackOfRef(vector<VNode*>& ref_list, const VNode* move_ref,
                    const VNode* anchor_ref) {
  auto move_pos = IndexOf(ref_list, move_ref);
  int index = static_cast<int>(std::distance(ref_list.begin(), move_pos));
  if (move_pos == ref_list.end()) {
#if VRENDER_LOG
    LOGE("[VRenderManager] moveToBackOfRef movePos == refList.end() ref: %s",
         move_ref->ref().c_str());
#endif
    return -1;  // wtf!
  }
  VNode* value = *move_pos;
  ref_list.erase(move_pos);
  auto anchor_pos = IndexOf(ref_list, anchor_ref);
  if (anchor_pos == ref_list.end()) {
#if VRENDER_LOG
    LOGE("[VRenderManager] moveToBackOfRef anchorPos == refList.end() ref: %s",
         anchor_ref->ref().c_str());
#endif
    return -1;  // wtf
  }
  ref_list.insert(++anchor_pos, value);
  return index;
}

int MoveToFrontOfRef(vector<VNode*>& ref_list, const VNode* move_ref,
                     const VNode* anchor_ref) {
  auto move_pos = IndexOf(ref_list, move_ref);
  int index = static_cast<int>(std::distance(ref_list.begin(), move_pos));
  if (move_pos == ref_list.end()) {
#if VRENDER_LOG
    LOGE("[VRenderManager] moveToFrontOfRef movePos == refList.end() ref: %s",
         move_ref->ref().c_str());
#endif
    return -1;  // wtf!
  }
  VNode* value = *move_pos;
  ref_list.erase(move_pos);
  auto anchor_pos = IndexOf(ref_list, anchor_ref);
  if (anchor_pos == ref_list.end()) {
#if VRENDER_LOG
    LOGE("[VRenderManager] moveToFrontOfRef anchorPos == refList.end() ref: %s",
         anchor_ref->ref().c_str());
#endif
    return -1;  // wtf
  }
  ref_list.insert(anchor_pos, value);
  return index;
}

int MoveElmToFrontOfNode(const string& page_id, vector<VNode*>& ref_list,
                         VNode* move_node, VNode* anchor_node) {
  int move_to_index = MoveToFrontOfRef(ref_list, move_node, anchor_node);
  RenderManager::GetInstance()->MoveRenderObject(
      page_id, move_node->render_object_ref(),
      move_node->parent()->render_object_ref(), move_to_index);
  return move_to_index;
}

int MoveElmToBackOfNode(const string& page_id, vector<VNode*>& ref_list,
                        VNode* move_node, VNode* anchor_node) {
  int move_to_index = MoveToBackOfRef(ref_list, move_node, anchor_node);
  RenderManager::GetInstance()->MoveRenderObject(
      page_id, move_node->render_object_ref(),
      move_node->parent()->render_object_ref(), move_to_index);
  return move_to_index;
}

void AddNodes(const string& pageId, vector<VNode*>& vec,
              vector<VNode*>& ref_list, unsigned int start, unsigned int end) {
  for (int i = start; i <= end; ++i) {
    auto p_node = vec[i];
    ref_list.insert(ref_list.begin() + i, p_node);
    WeexCore::RenderObject *node = ParseVNode2RenderObject(p_node, nullptr, false, 0, pageId);
    RenderManager::GetInstance()->AddRenderObject(
        pageId, p_node->parent()->render_object_ref(), i, node);
  }
}

void RemoveNodes(const string& pageId, vector<VNode*>& vec,
                 vector<VNode*>& ref_list, unsigned int start,
                 unsigned int end) {
  for (int i = start; i <= end; ++i) {
    auto p_node = vec[i];
    // some might already been used for patch, which is null.
    if (p_node == nullptr) {
      continue;
    }
    auto pos = IndexOf(ref_list, p_node);
    if (pos == ref_list.end()) {
      // wtf???
#if VRENDER_LOG
      LOGE("[VRenderManager] removeNodes pos == refList.end() ref: %s",
           p_node->ref().c_str());
#endif
      continue;
    }

    RenderManager::GetInstance()->RemoveRenderObject(
        pageId, (*pos)->render_object_ref());
  }
}

vector<pair<string, string>>* CompareMap(const map<string, string>& oldMap,
                                         const map<string, string>& newMap) {
  auto p_vec = new vector<pair<string, string>>();
  for (auto it = newMap.cbegin(); it != newMap.cend(); it++) {
    auto pos = oldMap.find(it->first);

    if (pos == oldMap.end() || pos->second != it->second) {
      // key not exist, or value not same
      p_vec->push_back({it->first, it->second});
    }
  }

  for (auto it = oldMap.cbegin(); it != oldMap.cend(); it++) {
    auto pos = newMap.find(it->first);

    if (pos == newMap.end()) {
      // key not exist, remove //todo check if this is correct
      p_vec->push_back({it->first, ""});
    }
  }
  return p_vec;
};
 
void PatchVNode(const string& page_id, VNode* old_node, VNode* new_node) {
  // patch render object link
  new_node->set_render_object_ref(old_node->render_object_ref());

  // compare attr, ptr will be delete by RenderPage
  auto p_vec = CompareMap(*(old_node->attributes()), *(new_node->attributes()));
  if (p_vec->size() > 0) {
      RenderManager::GetInstance()->UpdateAttr(page_id, new_node->render_object_ref(), p_vec);
  }
  else {
      delete p_vec;
      p_vec = nullptr;
  }
  // compare style, ptr will be delete by RenderPage
  p_vec = CompareMap(*(old_node->styles()), *(new_node->styles()));
  if (p_vec->size()) {
      RenderManager::GetInstance()->UpdateStyle(page_id, new_node->render_object_ref(), p_vec);
  }
  else {
      delete p_vec;
      p_vec = nullptr;
  }

  // compare event
  // todo

  // compare children
  if (old_node->HasChildren() && new_node->HasChildren()) {
    UpdateChildren(page_id, old_node, new_node);
  } else if (old_node->HasChildren() && !new_node->HasChildren()) {
    for (auto i = old_node->child_list()->cbegin();
         i != old_node->child_list()->cend(); i++) {
      RenderManager::GetInstance()->RemoveRenderObject(
          page_id, (*i)->render_object_ref());
    }
  } else if (!old_node->HasChildren() && new_node->HasChildren()) {
    int index = 0;
    for (auto it = new_node->child_list()->cbegin();
         it != new_node->child_list()->cend(); it++) {
      WeexCore::RenderObject *root = VNode2RenderObject(*it, page_id);
      RenderManager::GetInstance()->AddRenderObject(
          page_id, (*it)->parent()->render_object_ref(), index, root);
      ++index;
    }
  }
}

void Patch(const string& page_id, VNode *old_node, VNode *new_node) {
    if (old_node->parent() == NULL || SameNode(old_node, new_node)) {
        // root must be the same;
        PatchVNode(page_id, old_node, new_node);
    }
    else {
        VNode *parent = (VNode *)old_node->parent();
        vector<VNode *> &old_children = *parent->child_list();
        WeexCore::RenderObject *new_render_object = ParseVNode2RenderObject(new_node, nullptr, false, 0, page_id);
        auto pos = std::find(old_children.begin(), old_children.end(), old_node);
        int index = static_cast<int>(std::distance(old_children.begin(), pos));
        parent->InsertChild(new_node, index);
        RenderManager::GetInstance()->AddRenderObject(page_id, parent->render_object_ref(), index, new_render_object);
        parent->RemoveChild(old_node);
        RenderManager::GetInstance()->RemoveRenderObject(page_id, old_node->render_object_ref());
    }
}
    
}  // namespace data_render
}  // namespace core
}  // namespace weex
