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
#include <core/render/action/render_action_add_element.h>
#include <core/render/action/render_action_remove_element.h>
#include <core/render/action/render_action_move_element.h>
#include <core/render/action/render_action_createbody.h>
#include <core/render/action/render_action_update_style.h>
#include <core/render/action/render_action_update_attr.h>
#include <core/render/action/render_action_layout.h>
#include <core/render/action/render_action_createfinish.h>
#include <core/render/action/render_action_appendtree_createfinish.h>
#include <core/layout/layout.h>
#include <core/moniter/render_performance.h>
#include <core/config/core_environment.h>
#include <base/ViewUtils.h>
#include <core/render/action/render_action_add_event.h>
#include <core/render/action/render_action_remove_event.h>
#include <core/css/constants_value.h>
#include <core/render/node/factory/render_type.h>
#include <core/render/node/render_list.h>
#include <core/manager/weex_core_manager.h>
#include "render_page.h"
#include "core/render/manager/render_manager.h"
#include "core/render/node/render_object.h"

namespace WeexCore {

  static bool splitScreenRendering = false;

  RenderPage::RenderPage(std::string pageId) {

#if RENDER_LOG
    LOGD("[RenderPage] new RenderPage >>>> pageId: %s", pageId.c_str());
#endif

    mPageId = pageId;
    mWXCorePerformance = new RenderPerformance();
    mViewPortWidth = kDefaultViewPortWidth;
    renderPageSize.first = WXCoreEnvironment::getInstance()->DeviceWidth();
    renderPageSize.second = NAN;
  }

  RenderPage::~RenderPage() {

#if RENDER_LOG
    LOGD("[RenderPage] Delete RenderPage >>>> pageId: %s", mPageId.c_str());
#endif

    mRenderObjectRegisterMap.clear();

    if (render_root != nullptr) {
      delete render_root;
      render_root = nullptr;
    }

    if (mWXCorePerformance != nullptr) {
      delete mWXCorePerformance;
      mWXCorePerformance = nullptr;
    }
  }

  void RenderPage::CalculateLayout() {
    if (render_root == nullptr || !render_root->ViewInit())
      return;

#if RENDER_LOG
    LOGD("[RenderPage] CalculateLayout >>>> pageId: %s", mPageId.c_str());
#endif

    long long startTime = getCurrentTime();
    render_root->LayoutBefore();
    render_root->calculateLayout(renderPageSize);
    render_root->LayoutAfter();
    CssLayoutTime(getCurrentTime() - startTime);

    if (splitScreenRendering) {
      if (mAlreadyCreateFinish) {
        TraverseTree(render_root, 0);
      } else {
        float deviceHeight = WXCoreEnvironment::getInstance()->DeviceHeight();
        float deviceWidth = WXCoreEnvironment::getInstance()->DeviceWidth();
        float radio = deviceWidth / (mViewPortWidth * kLayoutFirstScreenOverflowRadio);

        switch (render_root->getFlexDirection()) {
          case kFlexDirectionColumn:
          case kFlexDirectionColumnReverse:
            if (render_root->getLargestMainSize() * radio > deviceHeight / 3) {
              TraverseTree(render_root, 0);
            }
            break;
          case kFlexDirectionRow:
          case kFlexDirectionRowReverse:
          default:
            if (render_root->getLargestMainSize() * radio > deviceWidth / 3) {
              TraverseTree(render_root, 0);
            }
            break;
        }
      }
    } else {
      TraverseTree(render_root, 0);
    }
  }

  void RenderPage::TraverseTree(RenderObject *render,int index) {

    if (render == nullptr)
      return;

    if (render->hasNewLayout()) {
      SendLayoutAction(render, index);
      render->setHasNewLayout(false);
    }

    for(auto it = render->ChildListIterBegin(); it != render->ChildListIterEnd(); it++) {
      RenderObject* child = static_cast<RenderObject*>(*it);
      if (child != nullptr) {
        TraverseTree(child, it-render->ChildListIterBegin());
      }
    }
  }

  bool RenderPage::CreateRootRender(RenderObject *root) {

    if (root == nullptr)
      return false;

    SetRootRenderObject(root);

    if (isnan(render_root->getStyleWidth())) {
      render_root->setStyleWidthLevel(FALLBACK_STYLE);
      if (GetRenderContainerWidthWrapContent())
        render_root->setStyleWidthToNAN();
      else
        render_root->setStyleWidth(WXCoreEnvironment::getInstance()->DeviceWidth(), false);
    } else {
      render_root->setStyleWidthLevel(CSS_STYLE);
    }
    PushRenderToRegisterMap(root);

    SendCreateBodyAction(root);
    return true;
  }

  void RenderPage::SetRootRenderObject(RenderObject *root) {
    if (root != nullptr) {
      render_root = root;
      render_root->MarkRootRender();
    }
  }

  bool RenderPage::AddRenderObject(const std::string &parentRef, int insertPosition, RenderObject *child) {
    RenderObject *parent = GetRenderObject(parentRef);
    if (parent == nullptr || child == nullptr) {
      return false;
    }

    // add child to Render Tree
    insertPosition = parent->AddRenderObject(insertPosition, child);
    if (insertPosition < -1) {
      return false;
    }

    PushRenderToRegisterMap(child);
    SendAddElementAction(child, parent, insertPosition, false);

    Batch();
    return true;
  }

  bool RenderPage::RemoveRenderObject(const std::string &ref) {
    RenderObject *child = GetRenderObject(ref);
    if (child == nullptr)
      return false;

    RenderObject *parent = child->GetParentRender();
    if (parent == nullptr)
      return false;

    parent->RemoveRenderObject(child);

    RemoveRenderFromRegisterMap(child);
    delete child;

    SendRemoveElementAction(ref);
    return true;
  }

  bool RenderPage::MoveRenderObject(const std::string &ref, const std::string &parentRef, int index) {
    RenderObject *child = GetRenderObject(ref);
    if (child == nullptr)
      return false;

    RenderObject *oldParent = child->GetParentRender();
    RenderObject *newParent = GetRenderObject(parentRef);
    if (oldParent == nullptr || newParent == nullptr)
      return false;

    if (oldParent->Ref() == newParent->Ref()) {
      if (oldParent->IndexOf(child) < 0) {
        return false;
      } else if (oldParent->IndexOf(child) == index) {
        return false;
      } else if (oldParent->IndexOf(child) < index) {
        index = index - 1;
      }
    }

    child->getParent()->removeChild(child);
    newParent->addChildAt(child, index);

    SendMoveElementAction(ref, parentRef, index);
    return true;
  }

  bool RenderPage::UpdateStyle(const std::string &ref, std::vector<std::pair<std::string, std::string>> *src) {
    RenderObject *render = GetRenderObject(ref);
    if (render == nullptr || src == nullptr || src->empty())
      return false;

    std::vector<std::pair<std::string, std::string>> *style = nullptr;
    std::vector<std::pair<std::string, std::string>> *margin = nullptr;
    std::vector<std::pair<std::string, std::string>> *padding = nullptr;
    std::vector<std::pair<std::string, std::string>> *border = nullptr;

    bool flag = false;
    int result = WeexCoreManager::getInstance()->getPlatformBridge()->callHasTransitionPros(mPageId.c_str(), ref.c_str(), src);
    //int result = Bridge_Impl_Android::getInstance()->callHasTransitionPros(mPageId.c_str(), ref.c_str(), src);

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
            render->UpdateStyle((*iter).first,
                                (*iter).second,
                                0,
                                [=, &flag](float foo) {
                                  (*iter).second = to_string(foo),
                                      margin->insert(margin->end(), (*iter)),
                                  flag = true;
                                });
            break;
          case kTypePadding:
            if (padding == nullptr) {
              padding = new std::vector<std::pair<std::string, std::string>>();
            }
            render->UpdateStyle((*iter).first,
                                (*iter).second,
                                0,
                                [=, &flag](float foo) {
                                  (*iter).second = to_string(foo),
                                      padding->insert(padding->end(), (*iter)),
                                  flag = true;
                                });
            break;
          case kTypeBorder:
            if (border == nullptr) {
              border = new std::vector<std::pair<std::string, std::string>>();
            }
            render->UpdateStyle((*iter).first,
                                (*iter).second,
                                0,
                                [=, &flag](float foo) {
                                  (*iter).second = to_string(foo),
                                      border->insert(border->end(), (*iter)),
                                  flag = true;
                                });
            break;
        }
      }
    }

    if (style != nullptr || margin != nullptr || padding != nullptr || border != nullptr)
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

  bool RenderPage::UpdateAttr(const std::string &ref, std::vector<std::pair<std::string, std::string>> *attrs) {
    RenderObject *render = GetRenderObject(ref);
    if (render == nullptr || attrs == nullptr || attrs->empty())
      return false;

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

  void RenderPage::SetDefaultHeightAndWidthIntoRootRender(const float defaultWidth,
                                                          const float defaultHeight,
                                                          const bool isWidthWrapContent, const bool isHeightWrapContent) {
    renderPageSize.first = defaultWidth;
    renderPageSize.second = defaultHeight;
    if (render_root->getStyleWidthLevel() >= INSTANCE_STYLE) {
      render_root->setStyleWidthLevel(INSTANCE_STYLE);
      if (isWidthWrapContent) {
        SetRenderContainerWidthWrapContent(true);
        render_root->setStyleWidthToNAN();
        renderPageSize.first = NAN;
      } else {
        render_root->setStyleWidth(defaultWidth, true);
      }
      updateDirty(true);
    }

    if (render_root->getStyleHeightLevel() >= INSTANCE_STYLE) {
      if(!isHeightWrapContent) {
        render_root->setStyleHeightLevel(INSTANCE_STYLE);
        render_root->setStyleHeight(defaultHeight);
        updateDirty(true);
      }
    }

    Batch();
  }

  bool RenderPage::AddEvent(const std::string &ref, const std::string &event) {
    RenderObject *render = GetRenderObject(ref);
    if (render == nullptr)
      return false;

    render->AddEvent(event);

    render_action *action = new RenderActionAddEvent(mPageId, ref, event);
    PostRenderAction(action);
    return true;
  }

  bool RenderPage::RemoveEvent(const std::string &ref, const std::string &event) {
    RenderObject *render = GetRenderObject(ref);
    if (render == nullptr)
      return false;

    render->RemoveEvent(event);

    render_action *action = new RenderActionRemoveEvent(mPageId, ref, event);
    PostRenderAction(action);
    return true;
  }

  bool RenderPage::CreateFinish() {
    if (render_root == nullptr) {
      return false;
    }
    mAlreadyCreateFinish = true;
    Batch();
    SendCreateFinishAction();
    return true;
  }

  void RenderPage::LayoutImmediately() {
    if(isDirty() && useVSync){
      CalculateLayout();
      needLayout.store(false);
      updateDirty(false);
    }
  }

  void RenderPage::PostRenderAction(render_action *action) {
    if (action != nullptr) {
      action->ExecuteAction();
    }
  }

  void RenderPage::PushRenderToRegisterMap(RenderObject *render) {
    if (render == nullptr)
      return;

    std::string ref = render->Ref();
    mRenderObjectRegisterMap.insert(std::pair<std::string, RenderObject *>(ref, render));

    for(auto it = render->ChildListIterBegin(); it != render->ChildListIterEnd(); it++) {
      RenderObject* child = static_cast<RenderObject*>(*it);
      if (child != nullptr) {
        PushRenderToRegisterMap(child);
      }
    }
  }

  void RenderPage::RemoveRenderFromRegisterMap(RenderObject *render) {
    if (render == nullptr)
      return;

    mRenderObjectRegisterMap.erase(render->Ref());

    for(auto it = render->ChildListIterBegin(); it != render->ChildListIterEnd(); it++) {
      RenderObject* child = static_cast<RenderObject*>(*it);
      if (child != nullptr) {
        RemoveRenderFromRegisterMap(child);
      }
    }
  }

  void RenderPage::SendCreateBodyAction(RenderObject *render) {
    if (render == nullptr)
      return;

    render_action *action = new RenderActionCreateBody(PageId(), render);
    PostRenderAction(action);

    Index i = 0;
    for(auto it = render->ChildListIterBegin(); it != render->ChildListIterEnd(); it++) {
      RenderObject* child = static_cast<RenderObject*>(*it);
      if (child != nullptr) {
        SendAddElementAction(child, render, i, true);
      }
      ++i;
    }

    if (i > 0 && render->IsAppendTree()) {
      SendAppendTreeCreateFinish(render->Ref());
    }
  }

  void RenderPage::SendAddElementAction(RenderObject *child, RenderObject *parent, int index, bool is_recursion, bool willLayout) {
    if (child == nullptr || parent == nullptr)
      return;
    if(parent != nullptr && parent->Type() == WeexCore::kRenderRecycleList){
        willLayout = false;
    }

    render_action *action = new RenderActionAddElement(PageId(), child, parent, index, willLayout);
    PostRenderAction(action);

    Index i = 0;
    for(auto it = child->ChildListIterBegin(); it != child->ChildListIterEnd(); it++) {
      RenderObject* grandson = static_cast<RenderObject*>(*it);
      if (grandson != nullptr) {
        SendAddElementAction(grandson, child, i, true, willLayout);
      }
      ++i;
    }

    if(child->Type() == WeexCore::kRenderRecycleList){
      RenderList* renderList = (RenderList*)child;
      std::vector<RenderObject*>& cellSlots = renderList->CellSlots();
      for(auto it = cellSlots.begin(); it != cellSlots.end(); it++) {
        RenderObject* grandson = static_cast<RenderObject*>(*it);
        if (grandson != nullptr) {
          SendAddElementAction(grandson, child, -1, true, willLayout);
        }
        ++i;
      }
    }

    if (!is_recursion && i > 0 && child->IsAppendTree()) {
      SendAppendTreeCreateFinish(child->Ref());
    }
  }

  void RenderPage::SendRemoveElementAction(const std::string &ref) {
    render_action *action = new RenderActionRemoveElement(PageId(), ref);
    PostRenderAction(action);
  }

  void RenderPage::SendMoveElementAction(const std::string &ref, const std::string &parentRef, int index) {
    render_action *action = new RenderActionMoveElement(PageId(), ref, parentRef, index);
    PostRenderAction(action);
  }

  void RenderPage::SendLayoutAction(RenderObject *render, int index) {
    if (render == nullptr)
      return;

    render_action *action = new RenderActionLayout(PageId(), render, index);
    PostRenderAction(action);
  }

  void RenderPage::SendUpdateStyleAction(RenderObject *render,
                                         std::vector<std::pair<std::string, std::string>> *style,
                                         std::vector<std::pair<std::string, std::string>> *margin,
                                         std::vector<std::pair<std::string, std::string>> *padding,
                                         std::vector<std::pair<std::string, std::string>> *border) {
    render_action *action = new RenderActionUpdateStyle(PageId(), render->Ref(), style, margin, padding, border);
    PostRenderAction(action);
  }

  void RenderPage::SendUpdateAttrAction(RenderObject *render,
                                        std::vector<std::pair<std::string, std::string>> *attrs) {
    render_action *action = new RenderActionUpdateAttr(PageId(), render->Ref(), attrs);
    PostRenderAction(action);
  }

  void RenderPage::SendUpdateAttrAction(RenderObject *render,
                                        std::map<std::string, std::string> *attrs) {
    std::vector<std::pair<std::string, std::string>> *vAttrs = new std::vector<std::pair<std::string, std::string>>();
    for (auto iter = attrs->cbegin(); iter != attrs->cend(); iter++) {
      vAttrs->insert(vAttrs->begin(), std::pair<std::string, std::string>(iter->first, iter->second));
    }

    render_action *action = new RenderActionUpdateAttr(PageId(), render->Ref(), vAttrs);
    PostRenderAction(action);

    if (vAttrs != nullptr) {
      vAttrs->clear();
      delete vAttrs;
      vAttrs = nullptr;
    }
  }

  void RenderPage::SendCreateFinishAction() {
    render_action *action = new RenderActionCreateFinish(PageId());
    PostRenderAction(action);
  }

  void RenderPage::SendAppendTreeCreateFinish(const std::string &ref) {
    render_action *action = new RenderActionAppendTreeCreateFinish(PageId(), ref);
    PostRenderAction(action);
  }

  void RenderPage::CssLayoutTime(const long long &time) {
    if (mWXCorePerformance != nullptr)
      mWXCorePerformance->cssLayoutTime += time;
  }

  void RenderPage::ParseJsonTime(const long long &time) {
    if (mWXCorePerformance != nullptr)
      mWXCorePerformance->parseJsonTime += time;
  }

  void RenderPage::CallBridgeTime(const long long &time) {
    if (mWXCorePerformance != nullptr)
      mWXCorePerformance->callBridgeTime += time;
  }

  std::vector<long> RenderPage::PrintFirstScreenLog() {
    std::vector<long> ret;
    if (mWXCorePerformance != nullptr)
      ret = mWXCorePerformance->PrintPerformanceLog(onFirstScreen);
    return ret;
  }

  std::vector<long> RenderPage::PrintRenderSuccessLog() {
    std::vector<long> ret;
    if (mWXCorePerformance != nullptr)
      ret = mWXCorePerformance->PrintPerformanceLog(onRenderSuccess);
    return ret;
  }

  void RenderPage::Batch() {
    if ((useVSync && needLayout.load()) || !useVSync) {
      CalculateLayout();
      needLayout.store(false);
      updateDirty(false);
    }
  }

  void RenderPage::OnRenderPageInit() {
  }

  void RenderPage::OnRenderProcessStart() {

  }

  void RenderPage::OnRenderProcessExited() {

  }

  void RenderPage::OnRenderProcessGone() {

  }

  void RenderPage::OnRenderPageClose() {
  }
} //namespace WeexCore
