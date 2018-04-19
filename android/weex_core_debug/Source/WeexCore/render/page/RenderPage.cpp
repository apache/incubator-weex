#include <WeexCore/parser/DomParser.h>
#include <WeexCore/render/action/RenderActionAddElement.h>
#include <WeexCore/render/action/RenderActionRemoveElement.h>
#include <WeexCore/render/action/RenderActionMoveElement.h>
#include <WeexCore/render/action/RenderActionCreateBody.h>
#include <WeexCore/render/action/RenderActionUpdateStyle.h>
#include <WeexCore/render/action/RenderActionUpdateAttr.h>
#include <WeexCore/render/action/RenderActionLayout.h>
#include <WeexCore/render/action/RenderActionCreateFinish.h>
#include <WeexCore/layout/WXCoreLayout.h>
#include <WeexCore/platform/android/base/string/StringUtils.h>
#include <WeexCore/moniter/RenderPerformance.h>
#include <WeexCore/config/CoreEnvironment.h>
#include <base/ViewUtils.h>
#include <WeexCore/render/action/RenderActionAddEvent.h>
#include <WeexCore/render/action/RenderActionRemoveEvent.h>
#include <WeexCore/css/ConstantsValue.h>
#include "RenderPage.h"
#include "WeexCore/render/manager/RenderManager.h"
#include "WeexCore/render/node/RenderObject.h"

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

    long long startTime = getCurrentTime();
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

    BuildRenderTreeTime(getCurrentTime() - startTime);
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
    long long startTime = getCurrentTime();
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
    BuildRenderTreeTime(getCurrentTime() - startTime);
    SendAddElementAction(child, parent, insertPosition);

    Batch();
    return true;
  }

  bool RenderPage::RemoveRenderObject(const std::string &ref) {
    long long startTime = getCurrentTime();
    RenderObject *child = GetRenderObject(ref);
    if (child == nullptr)
      return false;

    RenderObject *parent = child->GetParentRender();
    if (parent == nullptr)
      return false;

    parent->RemoveRenderObject(child);

    RemoveRenderFromRegisterMap(child);
    delete child;

    BuildRenderTreeTime(getCurrentTime() - startTime);

    SendRemoveElementAction(ref);
    return true;
  }

  bool RenderPage::MoveRenderObject(const std::string &ref, const std::string &parentRef, int index) {
    long long startTime = getCurrentTime();

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

    BuildRenderTreeTime(getCurrentTime() - startTime);

    SendMoveElementAction(ref, parentRef, index);
    return true;
  }

  bool RenderPage::UpdateStyle(const std::string &ref, std::vector<std::pair<std::string, std::string>> *src) {
    long long startTime = getCurrentTime();
    RenderObject *render = GetRenderObject(ref);
    if (render == nullptr || src == nullptr || src->empty())
      return false;

    std::vector<std::pair<std::string, std::string>> *style = nullptr;
    std::vector<std::pair<std::string, std::string>> *margin = nullptr;
    std::vector<std::pair<std::string, std::string>> *padding = nullptr;
    std::vector<std::pair<std::string, std::string>> *border = nullptr;

    bool flag = false;

    int result = Bridge_Impl_Android::getInstance()->callHasTransitionPros(mPageId.c_str(), ref.c_str(), src);

    if (result == 1) {
      BuildRenderTreeTime(getCurrentTime() - startTime);
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
                                  (*iter).second = std::to_string(foo),
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
                                  (*iter).second = std::to_string(foo),
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
                                  (*iter).second = std::to_string(foo),
                                      border->insert(border->end(), (*iter)),
                                  flag = true;
                                });
            break;
        }
      }
    }

    BuildRenderTreeTime(getCurrentTime() - startTime);

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
    long long startTime = getCurrentTime();
    RenderObject *render = GetRenderObject(ref);
    if (render == nullptr || attrs == nullptr || attrs->empty())
      return false;

    SendUpdateAttrAction(render, attrs);

    for (auto iter = attrs->cbegin(); iter != attrs->cend(); iter++) {
      render->UpdateAttr((*iter).first, (*iter).second);
    }

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
    long long startTime = getCurrentTime();
    RenderObject *render = GetRenderObject(ref);
    if (render == nullptr)
      return false;

    render->AddEvent(event);
    BuildRenderTreeTime(getCurrentTime() - startTime);

    RenderAction *action = new RenderActionAddEvent(mPageId, ref, event);
    PostRenderAction(action);
    return true;
  }

  bool RenderPage::RemoveEvent(const std::string &ref, const std::string &event) {
    long long startTime = getCurrentTime();
    RenderObject *render = GetRenderObject(ref);
    if (render == nullptr)
      return false;

    render->RemoveEvent(event);
    BuildRenderTreeTime(getCurrentTime() - startTime);

    RenderAction *action = new RenderActionRemoveEvent(mPageId, ref, event);
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

  void RenderPage::PostRenderAction(RenderAction *action) {
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

    RenderAction *action = new RenderActionCreateBody(PageId(), render);
    PostRenderAction(action);

    Index i = 0;
    for(auto it = render->ChildListIterBegin(); it != render->ChildListIterEnd(); it++) {
      RenderObject* child = static_cast<RenderObject*>(*it);
      if (child != nullptr) {
        SendAddElementAction(child, render, i);
      }
      ++i;
    }
  }

  void RenderPage::SendAddElementAction(RenderObject *child, RenderObject *parent, int index) {
    if (child == nullptr || parent == nullptr)
      return;

    RenderAction *action = new RenderActionAddElement(PageId(), child, parent, index);
    PostRenderAction(action);

    Index i = 0;
    for(auto it = child->ChildListIterBegin(); it != child->ChildListIterEnd(); it++) {
      RenderObject* grandson = static_cast<RenderObject*>(*it);
      if (grandson != nullptr) {
        SendAddElementAction(grandson, child, i);
      }
      ++i;
    }
  }

  void RenderPage::SendRemoveElementAction(const std::string &ref) {
    RenderAction *action = new RenderActionRemoveElement(PageId(), ref);
    PostRenderAction(action);
  }

  void RenderPage::SendMoveElementAction(const std::string &ref, const std::string &parentRef, int index) {
    RenderAction *action = new RenderActionMoveElement(PageId(), ref, parentRef, index);
    PostRenderAction(action);
  }

  void RenderPage::SendLayoutAction(RenderObject *render, int index) {
    if (render == nullptr)
      return;

    RenderAction *action = new RenderActionLayout(PageId(), render, index);
    PostRenderAction(action);
  }

  void RenderPage::SendUpdateStyleAction(RenderObject *render,
                                         std::vector<std::pair<std::string, std::string>> *style,
                                         std::vector<std::pair<std::string, std::string>> *margin,
                                         std::vector<std::pair<std::string, std::string>> *padding,
                                         std::vector<std::pair<std::string, std::string>> *border) {
    RenderAction *action = new RenderActionUpdateStyle(PageId(), render->Ref(), style, margin, padding, border);
    PostRenderAction(action);
  }

  void RenderPage::SendUpdateAttrAction(RenderObject *render,
                                        std::vector<std::pair<std::string, std::string>> *attrs) {
    RenderAction *action = new RenderActionUpdateAttr(PageId(), render->Ref(), attrs);
    PostRenderAction(action);
  }

  void RenderPage::SendCreateFinishAction() {
    RenderAction *action = new RenderActionCreateFinish(PageId());
    PostRenderAction(action);
  }

  void RenderPage::JniCallTime(const long long &time) {
    if (mWXCorePerformance != nullptr)
      mWXCorePerformance->jniCallTime += time;
  }

  void RenderPage::CssLayoutTime(const long long &time) {
    if (mWXCorePerformance != nullptr)
      mWXCorePerformance->cssLayoutTime += time;
  }

  void RenderPage::AddEventActionJNITime(const long long &time) {
    if (mWXCorePerformance != nullptr)
      mWXCorePerformance->addEventActionJNITime += time;
  }

  void RenderPage::RemoveEventActionJNITime(const long long &time) {
    if (mWXCorePerformance != nullptr)
      mWXCorePerformance->removeEventActionJNITime += time;
  }

  void RenderPage::AddElementActionJNITime(const long long &time) {
    if (mWXCorePerformance != nullptr)
      mWXCorePerformance->addElementActionJNITime += time;
  }

  void RenderPage::LayoutActionJniTime(const long long &time) {
    if (mWXCorePerformance != nullptr)
      mWXCorePerformance->layoutActionJniTime += time;
  }

  void RenderPage::ParseJsonTime(const long long &time) {
    if (mWXCorePerformance != nullptr)
      mWXCorePerformance->parseJsonTime += time;
  }

  void RenderPage::BuildRenderTreeTime(const long long &time) {
    if (mWXCorePerformance != nullptr)
      mWXCorePerformance->buildRenderObjectTime += time;
  }

  void RenderPage::CreateJMapJNITime(const long long &time) {
    if (mWXCorePerformance != nullptr)
      mWXCorePerformance->createJMapJNITime += time;
  }

  void RenderPage::CallBridgeTime(const long long &time) {
    if (mWXCorePerformance != nullptr)
      mWXCorePerformance->jniCallBridgeTime += time;
  }

  int RenderPage::PrintFirstScreenLog() {
    if (mWXCorePerformance != nullptr)
      return mWXCorePerformance->PrintPerformanceLog(onFirstScreen);
    return 0;
  }

  int RenderPage::PrintRenderSuccessLog() {
    if (mWXCorePerformance != nullptr)
      return mWXCorePerformance->PrintPerformanceLog(onRenderSuccess);
    return 0;
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
