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
#include <WeexCore/env/RenderPerformance.h>
#include <WeexCore/env/CoreEnvironment.h>
#include "RenderPage.h"
#include "WeexCore/render/manager/RenderManager.h"
#include "WeexCore/render/node/RenderObject.h"

namespace WeexCore {

  typedef std::vector<RenderAction *>::iterator RENDERACTION_IT;
  typedef std::map<std::string, RenderObject *>::iterator RENDEROBJECT_COLLECTION_IT;

  RenderPage::RenderPage(std::string pageID) {
    mPageId = pageID;
    mWXCorePerformance = new RenderPerformance();
    mInstance_Impl_Android = nullptr;
    mInstance_Impl_iOS = nullptr;
    mViewPortWidth = 750.0f;
  }

  RenderPage::~RenderPage() {

    JNIEnv *env = getJNIEnv();

    if (pRoot != nullptr) {
      delete pRoot;
      pRoot = nullptr;
    }

    for (RENDERACTION_IT it = mRenderActions.begin();
         it != mRenderActions.end(); it++) {
      if (nullptr != *it) {
        delete *it;
        *it = nullptr;
      }
    }
    mRenderActions.clear();

    RENDEROBJECT_COLLECTION_IT begin = mRenderObjectRegisterMap.begin();
    RENDEROBJECT_COLLECTION_IT end = mRenderObjectRegisterMap.end();
    for (; begin != end; ++begin) {
      delete begin->second;
    }
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

  void RenderPage::calculateLayout() {
    if (pRoot == nullptr)
      return;
    long long startTime = getCurrentTime();
    pRoot->calculateLayout();
    cssLayoutTime(getCurrentTime() - startTime);

    float deviceHeight = WXCoreEnvironment::getInstance()->getDeviceHeight();
    float deviceWidth = WXCoreEnvironment::getInstance()->getDeviceWidth();
    float radio = deviceWidth / (mViewPortWidth * LAYOUT_FIRSTSCREEN_OVERFLOW_RADIO);

    switch (pRoot->getFlexDirection()) {
      case WXCore_Flex_Direction_Column:
      case WXCore_Flex_Direction_Column_Reverse:
        if (pRoot->getTotalMainSize() * radio > deviceHeight) {
          traverseTree(pRoot);
        }
        break;
      case WXCore_Flex_Direction_Row:
      case WXCore_Flex_Direction_Row_Reverse:
      default:
        if (pRoot->getTotalMainSize() * radio > deviceWidth) {
          traverseTree(pRoot);
        }
        break;
    }
  }

  void RenderPage::traverseTree(RenderObject *render) {

    if (render == nullptr)
      return;

    if (render->hasNewLayout()) {
      sendLayoutAction(render);
      render->setHasNewLayout(false);
    }

    for (int i = 0; i < render->getChildCount(); i++) {
      RenderObject *child = render->getChild(i);
      if (child != nullptr)
        traverseTree(child);
    }
  }

  bool RenderPage::createRootRender(RenderObject *root) {
    if (root == nullptr)
      return false;
    long long startTime = getCurrentTime();
    setRootRenderObject(root);

    pushRenderToRegisterMap(root);

    buildRenderObjectTime(getCurrentTime() - startTime);
    sendCreateBodyAction(root);
    return true;
  }

  bool RenderPage::addRenderObject(std::string parentRef, int insertPosition, RenderObject *child) {
    long long startTime = getCurrentTime();
    RenderObject *parent = getRenderObject(parentRef);
    if (parent == nullptr || child == nullptr) {
      return false;
    }

    pushRenderToRegisterMap(child);

    // add child to Render Tree
    child->setParentRender(parent);
    parent->addRenderObject(insertPosition, child);
    buildRenderObjectTime(getCurrentTime() - startTime);
    sendAddElementAction(child, parent, insertPosition);

    calculateLayout();
    return true;
  }

  bool RenderPage::removeRenderObject(std::string ref) {
    long long startTime = getCurrentTime();
    RenderObject *child = getRenderObject(ref);
    if (child == nullptr)
      return false;

    RenderObject *parent = child->getParentRender();
    if (parent == nullptr)
      return false;

    removeRenderFromRegisterMap(child);
    parent->removeRenderObject(child);

    delete child;

    buildRenderObjectTime(getCurrentTime() - startTime);

    sendRemoveElementAction(ref);
    return true;
  }

  bool RenderPage::moveRenderObject(std::string ref, std::string parentRef, int index) {
    long long startTime = getCurrentTime();

    RenderObject *child = getRenderObject(ref);
    if (child == nullptr)
      return false;

    RenderObject *oldParent = child->getParentRender();
    RenderObject *newParent = getRenderObject(parentRef);
    if (oldParent == nullptr || newParent == nullptr)
      return false;

    if (oldParent->getRef() == newParent->getRef()) {
      if (oldParent->indexOf(child) < 0) {
        return false;
      } else if (oldParent->indexOf(child) == index) {
        return false;
      } else if (oldParent->indexOf(child) < index) {
        index = index - 1;
      }
    }

    child->getParent()->removeChild(child);
    newParent->addChildAt(child, index);

    buildRenderObjectTime(getCurrentTime() - startTime);

    sendMoveElementAction(ref, parentRef, index);
    return true;
  }

  bool RenderPage::updateStyle(std::string ref,
                               std::vector<std::pair<std::string, std::string> *> *styles) {
    long long startTime = getCurrentTime();
    RenderObject *render = getRenderObject(ref);
    if (render == nullptr || styles == nullptr || styles->empty())
      return false;

    std::vector<std::pair<std::string, std::string> *> *style = nullptr;
    std::vector<std::pair<std::string, std::string> *> *margin = nullptr;
    std::vector<std::pair<std::string, std::string> *> *padding = nullptr;
    std::vector<std::pair<std::string, std::string> *> *border = nullptr;

    bool flag = false;

    for (int i = 0; i < styles->size(); ++i) {
      if ((*styles)[i] != nullptr) {
        switch (render->addStyle((*styles)[i]->first, (*styles)[i]->second)) {
          case TypeStyle:
            if (style == nullptr)
              style = new std::vector<std::pair<std::string, std::string> *>();
            style->insert(style->end(), (*styles)[i]);
            flag = true;
            break;
          case TypeMargin:
            if (margin == nullptr)
              margin = new std::vector<std::pair<std::string, std::string> *>();
            margin->insert(margin->end(), (*styles)[i]);
            flag = true;
            break;
          case TypePadding:
            if (padding == nullptr)
              padding = new std::vector<std::pair<std::string, std::string> *>();
            padding->insert(padding->end(), (*styles)[i]);
            flag = true;
            break;
          case TypeBorder:
            if (border == nullptr)
              border = new std::vector<std::pair<std::string, std::string> *>();
            border->insert(border->end(), (*styles)[i]);
            flag = true;
            break;
        }
      }
    }

    buildRenderObjectTime(getCurrentTime() - startTime);

    sendUpdateStyleAction(render, style, margin, padding, border);
    calculateLayout();

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

  bool RenderPage::updateAttr(std::string ref,
                              std::vector<std::pair<std::string, std::string> *> *attrs) {
    long long startTime = getCurrentTime();
    RenderObject *render = getRenderObject(ref);
    if (render == nullptr || attrs == nullptr || attrs->empty())
      return false;

    for (int i = 0; i < attrs->size(); ++i) {
      if ((*attrs)[i] == nullptr)
        return false;
      render->addAttr((*attrs)[i]->first, (*attrs)[i]->second);
    }

    buildRenderObjectTime(getCurrentTime() - startTime);

    sendUpdateAttrAction(render, attrs);

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

  bool RenderPage::addEvent(std::string ref, std::string event) {
    long long startTime = getCurrentTime();
    RenderObject *render = getRenderObject(ref);
    if (render == nullptr)
      return false;

    render->addEvent(event);
    buildRenderObjectTime(getCurrentTime() - startTime);

    return true;
  }

  bool RenderPage::removeEvent(std::string ref, std::string event) {
    long long startTime = getCurrentTime();
    RenderObject *render = getRenderObject(ref);
    if (render == nullptr)
      return false;

    render->removeEvent(event);
    buildRenderObjectTime(getCurrentTime() - startTime);

    return true;
  }

  bool RenderPage::createFinish() {
    if (pRoot == nullptr) {
      return false;
    }
    traverseTree(pRoot);
    sendCreateFinishAction();
    return true;
  }

  void RenderPage::postRenderAction(RenderAction *action) {
    if (action != nullptr) {
      action->ExecuteAction();
    }
  }

  void RenderPage::pushRenderToRegisterMap(RenderObject *render) {
    if (render == nullptr)
      return;

    mRenderObjectRegisterMap.insert(
        std::pair<std::string, RenderObject *>(render->getRef(), render));

    for (int i = 0; i < render->getChildCount(); ++i) {
      pushRenderToRegisterMap(render->getChild(i));
    }
  }

  void RenderPage::removeRenderFromRegisterMap(RenderObject *render) {
    if (render == nullptr)
      return;

    mRenderObjectRegisterMap.erase(render->getRef());

    for (int i = 0; i < render->getChildCount(); ++i) {
      removeRenderFromRegisterMap(render->getChild(i));
    }
  }

  void RenderPage::sendCreateBodyAction(RenderObject *render) {
    if (render == nullptr)
      return;

    RenderAction *action = new RenderActionCreateBody(getPageId(), render);
    postRenderAction(action);
  }

  void RenderPage::sendAddElementAction(RenderObject *child, RenderObject *parent, int index) {
    if (child == nullptr || parent == nullptr)
      return;

    RenderAction *action = new RenderActionAddElement(getPageId(), child, parent, index);
    postRenderAction(action);

    for (int i = 0; i < child->getChildCount(); ++i) {
      sendAddElementAction(child->getChild(i), child, i);
    }
  }

  void RenderPage::sendRemoveElementAction(std::string ref) {
    RenderAction *action = new RenderActionRemoveElement(getPageId(), ref);
    postRenderAction(action);
  }

  void RenderPage::sendMoveElementAction(std::string ref, std::string parentRef, int index) {
    RenderAction *action = new RenderActionMoveElement(getPageId(), ref, parentRef, index);
    postRenderAction(action);
  }

  void RenderPage::sendLayoutAction(RenderObject *render) {
    if (render == nullptr)
      return;

    RenderAction *action = new RenderActionLayout(getPageId(), render);
    postRenderAction(action);
  }

  void RenderPage::sendUpdateStyleAction(RenderObject *render,
                                         std::vector<std::pair<std::string, std::string> *> *style,
                                         std::vector<std::pair<std::string, std::string> *> *margin,
                                         std::vector<std::pair<std::string, std::string> *> *padding,
                                         std::vector<std::pair<std::string, std::string> *> *border) {
    RenderAction *action = new RenderActionUpdateStyle(getPageId(), render->getRef(),
                                                       style, margin, padding, border);
    postRenderAction(action);
  }

  void RenderPage::sendUpdateAttrAction(RenderObject *render,
                                        std::vector<std::pair<std::string, std::string> *> *attrs) {
    RenderAction *action = new RenderActionUpdateAttr(getPageId(), render->getRef(),
                                                      attrs);
    postRenderAction(action);
  }

  void RenderPage::sendAddEventAction(RenderObject *render) {

  }

  void RenderPage::sendCreateFinishAction() {
    RenderAction *action = new RenderActionCreateFinish(getPageId());
    postRenderAction(action);
  }

  void RenderPage::jniCallTime(long long time) {
    if (mWXCorePerformance != nullptr)
      mWXCorePerformance->jniCallTime += time;
  }

  void RenderPage::cssLayoutTime(long long time) {
    if (mWXCorePerformance != nullptr)
      mWXCorePerformance->cssLayoutTime += time;
  }

  void RenderPage::addElementActionJNITime(long long time) {
    if (mWXCorePerformance != nullptr)
      mWXCorePerformance->addElementActionJNITime += time;
  }

  void RenderPage::layoutActionJniTime(long long time) {
    if (mWXCorePerformance != nullptr)
      mWXCorePerformance->layoutActionJniTime += time;
  }

  void RenderPage::parseJsonTime(long long time) {
    if (mWXCorePerformance != nullptr)
      mWXCorePerformance->parseJsonTime += time;
  }

  void RenderPage::buildRenderObjectTime(long long time) {
    if (mWXCorePerformance != nullptr)
      mWXCorePerformance->buildRenderObjectTime += time;
  }

  void RenderPage::createJMapJNITime(long long time) {
    if (mWXCorePerformance != nullptr)
      mWXCorePerformance->createJMapJNITime += time;
  }

  void RenderPage::jniCallBridgeTime(long long time) {
    if (mWXCorePerformance != nullptr)
      mWXCorePerformance->jniCallBridgeTime += time;
  }

  bool RenderPage::bindInstance_Impl_Android(jobject instance) {
    if (instance == nullptr)
      return false;
    this->mInstance_Impl_Android = getJNIEnv()->NewGlobalRef(instance);
    return true;
  }

  bool RenderPage::bindInstance_Impl_iOS(void *instance) {
    if (instance == nullptr)
      return false;
    this->mInstance_Impl_iOS = instance;
    return true;
  }

  void RenderPage::printFirstScreenLog() {
    if (mWXCorePerformance != nullptr)
      mWXCorePerformance->printPerformanceLog(onFirstScreen);
  }

  void RenderPage::printRenderSuccessLog() {
    if (mWXCorePerformance != nullptr)
      mWXCorePerformance->printPerformanceLog(onRenderSuccess);
  }
} //namespace WeexCore
