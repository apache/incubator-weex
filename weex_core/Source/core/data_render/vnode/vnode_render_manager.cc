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

#include "core/render/manager/render_manager.h"
#include "core/render/node/factory/render_creator.h"
#include "core/data_render/exec_state.h"
#include "core/data_render/string_table.h"
#include "core/data_render/vnode/vnode_render_manager.h"
#include "core/data_render/vnode/vnode.h"
#include <sstream>
#include <chrono>

#define VRENDER_LOG true

#if VRENDER_LOG

#include <android/base/log_utils.h>

#endif

namespace weex {
namespace core {
namespace data_render {

using std::map;
using std::vector;
using std::string;
using std::pair;
using WeexCore::RenderManager;

VNodeRenderManager* VNodeRenderManager::g_pInstance = nullptr;
VM* VNodeRenderManager::g_vm_ = nullptr;

WeexCore::RenderObject* parseVNode2RenderObject(const VNode* vnode, WeexCore::RenderObject* parent,
                                                int index, const string& pageId) {
  WeexCore::RenderObject* render_object = static_cast<WeexCore::RenderObject*>(WeexCore::RenderCreator::GetInstance()->CreateRender(
      vnode->tag_name(), vnode->ref()));
  //style
  map<string, string>* style = vnode->styles();
  for (auto it = style->begin(); it != style->end(); it++) {
    render_object->AddStyle(it->first, it->second);
  }

  //attr
  map<string, string>* attr = vnode->attributes();
  for (auto it = attr->begin(); it != attr->end(); it++) {
    render_object->AddAttr(it->first, it->second);
  }

  //event,todo
//  std::set<string> *event = vnode->event();
//  renderObject->events()->insert(event->begin(), event->end());

  //child
  vector<VNode*>* children = (const_cast<VNode*>(vnode))->child_list();
  for (int i = 0; i < children->size(); i++) {
    parseVNode2RenderObject((*children)[i], render_object, i, pageId);
  }

  render_object->set_page_id(pageId);

  render_object->ApplyDefaultStyle();
  render_object->ApplyDefaultAttr();


  //parent
  if (parent != nullptr) {
    parent->AddRenderObject(index, render_object);
  }
  return render_object;
}


WeexCore::RenderObject* VNode2RenderObject(const VNode* root, const string& page_id) {
  return parseVNode2RenderObject(root, nullptr, 0, page_id);
}


void patch(const string& page_id, VNode* old_root, VNode* new_root);

bool VNodeRenderManager::CreatePage(const string& page_id, VNode* vNode) {
  auto node = vnode_trees_.find(page_id);
  if (node != vnode_trees_.end()) {
    delete node->second;
    node->second = nullptr;
  }
  vnode_trees_[page_id] = vNode;

  auto v_2_ro = std::chrono::steady_clock::now();
  //update tree
  auto render_root = VNode2RenderObject(vNode, page_id);
  auto duration_v_2_ro = std::chrono::duration_cast<std::chrono::milliseconds>(
      std::chrono::steady_clock::now() - v_2_ro);
  LOGE("DATA_RENDER, v_2_ro time %lld", duration_v_2_ro.count());

  auto create_page_start = std::chrono::steady_clock::now();
  RenderManager::GetInstance()->CreatePage(page_id, render_root);
  auto duration_create_page = std::chrono::duration_cast<std::chrono::milliseconds>(
      std::chrono::steady_clock::now() - create_page_start);
  LOGE("DATA_RENDER, create_page time %lld", duration_create_page.count());

  RenderManager::GetInstance()->CreateFinish(page_id);
  return true;
}

bool VNodeRenderManager::RefreshPage(const string& page_id, VNode* new_node) {
  auto node = vnode_trees_.find(page_id);
  if (node == vnode_trees_.end()) {
    return false;
  }

  auto oldNode = node->second;
  patch(page_id, oldNode, new_node);
  node->second = new_node;
  delete oldNode;
  return true;
}

bool VNodeRenderManager::ClosePage(const string& page_id) {
  auto node = vnode_trees_.find(page_id);
  if (node == vnode_trees_.end()) {
    return false;
  }
  RenderManager::GetInstance()->ClosePage(page_id);
  delete node->second;
  vnode_trees_.erase(node);
  return true;
}

static Value Log(ExecState* exec_state) {
  size_t length = exec_state->GetArgumentCount();
  for (int i = 0; i < length; ++i) {
    Value* a = exec_state->GetArgument(i);
    switch (a->type) {
      case Value::Type::NUMBER:
        std::cout << a->n << "\n";
        break;
      case Value::Type::INT:
        std::cout << a->i << "\n";
        break;
      case Value::Type::STRING:
        std::cout << a->str->c_str() << "\n";
        break;
      default:
        break;
    }
  }
  return Value();
}

static Value CreateElement(ExecState* exec_state) {//createElement("tagName","id");
//  const std::string& page_name = exec_state->page_id();
  VNode* node = new VNode(
      exec_state->GetArgument(1)->str->c_str(),
      exec_state->GetArgument(0)->str->c_str()
  );
  if (exec_state->context()->root() == nullptr) {
    //set root
    exec_state->context()->setVNodeRoot(node);
  }
  exec_state->context()->insert_node(node);
  return Value();
}

static Value AppendChild(ExecState* exec_state) {//appendChild("tag","id",""parent_id");todo
  VNode* parent = exec_state->context()->find_node(exec_state->GetArgument(2)->str->c_str());
  VNode* child = exec_state->context()->find_node(exec_state->GetArgument(1)->str->c_str());
  if (parent == nullptr || child == nullptr) {
    return Value();
  }
  parent->AddChild(child);

  return Value();
}

static Value SetAttr(ExecState* exec_state) {//setAttr("id","key","value");
  VNode* node = exec_state->context()->find_node(exec_state->GetArgument(0)->str->c_str());
  if (node == nullptr) {
    return Value();
  }

  char* key = exec_state->GetArgument(1)->str->c_str();
  Value* p_value = exec_state->GetArgument(2);
  if (p_value->type == Value::STRING) {
    node->SetAttribute(key, p_value->str->c_str());
  } else if (p_value->type == Value::INT) {//todo use uniform type conversion.
    std::stringstream ss;
    ss << p_value->i;
    string str = ss.str();
    node->SetAttribute(key, str);
  }


  return Value();
}

static Value SetClassList(ExecState* exec_state) {
  VNode* node = exec_state->context()->find_node(exec_state->GetArgument(0)->str->c_str());
  char* key = exec_state->GetArgument(1)->str->c_str();

  if (node == nullptr) {
    return Value();
  }

  json11::Json& json = exec_state->context()->raw_json();
  const json11::Json& styles = json["styles"];
  const json11::Json& style = styles[key];
  if (style.is_null()) {
    return Value();
  }
  const json11::Json::object& items = style.object_items();
  for (auto it = items.begin(); it != items.end(); it++) {
    node->SetStyle(it->first, it->second.string_value());
  }
  return Value();
}

void RegisterCFunc(ExecState* state, const std::string& name, CFunction function) {
  Value func;
  func.type = Value::Type::CFUNC;
  func.cf = reinterpret_cast<void*>(function);
  state->global()->Add(name, func);
}

void VNodeRenderManager::InitVM() {
  if (g_vm_ == nullptr) {
    g_vm_ = new VM();
  }
}
void VNodeRenderManager::TestProcess(const std::string& input, const std::string& page_id) {
  auto start = std::chrono::steady_clock::now();

  ExecState* execState = new ExecState(g_vm_);

  //log
  RegisterCFunc(execState, "log", Log);
  RegisterCFunc(execState, "createElement", CreateElement);
  RegisterCFunc(execState, "appendChild", AppendChild);
  RegisterCFunc(execState, "setAttr", SetAttr);
  RegisterCFunc(execState, "setClassList", SetClassList);

  execState->context()->page_id(page_id);
  auto compile_start = std::chrono::steady_clock::now();
  execState->Compile(input);
  auto duration_compile = std::chrono::duration_cast<std::chrono::milliseconds>(
      std::chrono::steady_clock::now() - compile_start);


  auto exec_start = std::chrono::steady_clock::now();
  execState->Execute();
  auto duration_exec = std::chrono::duration_cast<std::chrono::milliseconds>(
      std::chrono::steady_clock::now() - exec_start);


  CreatePage(page_id, execState->context()->root());

  auto duration_post = std::chrono::duration_cast<std::chrono::milliseconds>(
      std::chrono::steady_clock::now() - start);

  LOGE("DATA_RENDER, All time %lld", duration_post.count());
  LOGE("DATA_RENDER, Compile time %lld", duration_compile.count());
  LOGE("DATA_RENDER, Exec time %lld", duration_exec.count());
}

void PatchVNode(const string& page_id, VNode* old_node, VNode* new_node);

bool SameNode(VNode* a, VNode* b) {
  return a->tag_name() == b->tag_name()
         && a->ref() == b->ref();//todo to be more accurate

}

inline VNode* GetOrNull(vector<VNode*>& vec, unsigned int index) {
  if (index < 0 || index >= vec.size()) {
    return nullptr;
  }
  return vec[index];
}

void RemoveNodes(const string& pageId, vector<VNode*>& vec,
                 vector<VNode*>& ref_list, unsigned int start, unsigned int end);

void AddNodes(const string& pageId, vector<VNode*>& vec,
              vector<VNode*>& ref_list, unsigned int start, unsigned int end);

inline vector<VNode*>::iterator IndexOf(
    vector<VNode*>& vec, const VNode* value) {
  return std::find(vec.begin(), vec.end(), value);
}

int MoveToBackOfRef(vector<VNode*>& ref_list, const VNode* move_ref,
                    const VNode* anchor_ref);

int MoveToFrontOfRef(vector<VNode*>& ref_list, const VNode* move_ref,
                     const VNode* anchor_ref);

int MoveElmToFrontOfNode(const string& page_id, vector<VNode*>& ref_list,
                         VNode* move_node,
                         VNode* anchor_node);

int MoveElmToBackOfNode(const string& page_id, vector<VNode*>& ref_list,
                        VNode* move_node,
                        VNode* anchor_node);

void CreateAndInsertElm(const string& page_id, VNode* node, vector<VNode*>& ref_list,
                        const VNode* ref);

void UpdateChildren(const string& page_id, VNode* old_node, VNode* new_node) {
  vector<VNode*>& old_children = *old_node->child_list();
  vector<VNode*>& new_children = *new_node->child_list();
  vector<VNode*> ref_list;
  map<string, unsigned int>* refToIndex = nullptr;

  //ref
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
      //create on first time
      if (refToIndex == nullptr) {
        refToIndex = new map<string, unsigned int>();
        for (unsigned int i = old_start; i <= old_end; ++i) {
          auto vnode = GetOrNull(old_children, i);
          if (vnode == nullptr) {
            continue;
          }
          refToIndex->insert({vnode->ref(), i});
        }
      }

      auto pos = refToIndex->find(new_start_node->ref());
      if (pos == refToIndex->end()) {
        //no node found, create new
        CreateAndInsertElm(page_id, new_start_node, ref_list, old_start_node);
        new_start_node = GetOrNull(new_children, ++new_start);
      } else {
        auto node_to_move = GetOrNull(old_children, pos->second);
        if (node_to_move == nullptr) {
          //wtf!
#if VRENDER_LOG
          LOGE("[VRenderManager] already moved, has duplicated ref: %s",
               new_start_node->ref().c_str());
#endif
          continue;
        }
        if (SameNode(node_to_move, new_start_node)) {
          PatchVNode(page_id, node_to_move, new_start_node);

          //delete old node.
          *IndexOf(ref_list, node_to_move) = new_start_node;
          delete old_children[pos->second];
          old_children[pos->second] = nullptr;

          MoveElmToFrontOfNode(page_id, ref_list, new_start_node, old_start_node);
          new_start_node = GetOrNull(new_children, ++new_start);
        } else {
          //same ref exist, after insert there will be duplicated ref.
          //so must remove it in real dom first.
          RenderManager::GetInstance()->RemoveRenderObject(page_id, node_to_move->ref());
          ref_list.erase(IndexOf(ref_list, node_to_move));
          delete old_children[pos->second];
          old_children[pos->second] = nullptr;
          //no need to remove from keyToIndex, will go into wtf above;

          CreateAndInsertElm(page_id, new_start_node, ref_list, old_start_node);
          new_start_node = GetOrNull(new_children, ++new_start);
        }
      }

    }
  }

  if (old_start > old_end) {
    AddNodes(page_id, new_children, ref_list, new_start, new_end);//end is include
  } else if (new_start > new_end) {
    RemoveNodes(page_id, old_children, ref_list, old_start, old_end);
  }

  if (refToIndex != nullptr) {
    delete refToIndex;
    refToIndex = nullptr;
  }
}

void CreateAndInsertElm(const string& page_id, VNode* node, vector<VNode*>& ref_list,
                        const VNode* ref) {
  auto insert_pos = IndexOf(ref_list, ref);
  int index = std::distance(ref_list.begin(), insert_pos);
  ref_list.insert(insert_pos, node);

  WeexCore::RenderObject* root = VNode2RenderObject(node, page_id);
  RenderManager::GetInstance()->AddRenderObject(
      page_id, node->parent()->ref(), index, root
  );
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
    return -1;//wtf!
  }
  VNode* value = *move_pos;
  ref_list.erase(move_pos);
  auto anchor_pos = IndexOf(ref_list, anchor_ref);
  if (anchor_pos == ref_list.end()) {
#if VRENDER_LOG
    LOGE("[VRenderManager] moveToBackOfRef anchorPos == refList.end() ref: %s",
         anchor_ref->ref().c_str());
#endif
    return -1;//wtf
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
    return -1;//wtf!
  }
  VNode* value = *move_pos;
  ref_list.erase(move_pos);
  auto anchor_pos = IndexOf(ref_list, anchor_ref);
  if (anchor_pos == ref_list.end()) {
#if VRENDER_LOG
    LOGE("[VRenderManager] moveToFrontOfRef anchorPos == refList.end() ref: %s",
         anchor_ref->ref().c_str());
#endif
    return -1;//wtf
  }
  ref_list.insert(anchor_pos, value);
  return index;
}

int MoveElmToFrontOfNode(const string& page_id, vector<VNode*>& ref_list,
                         VNode* move_node, VNode* anchor_node) {
  int move_to_index = MoveToFrontOfRef(ref_list, move_node, anchor_node);
  RenderManager::GetInstance()->MoveRenderObject(
      page_id, move_node->ref(), move_node->parent()->ref(), move_to_index
  );
  return move_to_index;
}

int MoveElmToBackOfNode(const string& page_id, vector<VNode*>& ref_list,
                        VNode* move_node, VNode* anchor_node) {
  int move_to_index = MoveToBackOfRef(ref_list, move_node, anchor_node);
  RenderManager::GetInstance()->MoveRenderObject(
      page_id, move_node->ref(), move_node->parent()->ref(), move_to_index
  );
  return move_to_index;
}

void AddNodes(const string& pageId, vector<VNode*>& vec,
              vector<VNode*>& ref_list, unsigned int start, unsigned int end) {
  for (int i = start; i <= end; ++i) {
    auto p_node = vec[i];
    ref_list.insert(ref_list.begin() + i, p_node);

    WeexCore::RenderObject* root = VNode2RenderObject(p_node, pageId);
    RenderManager::GetInstance()->AddRenderObject(
        pageId, p_node->parent()->ref(), i, root
    );
  }
}

void RemoveNodes(const string& pageId, vector<VNode*>& vec,
                 vector<VNode*>& ref_list, unsigned int start, unsigned int end) {
  for (int i = start; i <= end; ++i) {
    auto p_node = vec[start];
    //some might already been used for patch, which is null.
    if (p_node == nullptr) {
      continue;
    }
    auto pos = IndexOf(ref_list, p_node);
    if (pos == ref_list.end()) {
      //wtf???
#if VRENDER_LOG
      LOGE("[VRenderManager] removeNodes pos == refList.end() ref: %s", p_node->ref().c_str());
#endif
      continue;
    }

    RenderManager::GetInstance()->RemoveRenderObject(pageId, (*pos)->ref());
  }
}

vector<pair<string, string>>* compareMap(const map<string, string>& oldMap,
                                         const map<string, string>& newMap) {
  auto p_vec = new vector<pair<string, string>>();
  for (auto it = newMap.cbegin(); it != newMap.cend(); it++) {
    auto pos = oldMap.find(it->first);

    if (pos == oldMap.end() || pos->second != it->second) {
      //key not exist, or value not same
      p_vec->push_back({pos->first, pos->second});
    }
  }

  for (auto it = oldMap.cbegin(); it != oldMap.cend(); it++) {
    auto pos = newMap.find(it->first);

    if (pos == newMap.end()) {
      //key not exist, remove //todo check if this is correct
      p_vec->push_back({pos->first, ""});
    }
  }
  return p_vec;
};

void PatchVNode(const string& page_id, VNode* old_node, VNode* new_node) {
  //compare attr
  auto p_vec = compareMap(*(old_node->attributes()), *(new_node->attributes()));
  RenderManager::GetInstance()->UpdateAttr(
      page_id, new_node->ref(), p_vec
  );

  //compare style
  p_vec = compareMap(*(old_node->styles()), *(new_node->styles()));
  RenderManager::GetInstance()->UpdateAttr(
      page_id, new_node->ref(), p_vec
  );

  //compare event
  //todo

  //compare children
  if (old_node->HasChildren() && new_node->HasChildren()) {
    UpdateChildren(page_id, old_node, new_node);
  } else if (old_node->HasChildren() && !new_node->HasChildren()) {
    for (auto i = old_node->child_list()->cbegin(); i != old_node->child_list()->cend(); i++) {
      RenderManager::GetInstance()->RemoveRenderObject(page_id, (*i)->ref());
    }
  } else if (!old_node->HasChildren() && new_node->HasChildren()) {
    int index = 0;
    for (auto it = new_node->child_list()->cbegin(); it != new_node->child_list()->cend(); it++) {
      WeexCore::RenderObject* root = VNode2RenderObject(*it, page_id);
      RenderManager::GetInstance()->AddRenderObject(
          page_id, (*it)->parent()->ref(), index, root
      );
      ++index;
    }
  }
}

void patch(const string& page_id, VNode* old_root, VNode* new_root) {
  //root must be the same;
  PatchVNode(page_id, old_root, new_root);
}
}  // namespace data_render
}  // namespace core
}  // namespace weex
