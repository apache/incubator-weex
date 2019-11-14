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

#include <math.h>
#include "base/log_defines.h"
#include "base/time_utils.h"
#include "core/common/view_utils.h"
#include "core/config/core_environment.h"
#include "core/css/constants_value.h"
#include "core/layout/layout.h"
#include "core/manager/weex_core_manager.h"
#include "core/moniter/render_performance.h"
#include "core/render/page/render_page.h"
#include "core/render/action/render_action_add_element.h"
#include "core/render/action/render_action_add_child_to_richtext.h"
#include "core/render/action/render_action_add_event.h"
#include "core/render/action/render_action_appendtree_createfinish.h"
#include "core/render/action/render_action_createbody.h"
#include "core/render/action/render_action_createfinish.h"
#include "core/render/action/render_action_layout.h"
#include "core/render/action/render_action_move_element.h"
#include "core/render/action/render_action_remove_element.h"
#include "core/render/action/render_action_remove_child_from_richtext.h"
#include "core/render/action/render_action_remove_event.h"
#include "core/render/action/render_action_render_success.h"
#include "core/render/action/render_action_update_attr.h"
#include "core/render/action/render_action_update_richtext_child_attr.h"
#include "core/render/action/render_action_update_style.h"
#include "core/render/action/render_action_update_richtext_child_style.h"
#include "core/render/action/render_action_trigger_vsync.h"
#include "core/render/manager/render_manager.h"
#include "core/render/node/factory/render_type.h"
#include "core/render/node/render_list.h"
#include "core/render/node/render_object.h"

namespace WeexCore {

RenderPage::RenderPage(const std::string &page_id)
    : RenderPageBase(page_id, "platform"),
    viewport_width_(0),
      render_root_(nullptr),
      render_page_size_(),
      render_object_registers_() {
#if RENDER_LOG
  LOGD("[RenderPage] new RenderPage >>>> pageId: %s", pageId.c_str());
#endif

  this->render_page_size_.first =
      WXCoreEnvironment::getInstance()->DeviceWidth();
  this->render_page_size_.second = NAN;
  this->viewport_width_ = kDefaultViewPortWidth;
  this->device_width_ = WXCoreEnvironment::getInstance()->DeviceWidth();
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

  set_is_dirty(true);
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

  if (WeexCore::WXCoreEnvironment::getInstance()->isInteractionLogOpen()) {
    LOGD("wxInteractionAnalyzer: [weexcore][addElementStart],%s,%s,%s",
         this->page_id().c_str(),child->type().c_str(),child->ref().c_str());
  }

  // add child to Render Tree
  insert_posiotn = parent->AddRenderObject(insert_posiotn, child);
  if (insert_posiotn < -1) {
    return false;
  }
    
  set_is_dirty(true);

  PushRenderToRegisterMap(child);
  SendAddElementAction(child, parent, insert_posiotn, false);

  Batch();
  if (WeexCore::WXCoreEnvironment::getInstance()->isInteractionLogOpen()) {
    LOGD("wxInteractionAnalyzer: [weexcore][addElementEnd],%s,%s,%s",
         this->page_id().c_str(),child->type().c_str(),child->ref().c_str());
  }
  return true;
}

bool RenderPage::RemoveRenderObject(const std::string &ref) {
  RenderObject *child = GetRenderObject(ref);
  if (child == nullptr) return false;

  RenderObject *parent = child->parent_render();
  if (parent == nullptr) return false;

  set_is_dirty(true);
  parent->RemoveRenderObject(child);

  RemoveRenderFromRegisterMap(child);
  RenderObject* richtext = child->RichtextParent();
  if (richtext) {
      RenderObject* parent = child->parent_render();
      SendRemoveChildFromRichtextAction(ref, parent->type() == "richtext" ? nullptr : parent, richtext);
      richtext->markDirty();
      Batch();
  } else {
      SendRemoveElementAction(ref);
  }
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

  if(index > new_parent->getChildCount()){
    std::stringstream msg;
    msg << "Out of array bounds when RenderPage::MoveRenderObject, specified index: "
    << index << "array size " << new_parent->getChildCount();

    WeexCore::WeexCoreManager::Instance()->getPlatformBridge()->platform_side()
    ->ReportException(page_id().c_str(), "RenderPage::MoveRenderObject", msg.str().c_str());
    return false;
  }

  set_is_dirty(true);
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
    
  set_is_dirty(true);

  std::vector<std::pair<std::string, std::string>> *style = nullptr;
  std::vector<std::pair<std::string, std::string>> *margin = nullptr;
  std::vector<std::pair<std::string, std::string>> *padding = nullptr;
  std::vector<std::pair<std::string, std::string>> *border = nullptr;
  bool inheriableLayout = false;
    
  bool flag = false;
  RenderObject* richtext = render->RichtextParent();

  if (richtext) {
      richtext->markDirty();
      style = new std::vector<std::pair<std::string, std::string>>();
      for (auto it : *src) {
          style->push_back(it);
      }
      flag = true;
      RenderObject* parent = render->parent_render();
      SendUpdateRichtextChildStyleAction(render, style, parent->type() == "richtext" ? nullptr : parent, richtext);
  } else {
      int result = WeexCoreManager::Instance()
        ->getPlatformBridge()
        ->platform_side()
        ->HasTransitionPros(this->page_id_.c_str(), ref.c_str(), src);

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
              case kTypeInheritableLayout:
                  inheriableLayout = true;
                  break;
        default: break;
          }
        }
      }
      if (reserve_css_styles_ || render == render_root_) {
          // If a page requires that all raw css styles saved, we merge to RenderObject's styles map
          render->MergeStyles(src);
      }

     if (style != nullptr || margin != nullptr || padding != nullptr ||
         border != nullptr || inheriableLayout) {
         SendUpdateStyleAction(render, style, margin, padding, border);
     }
  }
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

  RenderObject* richtext = render->RichtextParent();
  if (richtext) {
      RenderObject* parent = render->parent_render();
      SendUpdateRichtextChildAttrAction(render, attrs, parent->type() == "richtext" ? nullptr : parent, richtext);
      richtext->markDirty();
  } else {
      set_is_dirty(true);
      SendUpdateAttrAction(render, attrs);
      for (auto iter = attrs->cbegin(); iter != attrs->cend(); iter++) {
          render->UpdateAttr((*iter).first, (*iter).second);
      }
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

  set_is_dirty(true);
  render->AddEvent(event);

  RenderAction *action = new RenderActionAddEvent(this->page_id_, ref, event);
  PostRenderAction(action);
  return true;
}

bool RenderPage::RemoveEvent(const std::string &ref, const std::string &event) {
  RenderObject *render = GetRenderObject(ref);
  if (render == nullptr) return false;

  set_is_dirty(true);
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

  set_is_dirty(true);
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

  for (auto it : render->shadow_objects_) {
      PushRenderToRegisterMap(it);
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
    RenderObject* richtext = child->RichtextParent();
    if (!richtext) {
        RenderAction *action =
        new RenderActionAddElement(page_id(), child, parent, index, will_layout);
        PostRenderAction(action);
    } else {
        SendAddChildToRichtextAction(child, parent->type() == "richtext" ? nullptr : parent, richtext);
        richtext->markDirty();
        return;
    }
    if (child->type() == "richtext") {
        for (auto it : child->get_shadow_objects()) {
            if (it) {
                SendAddChildToRichtextAction(it, nullptr, child);
            }
        }
        child->markDirty();
        return;
  }

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

void RenderPage::SendAddChildToRichtextAction(RenderObject *child, RenderObject *parent, RenderObject *richtext) {
    RenderAction *action =   new RenderActionAddChildToRichtext(page_id(), child, parent, richtext);
    PostRenderAction(action);

    for (auto it : child->get_child_list()) {
        RenderObject *grandson = static_cast<RenderObject *>(it);
        if (grandson) {
            SendAddChildToRichtextAction(grandson, child, richtext);
        }
    }
}

void RenderPage::SendRemoveChildFromRichtextAction(const std::string &ref, RenderObject *parent, RenderObject *richtext) {
    RenderAction *action = new RenderActionRemoveChildFromRichtext(page_id(), ref, parent, richtext);
    PostRenderAction(action);
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

void RenderPage::SendUpdateRichtextChildStyleAction(RenderObject *render, std::vector<std::pair<std::string, std::string>> *style, RenderObject *parent, RenderObject *richtext) {
    RenderAction *action = new RenderActionUpdateRichtextChildStyle(
                                                       page_id(), render->ref(), style, parent, richtext);
    PostRenderAction(action);

}

void RenderPage::SendUpdateAttrAction(
    RenderObject *render,
    std::vector<std::pair<std::string, std::string>> *attrs) {
  RenderAction *action =
      new RenderActionUpdateAttr(page_id(), render->ref(), attrs);
  PostRenderAction(action);
}

void RenderPage::SendUpdateRichtextChildAttrAction(
                                      RenderObject *render,
                                      std::vector<std::pair<std::string, std::string>> *attrs, RenderObject *parent, RenderObject *richtext) {
    RenderAction *action =
    new RenderActionUpdateRichtextChildAttr(page_id(), render->ref(), attrs, parent, richtext);
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

void RenderPage::SendAppendTreeCreateFinish(const std::string &ref) {
  RenderAction *action = new RenderActionAppendTreeCreateFinish(page_id(), ref);
  PostRenderAction(action);
}

void RenderPage::Batch() {
  if ((kUseVSync && this->need_layout_.load()) || !kUseVSync) {
    LayoutInner();
  }
#if OS_IOS
  // vsync may stopped, trigger once
  RenderAction *action = new RenderActionTriggerVSync(page_id());
  PostRenderAction(action);
#endif
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

bool RenderPage::ReapplyStyles() {
  if (!reserve_css_styles_) {
    LOGE("CSS styles of page %s are dropped, unable to do reapply styles actions.", page_id().c_str());
    return false;
  }
  
  for (auto it = render_object_registers_.begin(); it != render_object_registers_.end(); ++ it) {
    if (it->second == nullptr) {
      continue;
    }
    
    auto stylesMap = it->second->styles_;
    if (stylesMap != nullptr) {
      std::vector<std::pair<std::string, std::string>> *style = nullptr;
      std::vector<std::pair<std::string, std::string>> *margin = nullptr;
      std::vector<std::pair<std::string, std::string>> *padding = nullptr;
      std::vector<std::pair<std::string, std::string>> *border = nullptr;
      bool inheriableLayout = false;
      
      for (auto sit = stylesMap->begin(); sit != stylesMap->end(); ++ sit) {
        switch (it->second->UpdateStyle(sit->first, sit->second)) {
          case kTypeStyle:
            if (style == nullptr) {
              style = new std::vector<std::pair<std::string, std::string>>();
            }
            style->insert(style->end(), std::make_pair(sit->first, sit->second));
            break;
          case kTypeMargin:
            if (margin == nullptr) {
              margin = new std::vector<std::pair<std::string, std::string>>();
            }
            it->second->UpdateStyleInternal(
                                        sit->first, sit->second, 0, [=](float foo) {
                                          margin->insert(margin->end(), std::make_pair(sit->first, to_string(foo)));
                                        });
            break;
          case kTypePadding:
            if (padding == nullptr) {
              padding = new std::vector<std::pair<std::string, std::string>>();
            }
            it->second->UpdateStyleInternal(
                                        sit->first, sit->second, 0, [=](float foo) {
                                          padding->insert(padding->end(), std::make_pair(sit->first, to_string(foo)));
                                        });
            break;
          case kTypeBorder:
            if (border == nullptr) {
              border = new std::vector<std::pair<std::string, std::string>>();
            }
            it->second->UpdateStyleInternal(
                                        sit->first, sit->second, 0, [=](float foo) {
                                          border->insert(border->end(), std::make_pair(sit->first, to_string(foo)));
                                        });
            break;
          case kTypeInheritableLayout:
            inheriableLayout = true;
            break;
          default: break;
        }
      }
      
      if (style != nullptr || margin != nullptr || padding != nullptr ||
          border != nullptr || inheriableLayout) {
        SendUpdateStyleAction(it->second, style, margin, padding, border);
      }
      
      if (style != nullptr) {
        delete style;
      }
      
      if (margin != nullptr) {
        delete margin;
      }
      
      if (padding != nullptr) {
        delete padding;
      }
      
      if (border != nullptr) {
        delete border;
      }
    }
  }
  
  Batch();
  return true;
}
  
}  // namespace WeexCore
