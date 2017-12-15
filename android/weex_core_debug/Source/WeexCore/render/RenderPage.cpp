#include <rapidjson/weexjsontools.h>
#include <WeexCore/render/action/AddElementAction.h>
#include <WeexCore/render/action/CreateBodyAction.h>
#include <WeexCore/render/action/UpdateStyleAction.h>
#include <WeexCore/render/action/UpdateAttrAction.h>
#include <WeexCore/render/action/LayoutRenderAction.h>
#include <WeexCore/render/action/RenderAction.h>
#include <WeexCore/render/action/CreateFinishAction.h>
#include <Layout/WXCoreLayout.h>
#include <base/android/string/StringUtils.h>
#include <base/WXCorePerformance.h>
#include <base/WXCoreEnvironment.h>
#include "RenderPage.h"
#include "RenderManager.h"
#include "RenderObject.h"

namespace WeexCore {

  typedef std::vector<RenderAction *>::iterator RENDERACTION_IT;
  typedef std::map<std::string, RenderObject *>::iterator RENDEROBJECT_COLLECTION_IT;

  RenderPage::RenderPage(std::string pageID) {
    mPageId = pageID;
    mWXCorePerformance = new WXCorePerformance();
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

    RENDEROBJECT_COLLECTION_IT begin = mRenderObjectMap.begin();
    RENDEROBJECT_COLLECTION_IT end = mRenderObjectMap.end();
    for (; begin != end; ++begin) {
      delete begin->second;
    }
    mRenderObjectMap.clear();

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
        if (pRoot->getTotalMainSize() * radio > deviceHeight) {
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
    pushRenderToMap(root);
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

    pushRenderToMap(child);

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

    parent->removeRenderObject(child);
    mRenderObjectMap.erase(child->getRef());
    delete child;
    buildRenderObjectTime(getCurrentTime() - startTime);
    return true;
  }

  bool RenderPage::moveRenderObject(std::string ref, std::string parentRef, int index) {
    long long startTime = getCurrentTime();
    RenderObject *child = getRenderObject(ref);
    if (child == nullptr)
      return false;

    RenderObject *oldParent = child->getParentRender();
    if (oldParent == nullptr)
      return false;

    RenderObject *newParent = getRenderObject(parentRef);
    if (newParent == nullptr)
      return false;

    oldParent->removeRenderObject(child);
    newParent->addRenderObject(index, child);
    buildRenderObjectTime(getCurrentTime() - startTime);
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

  void RenderPage::addRenderAction(RenderAction *action) {
    if (action != nullptr) {
      action->ExecuteAction();
    }
  }

  void RenderPage::pushRenderToMap(RenderObject *render) {
    if (render == nullptr)
      return;

    mRenderObjectMap.insert(std::pair<std::string, RenderObject *>(render->getRef(), render));

    for (int i = 0; i < render->getChildCount(); ++i) {
      pushRenderToMap(render->getChild(i));
    }
  }

  void RenderPage::sendCreateBodyAction(RenderObject *render) {
    if (render == nullptr)
      return;

    CreateBodyAction *action = new CreateBodyAction();
    action->GenerateAction(getPageId(), render);
    addRenderAction(action);

    for (int i = 0; i < render->getChildCount(); ++i) {
      sendAddElementAction(render->getChild(i), render, i);
    }
  }

  void RenderPage::sendAddElementAction(RenderObject *child, RenderObject *parent, int index) {
    if (child == nullptr || parent == nullptr)
      return;

    AddElementAction *action = new AddElementAction();
    action->GenerateAction(getPageId(), child, parent, index);
    addRenderAction(action);

    for (int i = 0; i < child->getChildCount(); ++i) {
      sendAddElementAction(child->getChild(i), child, i);
    }
  }

  void RenderPage::sendLayoutAction(RenderObject *render) {
    if (render == nullptr)
      return;

    LayoutRenderAction *action = new LayoutRenderAction();
    action->GenerateAction(getPageId(), render);
    addRenderAction(action);
  }

  void RenderPage::sendUpdateStyleAction(RenderObject *render,
                                         std::vector<std::pair<std::string, std::string> *> *style,
                                         std::vector<std::pair<std::string, std::string> *> *margin,
                                         std::vector<std::pair<std::string, std::string> *> *padding,
                                         std::vector<std::pair<std::string, std::string> *> *border) {
    UpdateStyleAction *action = new UpdateStyleAction();
    action->GenerateAction(getPageId(), render->getRef(), style, margin, padding, border);
    addRenderAction(action);
  }

  void RenderPage::sendUpdateAttrAction(RenderObject *render,
                                        std::vector<std::pair<std::string, std::string> *> *attrs) {
    UpdateAttrAction *action = new UpdateAttrAction();
    action->GenerateAction(getPageId(), render->getRef(), attrs);
    addRenderAction(action);
  }

  void RenderPage::sendAddEventAction(RenderObject *render) {

  }

  void RenderPage::sendCreateFinishAction() {
    CreateFinishAction *action = new CreateFinishAction();
    action->GenerateAction(getPageId());
    addRenderAction(action);
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
