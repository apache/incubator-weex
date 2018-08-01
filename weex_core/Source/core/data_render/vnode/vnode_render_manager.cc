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

#include "core/data_render/vnode/vnode_render_manager.h"
#include <chrono>
#include <sstream>
#include "core/data_render/exec_state.h"
#include "core/data_render/string_table.h"
#include "core/data_render/vnode/vnode.h"
#include "core/data_render/vnode/vnode_exec_env.h"
#include "core/manager/weex_core_manager.h"
#include "core/render/manager/render_manager.h"
#include "core/render/node/factory/render_creator.h"

#define VRENDER_LOG true

#if VRENDER_LOG

#include "android/base/log_utils.h"

#endif

namespace weex {
namespace core {
namespace data_render {

using std::map;
using std::pair;
using std::string;
using std::vector;
using WeexCore::RenderManager;

VNodeRenderManager* VNodeRenderManager::g_instance = nullptr;
VM* VNodeRenderManager::g_vm = nullptr;
// TODO establish linkages between page ref_id
int ref_id = 0;

WeexCore::RenderObject* ParseVNode2RenderObject(VNode* vnode,
                                                WeexCore::RenderObject* parent, bool isRoot,
                                                int index,
                                                const string& pageId) {
  std::stringstream ss;
  ss << ref_id++;
  std::string ref_str = isRoot ? "_root" : ss.str();
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
  //  std::set<string> *event = vnode->event();
  //  renderObject->events()->insert(event->begin(), event->end());

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

WeexCore::RenderObject* VNode2RenderObject(VNode* root, const string& page_id) {
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

void VNodeRenderManager::CreatePage(const std::string& input,
                                    const std::string& page_id,
                                    const std::string& init_data) {
  InitVM();
  auto start = std::chrono::steady_clock::now();

  ExecState* exec_state = new ExecState(g_vm);
  exec_states_.insert({page_id, exec_state});

  VNodeExecEnv::InitCFuncEnv(exec_state);

  std::string err;
  exec_state->context()->raw_json() = json11::Json::parse(input, err);

  VNodeExecEnv::InitGlobalValue(exec_state);
  VNodeExecEnv::InitInitDataValue(exec_state, init_data);
  VNodeExecEnv::InitStyleList(exec_state);

  exec_state->context()->page_id(page_id);
  auto compile_start = std::chrono::steady_clock::now();
  exec_state->Compile();

  auto exec_start = std::chrono::steady_clock::now();
  exec_state->Execute();

  CreatePageInternal(page_id, exec_state->context()->root());
  exec_state->context()->Reset();

  auto duration_post = std::chrono::duration_cast<std::chrono::milliseconds>(
      std::chrono::steady_clock::now() - start);

  LOGE("DATA_RENDER, All time %lld", duration_post.count());
}
bool VNodeRenderManager::RefreshPage(const std::string& page_id,
                                     const std::string& init_data) {
  auto it = exec_states_.find(page_id);
  if (it == exec_states_.end()) {
    return false;
  }
  ExecState* exec_state = it->second;

  VNodeExecEnv::InitInitDataValue(exec_state, init_data);

  exec_state->Execute();  // refresh root
  RefreshPageInternal(page_id, exec_state->context()->root());
  exec_state->context()->Reset();
  WeexCore::WeexCoreManager::getInstance()
      ->getPlatformBridge()
      ->callRefreshFinish(page_id.c_str(), nullptr, "");
  return true;
}

bool VNodeRenderManager::ClosePage(const std::string& page_id) {
  auto it = exec_states_.find(page_id);
  if (it == exec_states_.end()) {
    return false;
  }
  ExecState* exec_state = it->second;

  ClosePageInternal(page_id);
  delete exec_state;
  return true;
}

void PatchVNode(const string& page_id, VNode* old_node, VNode* new_node);

bool SameNode(VNode* a, VNode* b) {
  return a->tag_name() == b->tag_name() &&
         a->ref() == b->ref();  // todo to be more accurate
}

inline VNode* GetOrNull(vector<VNode*>& vec, unsigned int index) {
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
  unsigned int old_end = old_children.size() - 1;
  unsigned int new_start = 0;
  unsigned int new_end = new_children.size() - 1;
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
  int index = std::distance(ref_list.begin(), insert_pos);
  ref_list.insert(insert_pos, node);

  WeexCore::RenderObject* root = ParseVNode2RenderObject(node, nullptr, false, 0, page_id);
  RenderManager::GetInstance()->AddRenderObject(
      page_id, node->parent()->render_object_ref(), index, root);
}

int MoveToBackOfRef(vector<VNode*>& ref_list, const VNode* move_ref,
                    const VNode* anchor_ref) {
  auto move_pos = IndexOf(ref_list, move_ref);
  int index = std::distance(ref_list.begin(), move_pos);
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
  int index = std::distance(ref_list.begin(), move_pos);
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

    WeexCore::RenderObject* root = VNode2RenderObject(p_node, pageId);
    RenderManager::GetInstance()->AddRenderObject(
        pageId, p_node->parent()->render_object_ref(), i, root);
  }
}

void RemoveNodes(const string& pageId, vector<VNode*>& vec,
                 vector<VNode*>& ref_list, unsigned int start,
                 unsigned int end) {
  for (int i = start; i <= end; ++i) {
    auto p_node = vec[start];
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
  RenderManager::GetInstance()->UpdateAttr(
      page_id, new_node->render_object_ref(), p_vec);

  // compare style, ptr will be delete by RenderPage
  p_vec = CompareMap(*(old_node->styles()), *(new_node->styles()));
  RenderManager::GetInstance()->UpdateStyle(
      page_id, new_node->render_object_ref(), p_vec);

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
      WeexCore::RenderObject* root = VNode2RenderObject(*it, page_id);
      RenderManager::GetInstance()->AddRenderObject(
          page_id, (*it)->parent()->render_object_ref(), index, root);
      ++index;
    }
  }
}

void Patch(const string& page_id, VNode* old_root, VNode* new_root) {
  // root must be the same;
  PatchVNode(page_id, old_root, new_root);
}
}  // namespace data_render
}  // namespace core
}  // namespace weex
