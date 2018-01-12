#include <WeexCore/render/node/factory/parser/DomParser.h>
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
#include <WeexCore/env/RenderPerformance.h>
#include <WeexCore/env/CoreEnvironment.h>
#include <base/ViewUtils.h>
#include <WeexCore/render/action/RenderActionAddEvent.h>
#include <WeexCore/render/action/RenderActionRemoveEvent.h>
#include "RenderPage.h"
#include "WeexCore/render/manager/RenderManager.h"
#include "WeexCore/render/node/RenderObject.h"

namespace WeexCore {

  typedef std::vector<RenderAction *>::iterator RenderActionIterator;

  RenderPage::RenderPage(const std::string &pageID) {
    mPageId = pageID;
    mWXCorePerformance = new RenderPerformance();
    mInstance_Impl_Android = nullptr;
    mInstance_Impl_iOS = nullptr;
    mViewPortWidth = 750.0f;
  }

  RenderPage::~RenderPage() {
    JNIEnv *env = getJNIEnv();

    if (render_root != nullptr) {
      delete render_root;
      render_root = nullptr;
    }

    for (RenderActionIterator it = mRenderActions.begin();
         it != mRenderActions.end(); it++) {
      if (nullptr != *it) {
        delete *it;
        *it = nullptr;
      }
    }
    mRenderActions.clear();

    mRenderObjectRegisterMap.clear();

    if (mWXCorePerformance != nullptr) {
      delete mWXCorePerformance;
      mWXCorePerformance = nullptr;
    }

    if (mInstance_Impl_Android != nullptr) {
      env->DeleteGlobalRef(mInstance_Impl_Android);
      mInstance_Impl_Android = nullptr;
    }

    if (mInstance_Impl_iOS != nullptr) {
      delete mInstance_Impl_iOS;
      mInstance_Impl_iOS = nullptr;
    }
  }

  void RenderPage::CalculateLayout() {
    if (render_root == nullptr || !render_root->ViewInit())
      return;

    render_root->LayoutBefore();
    long long startTime = getCurrentTime();
    render_root->calculateLayout();
    CssLayoutTime(getCurrentTime() - startTime);
//    std::string mMessage = "end calculateLayout";
//    Bridge_Impl_Android::getInstance()->callLogOfFirstScreen(mMessage);
    render_root->LayoutAfter();
//    mMessage = "end layoutAfter";
//    Bridge_Impl_Android::getInstance()->callLogOfFirstScreen(mMessage);

//    float deviceHeight = WXCoreEnvironment::getInstance()->getDeviceHeight();
//    float deviceWidth = WXCoreEnvironment::getInstance()->getDeviceWidth();
//    float radio = deviceWidth / (mViewPortWidth * kLayoutFirstScreenOverflowRadio);

//    switch (render_root->getFlexDirection()) {
//      case kFlexDirectionColumn:
//      case kFlexDirectionColumnReverse:
//        if (render_root->getLargestMainSize() * radio > deviceHeight / 2) {
//          traverseTree(render_root);
//        }
//        break;
//      case kFlexDirectionRow:
//      case kFlexDirectionRowReverse:
//      default:
//        if (render_root->getLargestMainSize() * radio > deviceWidth / 2) {
//          traverseTree(render_root);
//        }
//        break;
//    }
    TraverseTree(render_root);
  }

  void RenderPage::TraverseTree(RenderObject *render) {

    if (render == nullptr)
      return;

    if (render->hasNewLayout()) {
      SendLayoutAction(render);
      render->setHasNewLayout(false);
    }

    for (Index i = 0; i < render->getChildCount(); i++) {
      RenderObject *child = render->GetChild(i);
      if (child != nullptr)
        TraverseTree(child);
    }
  }

  bool RenderPage::CreateRootRender(RenderRoot *root) {
    if (root == nullptr)
      return false;
    long long startTime = getCurrentTime();
    SetRootRenderObject(root);

    render_root->setStyleWidth(WXCoreEnvironment::getInstance()->DeviceWidth());
    render_root->setStyleHeight(WXCoreEnvironment::getInstance()->DeviceHeight());

    PushRenderToRegisterMap(root);

    BuildRenderTreeTime(getCurrentTime() - startTime);
//    std::string mMessage = "sendCreateBodyAction";
//    Bridge_Impl_Android::getInstance()->callLogOfFirstScreen(mMessage);
    SendCreateBodyAction(root);
    return true;
  }

  bool RenderPage::AddRenderObject(const std::string &parentRef, const int &insertPosition,
                                   RenderObject *child) {
    long long startTime = getCurrentTime();
    RenderObject *parent = GetRenderObject(parentRef);
    if (parent == nullptr || child == nullptr) {
      return false;
    }

    PushRenderToRegisterMap(child);

    // add child to Render Tree
    child->SetParentRender(parent);
    parent->AddRenderObject(insertPosition, child);
    BuildRenderTreeTime(getCurrentTime() - startTime);
//    std::string mMessage = "start sendAddElementAction";
//    Bridge_Impl_Android::getInstance()->callLogOfFirstScreen(mMessage);
    SendAddElementAction(child, parent, insertPosition);

//    mMessage = "start calculateLayout";
//    Bridge_Impl_Android::getInstance()->callLogOfFirstScreen(mMessage);
    CalculateLayout();
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

    BuildRenderTreeTime(getCurrentTime() - startTime);

    SendRemoveElementAction(ref);
    return true;
  }

  bool
  RenderPage::MoveRenderObject(const std::string &ref, const std::string &parentRef, int &index) {
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

  bool RenderPage::UpdateStyle(const std::string &ref,
                               std::vector<std::pair<std::string, std::string> *> *styles) {
    long long startTime = getCurrentTime();
    RenderObject *render = GetRenderObject(ref);
    if (render == nullptr || styles == nullptr || styles->empty())
      return false;

    std::vector<std::pair<std::string, std::string> *> *style = nullptr;
    std::vector<std::pair<std::string, std::string> *> *margin = nullptr;
    std::vector<std::pair<std::string, std::string> *> *padding = nullptr;
    std::vector<std::pair<std::string, std::string> *> *border = nullptr;

    bool flag = false;

    for (int i = 0; i < styles->size(); ++i) {
      if ((*styles)[i] != nullptr) {
        switch (render->AddStyle((*styles)[i]->first, (*styles)[i]->second)) {
          case kTypeStyle:
            if (style == nullptr)
              style = new std::vector<std::pair<std::string, std::string> *>();
            style->insert(style->end(), (*styles)[i]);
            flag = true;
            break;
          case kTypeMargin:
            if (margin == nullptr)
              margin = new std::vector<std::pair<std::string, std::string> *>();
            margin->insert(margin->end(), (*styles)[i]);
            flag = true;
            break;
          case kTypePadding:
            if (padding == nullptr)
              padding = new std::vector<std::pair<std::string, std::string> *>();
            padding->insert(padding->end(), (*styles)[i]);
            flag = true;
            break;
          case kTypeBorder:
            if (border == nullptr)
              border = new std::vector<std::pair<std::string, std::string> *>();
            border->insert(border->end(), (*styles)[i]);
            flag = true;
            break;
        }
      }
    }

    BuildRenderTreeTime(getCurrentTime() - startTime);

    SendUpdateStyleAction(render, style, margin, padding, border);
    CalculateLayout();

    if (style != nullptr) {
      for (int i = 0; i < style->size(); ++i) {
        if ((*style)[i] != nullptr) {
          delete (*style)[i];
          (*style)[i] = nullptr;
        }
      }
      style->clear();
      delete style;
      style = nullptr;
    }

    if (margin != nullptr) {
      for (int i = 0; i < margin->size(); ++i) {
        if ((*margin)[i] != nullptr) {
          delete (*margin)[i];
          (*margin)[i] = nullptr;
        }
      }
      margin->clear();
      delete margin;
      margin = nullptr;
    }

    if (padding != nullptr) {
      for (int i = 0; i < padding->size(); ++i) {
        if ((*padding)[i] != nullptr) {
          delete (*padding)[i];
          (*padding)[i] = nullptr;
        }
      }
      padding->clear();
      delete padding;
      padding = nullptr;
    }

    if (border != nullptr) {
      for (int i = 0; i < border->size(); ++i) {
        if ((*border)[i] != nullptr) {
          delete (*border)[i];
          (*border)[i] = nullptr;
        }
      }
      border->clear();
      delete border;
      border = nullptr;
    }

//    if (styles != nullptr) {
//      for (int i = 0; i < styles->size(); ++i) {
//        if ((*styles)[i] != nullptr) {
//          delete (*styles)[i];
//          (*styles)[i] = nullptr;
//        }
//      }
//      styles->clear();
//      delete styles;
//      styles = nullptr;
//    }
    return flag;
  }

  bool RenderPage::UpdateAttr(const std::string &ref,
                              std::vector<std::pair<std::string, std::string> *> *attrs) {
    long long startTime = getCurrentTime();
    RenderObject *render = GetRenderObject(ref);
    if (render == nullptr || attrs == nullptr || attrs->empty())
      return false;

    SendUpdateAttrAction(render, attrs);

    for (int i = 0; i < attrs->size(); ++i) {
      if ((*attrs)[i] == nullptr)
        return false;
      render->UpdateAttr((*attrs)[i]->first, (*attrs)[i]->second);
    }

    for (int i = 0; i < attrs->size(); ++i) {
      if ((*attrs)[i] != nullptr) {
        delete (*attrs)[i];
        (*attrs)[i] = nullptr;
      }
    }

    if (attrs != nullptr) {
      delete attrs;
      attrs = nullptr;
    }

    return true;
  }

  void RenderPage::SetDefaultHeightAndWidthIntoRootRender(const float defaultWidth,
                                                          const float defaultHeight) {
    render_root->setStyleWidth(defaultWidth);
    render_root->setStyleHeight(defaultHeight);
    CalculateLayout();
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
    TraverseTree(render_root);
    SendCreateFinishAction();
    return true;
  }

  void RenderPage::PostRenderAction(RenderAction *action) {
    if (action != nullptr) {
      action->ExecuteAction();
    }
  }

  void RenderPage::PushRenderToRegisterMap(RenderObject *render) {
    if (render == nullptr)
      return;

    mRenderObjectRegisterMap.insert(
        std::pair<std::string, RenderObject *>(render->Ref(), render));

    for (Index i = 0; i < render->getChildCount(); ++i) {
      PushRenderToRegisterMap(render->GetChild(i));
    }
  }

  void RenderPage::RemoveRenderFromRegisterMap(RenderObject *render) {
    if (render == nullptr)
      return;

    mRenderObjectRegisterMap.erase(render->Ref());

    for (Index i = 0; i < render->getChildCount(); ++i) {
      RemoveRenderFromRegisterMap(render->GetChild(i));
    }
    delete render;

  }

  void RenderPage::SendCreateBodyAction(RenderObject *render) {
    if (render == nullptr)
      return;

    RenderAction *action = new RenderActionCreateBody(PageId(), render);
    PostRenderAction(action);
  }

  void
  RenderPage::SendAddElementAction(RenderObject *child, RenderObject *parent, const int &index) {
    if (child == nullptr || parent == nullptr)
      return;

    RenderAction *action = new RenderActionAddElement(PageId(), child, parent, index);
    PostRenderAction(action);

    for (Index i = 0; i < child->getChildCount(); ++i) {
      SendAddElementAction(child->GetChild(i), child, i);
    }
  }

  void RenderPage::SendRemoveElementAction(const std::string &ref) {
    RenderAction *action = new RenderActionRemoveElement(PageId(), ref);
    PostRenderAction(action);
  }

  void RenderPage::SendMoveElementAction(const std::string &ref, const std::string &parentRef,
                                         int index) {
    RenderAction *action = new RenderActionMoveElement(PageId(), ref, parentRef, index);
    PostRenderAction(action);
  }

  void RenderPage::SendLayoutAction(RenderObject *render) {
    if (render == nullptr)
      return;

    RenderAction *action = new RenderActionLayout(PageId(), render);
    PostRenderAction(action);
  }

  void RenderPage::SendUpdateStyleAction(RenderObject *render,
                                         std::vector<std::pair<std::string, std::string> *> *style,
                                         std::vector<std::pair<std::string, std::string> *> *margin,
                                         std::vector<std::pair<std::string, std::string> *> *padding,
                                         std::vector<std::pair<std::string, std::string> *> *border) {
    RenderAction *action = new RenderActionUpdateStyle(PageId(), render->Ref(),
                                                       style, margin, padding, border);
    PostRenderAction(action);
  }

  void RenderPage::SendUpdateAttrAction(RenderObject *render,
                                        std::vector<std::pair<std::string, std::string> *> *attrs) {
    RenderAction *action = new RenderActionUpdateAttr(PageId(), render->Ref(),
                                                      attrs);
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

  bool RenderPage::BindInstanceImplAndroid(jobject instance) {
    if (instance == nullptr)
      return false;
    this->mInstance_Impl_Android = getJNIEnv()->NewGlobalRef(instance);
    return true;
  }

  bool RenderPage::BindInstanceImplIOS(void *instance) {
    if (instance == nullptr)
      return false;
    this->mInstance_Impl_iOS = instance;
    return true;
  }

  void RenderPage::PrintFirstScreenLog() {
    if (mWXCorePerformance != nullptr)
      mWXCorePerformance->PrintPerformanceLog(onFirstScreen);
  }

  void RenderPage::PrintRenderSuccessLog() {
    if (mWXCorePerformance != nullptr)
      mWXCorePerformance->PrintPerformanceLog(onRenderSuccess);
  }

  void RenderPage::Batch() {

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
