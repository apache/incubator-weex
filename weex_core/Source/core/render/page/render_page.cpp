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

#include "core/render/page/render_page.h"
#include "base/TimeUtils.h"
#include "base/ViewUtils.h"
#include "base/LogDefines.h"
#include "core/config/core_environment.h"
#include "core/css/constants_value.h"
#include "core/layout/layout.h"
#include "core/manager/weex_core_manager.h"
#include "core/moniter/render_performance.h"
#include "core/render/action/render_action_add_element.h"
#include "core/render/action/render_action_add_event.h"
#include "core/render/action/render_action_appendtree_createfinish.h"
#include "core/render/action/render_action_createbody.h"
#include "core/render/action/render_action_createfinish.h"
#include "core/render/action/render_action_layout.h"
#include "core/render/action/render_action_move_element.h"
#include "core/render/action/render_action_remove_element.h"
#include "core/render/action/render_action_remove_event.h"
#include "core/render/action/render_action_render_success.h"
#include "core/render/action/render_action_update_attr.h"
#include "core/render/action/render_action_update_style.h"
#include "core/render/action/render_action_trigger_vsync.h"
#include "core/render/manager/render_manager.h"
#include "core/render/node/factory/render_type.h"
#include "core/render/node/render_list.h"
#include "core/render/node/render_object.h"

namespace WeexCore {

RenderPage::RenderPage(const std::string &page_id)
    : viewport_width_(0),
      render_root_(nullptr),
      page_id_(),
      render_page_size_(),
      render_object_registers_(),
      render_performance_(nullptr) {
#if RENDER_LOG
  LOGD("[RenderPage] new RenderPage >>>> pageId: %s", pageId.c_str());
#endif

  this->page_id_ = page_id;
  this->render_performance_ = new RenderPerformance();
  this->render_page_size_.first =
      WXCoreEnvironment::getInstance()->DeviceWidth();
  this->render_page_size_.second = NAN;
  this->viewport_width_ = kDefaultViewPortWidth;
}

RenderPage::~RenderPage() {
  //#if RENDER_LOG
  LOGE("[RenderPage] Delete RenderPage >>>> pageId: %s", page_id().c_str());
  //#endif

  this->render_object_registers_.clear();

  if (this->render_root_ != nullptr) {
    delete this->render_root_;
    this->render_root_ = nullptr;
  }

  if (this->render_performance_ != nullptr) {
    delete this->render_performance_;
    this->render_performance_ = nullptr;
  }
}

void RenderPage::CalculateLayout() {
  if (this->render_root_ == nullptr || !this->render_root_->ViewInit()) return;

#if RENDER_LOG
  LOGD("[RenderPage] CalculateLayout >>>> pageId: %s", mPageId.c_str());
#endif

  int64_t start_time = getCurrentTime();
  if (is_before_layout_needed_.load()) {
    this->render_root_->LayoutBeforeImpl();
  }
  this->render_root_->calculateLayout(this->render_page_size_);
  if (is_platform_layout_needed_.load()) {
    this->render_root_->LayoutPlatformImpl();
  }
  if (is_after_layout_needed_.load()) {
    this->render_root_->LayoutAfterImpl();
  }
  CssLayoutTime(getCurrentTime() - start_time);
  TraverseTree(this->render_root_, 0);
}

void RenderPage::TraverseTree(RenderObject *render, long index) {
  if (render == nullptr) return;

  if (render->hasNewLayout()) {
    SendLayoutAction(render, (int)index);
    render->setHasNewLayout(false);
  }

  for (auto it = render->ChildListIterBegin(); it != render->ChildListIterEnd();
       it++) {
    RenderObject *child = static_cast<RenderObject *>(*it);
    if (child != nullptr) {
      TraverseTree(child, it - render->ChildListIterBegin());
    }
  }
}

bool RenderPage::CreateRootRender(RenderObject *root) {
  if (root == nullptr) return false;

  SetRootRenderObject(root);

  if (isnan(this->render_root_->getStyleWidth())) {
    this->render_root_->setStyleWidthLevel(FALLBACK_STYLE);
    if (is_render_container_width_wrap_content())
      this->render_root_->setStyleWidthToNAN();
    else
      this->render_root_->setStyleWidth(
          WXCoreEnvironment::getInstance()->DeviceWidth(), false);
  } else {
    this->render_root_->setStyleWidthLevel(CSS_STYLE);
  }
  PushRenderToRegisterMap(root);

  SendCreateBodyAction(root);
  return true;
}

void RenderPage::SetRootRenderObject(RenderObject *root) {
  if (root != nullptr) {
    this->render_root_ = root;
    this->render_root_->set_is_root_render();
  }
}

bool RenderPage::AddRenderObject(const std::string &parent_ref,
                                 int insert_posiotn, RenderObject *child) {
  RenderObject *parent = GetRenderObject(parent_ref);
  if (parent == nullptr || child == nullptr) {
    return false;
  }

  // add child to Render Tree
  insert_posiotn = parent->AddRenderObject(insert_posiotn, child);
  if (insert_posiotn < -1) {
    return false;
  }

  PushRenderToRegisterMap(child);
  SendAddElementAction(child, parent, insert_posiotn, false);

  Batch();
  return true;
}

bool RenderPage::RemoveRenderObject(const std::string &ref) {
  RenderObject *child = GetRenderObject(ref);
  if (child == nullptr) return false;

  RenderObject *parent = child->parent_render();
  if (parent == nullptr) return false;

  parent->RemoveRenderObject(child);

  RemoveRenderFromRegisterMap(child);
  SendRemoveElementAction(ref);
    
  delete child;
  return true;
}

bool RenderPage::MoveRenderObject(const std::string &ref,
                                  const std::string &parent_ref, int index) {
  RenderObject *child = GetRenderObject(ref);
  if (child == nullptr) return false;

  RenderObject *old_parent = child->parent_render();
  RenderObject *new_parent = GetRenderObject(parent_ref);
  if (old_parent == nullptr || new_parent == nullptr) return false;

  if (old_parent->ref() == new_parent->ref()) {
    if (old_parent->IndexOf(child) == index) {
      return false;
    } else if (old_parent->IndexOf(child) < index) {
      index = index - 1;
    }
  }

  child->getParent()->removeChild(child);
  new_parent->addChildAt(child, index);

  SendMoveElementAction(ref, parent_ref, index);
  return true;
}

bool RenderPage::UpdateStyle(
    const std::string &ref,
    std::vector<std::pair<std::string, std::string>> *src) {
  RenderObject *render = GetRenderObject(ref);
  if (render == nullptr || src == nullptr || src->empty()) return false;

  std::vector<std::pair<std::string, std::string>> *style = nullptr;
  std::vector<std::pair<std::string, std::string>> *margin = nullptr;
  std::vector<std::pair<std::string, std::string>> *padding = nullptr;
  std::vector<std::pair<std::string, std::string>> *border = nullptr;

  bool flag = false;
  int result =
      WeexCoreManager::Instance()
          ->getPlatformBridge()
          ->platform_side()
          ->HasTransitionPros(this->page_id_.c_str(), ref.c_str(), src);
  // int result =
  // Bridge_Impl_Android::getInstance()->callHasTransitionPros(mPageId.c_str(),
  // ref.c_str(), src);

  if (result == 1) {
    SendUpdateStyleAction(render, src, margin, padding, border);
  } else {
    for (auto iter = src->begin(); iter != src->end(); iter++) {
      switch (render->UpdateStyle((*iter).first, (*iter).second)) {
        case kTypeStyle:
          if (style == nullptr) {
            style = new std::vector<std::pair<std::string, std::string>>();
          }
          style->insert(style->end(), (*iter));
          flag = true;
          break;
        case kTypeMargin:
          if (margin == nullptr) {
            margin = new std::vector<std::pair<std::string, std::string>>();
          }
          render->UpdateStyleInternal(
              (*iter).first, (*iter).second, 0, [=, &flag](float foo) {
                  (*iter).second = to_string(foo);
                  margin->insert(margin->end(), (*iter));
                  flag = true;
              });
          break;
        case kTypePadding:
          if (padding == nullptr) {
            padding = new std::vector<std::pair<std::string, std::string>>();
          }
          render->UpdateStyleInternal(
              (*iter).first, (*iter).second, 0, [=, &flag](float foo) {
                  (*iter).second = to_string(foo);
                  padding->insert(padding->end(), (*iter));
                  flag = true;
              });
          break;
        case kTypeBorder:
          if (border == nullptr) {
            border = new std::vector<std::pair<std::string, std::string>>();
          }
          render->UpdateStyleInternal(
              (*iter).first, (*iter).second, 0, [=, &flag](float foo) {
                  (*iter).second = to_string(foo);
                  border->insert(border->end(), (*iter));
                  flag = true;
              });
          break;
        default: break;
      }
    }
  }

  if (style != nullptr || margin != nullptr || padding != nullptr ||
      border != nullptr)
    SendUpdateStyleAction(render, style, margin, padding, border);

  Batch();

  if (src != nullptr) {
    src->clear();
    src->shrink_to_fit();
    delete src;
    src = nullptr;
  }

  if (style != nullptr) {
    style->clear();
    style->shrink_to_fit();
    delete style;
    style = nullptr;
  }

  if (margin != nullptr) {
    margin->clear();
    margin->shrink_to_fit();
    delete margin;
    margin = nullptr;
  }

  if (padding != nullptr) {
    padding->clear();
    padding->shrink_to_fit();
    delete padding;
    padding = nullptr;
  }

  if (border != nullptr) {
    border->clear();
    border->shrink_to_fit();
    delete border;
    border = nullptr;
  }

  return flag;
}

bool RenderPage::UpdateAttr(
    const std::string &ref,
    std::vector<std::pair<std::string, std::string>> *attrs) {
  RenderObject *render = GetRenderObject(ref);
  if (render == nullptr || attrs == nullptr || attrs->empty()) return false;

  SendUpdateAttrAction(render, attrs);

  for (auto iter = attrs->cbegin(); iter != attrs->cend(); iter++) {
    render->UpdateAttr((*iter).first, (*iter).second);
  }
  Batch();
  if (attrs != nullptr) {
    attrs->clear();
    attrs->shrink_to_fit();
    delete attrs;
    attrs = nullptr;
  }

  return true;
}

void RenderPage::SetDefaultHeightAndWidthIntoRootRender(
    const float default_width, const float default_height,
    const bool is_width_wrap_content, const bool is_height_wrap_content) {
  this->render_page_size_.first = default_width;
  this->render_page_size_.second = default_height;
  if (this->render_root_->getStyleWidthLevel() >= INSTANCE_STYLE) {
    this->render_root_->setStyleWidthLevel(INSTANCE_STYLE);
    if (is_width_wrap_content) {
      set_is_render_container_width_wrap_content(true);
      this->render_root_->setStyleWidthToNAN();
      this->render_page_size_.first = NAN;
    } else {
      this->render_root_->setStyleWidth(default_width, true);
    }
    set_is_dirty(true);
  }

  if (this->render_root_->getStyleHeightLevel() >= INSTANCE_STYLE) {
    if (!is_height_wrap_content) {
      this->render_root_->setStyleHeightLevel(INSTANCE_STYLE);
      this->render_root_->setStyleHeight(default_height);
      set_is_dirty(true);
    }
  }

  Batch();
}

bool RenderPage::AddEvent(const std::string &ref, const std::string &event) {
  RenderObject *render = GetRenderObject(ref);
  if (render == nullptr) return false;

  render->AddEvent(event);

  RenderAction *action = new RenderActionAddEvent(this->page_id_, ref, event);
  PostRenderAction(action);
  return true;
}

bool RenderPage::RemoveEvent(const std::string &ref, const std::string &event) {
  RenderObject *render = GetRenderObject(ref);
  if (render == nullptr) return false;

  render->RemoveEvent(event);

  RenderAction *action =
      new RenderActionRemoveEvent(this->page_id_, ref, event);
  PostRenderAction(action);
  return true;
}

bool RenderPage::CreateFinish() {
  if (this->render_root_ == nullptr) {
    return false;
  }
  Batch();
  SendCreateFinishAction();
  // RenderSuccess means the Dom created after executing script finishes layout
  // and render, it will be trigger even though body not yet attaches to parent.
  LayoutInner();
  SendRenderSuccessAction();
  return true;
}
    
void RenderPage::LayoutInner() {
  CalculateLayout();
  this->need_layout_.store(false);
  set_is_dirty(false);
}

void RenderPage::LayoutImmediately() {
  if (is_dirty() && kUseVSync) {
    LayoutInner();
  }
}

void RenderPage::PostRenderAction(RenderAction *action) {
  if (action != nullptr) {
    action->ExecuteAction();
    delete action;
    action = nullptr;
  }
}

void RenderPage::PushRenderToRegisterMap(RenderObject *render) {
  if (render == nullptr) return;

  std::string ref = render->ref();
  this->render_object_registers_.insert(
      std::pair<std::string, RenderObject *>(ref, render));

  for (auto it = render->ChildListIterBegin(); it != render->ChildListIterEnd();
       it++) {
    RenderObject *child = static_cast<RenderObject *>(*it);
    if (child != nullptr) {
      PushRenderToRegisterMap(child);
    }
  }
}

void RenderPage::RemoveRenderFromRegisterMap(RenderObject *render) {
  if (render == nullptr) return;

  this->render_object_registers_.erase(render->ref());

  for (auto it = render->ChildListIterBegin(); it != render->ChildListIterEnd();
       it++) {
    RenderObject *child = static_cast<RenderObject *>(*it);
    if (child != nullptr) {
      RemoveRenderFromRegisterMap(child);
    }
  }
}

void RenderPage::SendCreateBodyAction(RenderObject *render) {
  if (render == nullptr) return;

  RenderAction *action = new RenderActionCreateBody(page_id(), render);
  PostRenderAction(action);

  int i = 0;
  for (auto it = render->ChildListIterBegin(); it != render->ChildListIterEnd();
       it++) {
    RenderObject *child = static_cast<RenderObject *>(*it);
    if (child != nullptr) {
      SendAddElementAction(child, render, i, true);
    }
    ++i;
  }

  if (i > 0 && render->IsAppendTree()) {
    SendAppendTreeCreateFinish(render->ref());
  }
}

void RenderPage::SendAddElementAction(RenderObject *child, RenderObject *parent,
                                      int index, bool is_recursion,
                                      bool will_layout) {
  if (child == nullptr || parent == nullptr) return;
  if (parent != nullptr && parent->type() == WeexCore::kRenderRecycleList) {
    will_layout = false;
  }

  RenderAction *action =
      new RenderActionAddElement(page_id(), child, parent, index, will_layout);
  PostRenderAction(action);

  int i = 0;
  for (auto it = child->ChildListIterBegin(); it != child->ChildListIterEnd();
       it++) {
    RenderObject *grandson = static_cast<RenderObject *>(*it);
    if (grandson != nullptr) {
      SendAddElementAction(grandson, child, i, true, will_layout);
    }
    ++i;
  }

  if (child->type() == WeexCore::kRenderRecycleList) {
    RenderList *render_list = static_cast<RenderList *>(child);
    std::vector<RenderObject *> &cell_slots = render_list->CellSlots();
    for (auto it = cell_slots.begin(); it != cell_slots.end(); it++) {
      RenderObject *grandson = static_cast<RenderObject *>(*it);
      if (grandson != nullptr) {
        SendAddElementAction(grandson, child, -1, true, will_layout);
      }
      ++i;
    }
  }

  if (!is_recursion && i > 0 && child->IsAppendTree()) {
    SendAppendTreeCreateFinish(child->ref());
  }
}

void RenderPage::SendRemoveElementAction(const std::string &ref) {
  RenderAction *action = new RenderActionRemoveElement(page_id(), ref);
  PostRenderAction(action);
}

void RenderPage::SendMoveElementAction(const std::string &ref,
                                       const std::string &parent_ref,
                                       int index) {
  RenderAction *action =
      new RenderActionMoveElement(page_id(), ref, parent_ref, index);
  PostRenderAction(action);
}

void RenderPage::SendLayoutAction(RenderObject *render, int index) {
  if (render == nullptr) return;

  RenderAction *action = new RenderActionLayout(page_id(), render, index);
  PostRenderAction(action);
}
    
void RenderPage::SendUpdateStyleAction(
    RenderObject *render,
    std::vector<std::pair<std::string, std::string>> *style,
    std::vector<std::pair<std::string, std::string>> *margin,
    std::vector<std::pair<std::string, std::string>> *padding,
    std::vector<std::pair<std::string, std::string>> *border) {
  RenderAction *action = new RenderActionUpdateStyle(
      page_id(), render->ref(), style, margin, padding, border);
  PostRenderAction(action);
}

void RenderPage::SendUpdateAttrAction(
    RenderObject *render,
    std::vector<std::pair<std::string, std::string>> *attrs) {
  RenderAction *action =
      new RenderActionUpdateAttr(page_id(), render->ref(), attrs);
  PostRenderAction(action);
}

void RenderPage::SendUpdateAttrAction(
    RenderObject *render, std::map<std::string, std::string> *attrs) {
  std::vector<std::pair<std::string, std::string>> *vAttrs =
      new std::vector<std::pair<std::string, std::string>>();
  for (auto iter = attrs->cbegin(); iter != attrs->cend(); iter++) {
    vAttrs->insert(vAttrs->begin(), std::pair<std::string, std::string>(
                                        iter->first, iter->second));
  }

  RenderAction *action =
      new RenderActionUpdateAttr(page_id(), render->ref(), vAttrs);
  PostRenderAction(action);

  if (vAttrs != nullptr) {
    vAttrs->clear();
    delete vAttrs;
    vAttrs = nullptr;
  }
}

void RenderPage::SendCreateFinishAction() {
  RenderAction *action = new RenderActionCreateFinish(page_id());
  PostRenderAction(action);
}

void RenderPage::SendRenderSuccessAction() {
  RenderAction *action = new RenderActionRenderSuccess(page_id());
  PostRenderAction(action);
}

void RenderPage::SendAppendTreeCreateFinish(const std::string &ref) {
  RenderAction *action = new RenderActionAppendTreeCreateFinish(page_id(), ref);
  PostRenderAction(action);
}

void RenderPage::CssLayoutTime(const int64_t &time) {
  if (this->render_performance_ != nullptr)
    this->render_performance_->cssLayoutTime += time;
}

void RenderPage::ParseJsonTime(const int64_t &time) {
  if (this->render_performance_ != nullptr)
    this->render_performance_->parseJsonTime += time;
}

void RenderPage::CallBridgeTime(const int64_t &time) {
  if (this->render_performance_ != nullptr)
    this->render_performance_->callBridgeTime += time;
}

std::vector<int64_t> RenderPage::PrintFirstScreenLog() {
  std::vector<int64_t> ret;
  if (this->render_performance_ != nullptr)
    ret = this->render_performance_->PrintPerformanceLog(onFirstScreen);
  return ret;
}

std::vector<int64_t> RenderPage::PrintRenderSuccessLog() {
  std::vector<int64_t> ret;
  if (this->render_performance_ != nullptr)
    ret = this->render_performance_->PrintPerformanceLog(onRenderSuccess);
  return ret;
}

void RenderPage::Batch() {
  if ((kUseVSync && this->need_layout_.load()) || !kUseVSync) {
    LayoutInner();
  }
  else {
#if OS_IOS
    // vsync may stopped, trigger once
    RenderAction *action = new RenderActionTriggerVSync(page_id());
    PostRenderAction(action);
#endif
  }
}

RenderObject *RenderPage::GetRenderObject(const std::string &ref) {
  std::map<std::string, RenderObject *>::iterator iter =
      this->render_object_registers_.find(ref);
  if (iter != this->render_object_registers_.end()) {
    return iter->second;
  } else {
    return nullptr;
  }
}

void RenderPage::OnRenderPageInit() {}

void RenderPage::OnRenderProcessStart() {}

void RenderPage::OnRenderProcessExited() {}

void RenderPage::OnRenderProcessGone() {}

void RenderPage::OnRenderPageClose() {}
}  // namespace WeexCore
