#include <rapidjson/weexjsontools.h>
#include <WeexCore/render/action/AddElementAction.h>
#include <WeexCore/render/action/CreateBodyAction.h>
#include <WeexCore/render/action/UpdateStyleAction.h>
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
    float radio = deviceWidth / (750.0f * 1.2f);

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

  void RenderPage::createRootRender(std::string data) {
    int alen = data.length();
    char *c_data = (char *) malloc(alen + 1);
    strcpy(c_data, data.c_str());
    c_data[alen] = 0;
    RenderObject *render = json2RenderObject(c_data, this);
    free(c_data);
    c_data = nullptr;

    if (render == nullptr)
      return;
    setRootRenderObject(render);
    pushRenderToMap(render);
    sendCreateBodyAction(render);
  }

  void RenderPage::addRenderObject(std::string parentRef, int insertPosition, RenderObject *child) {
    RenderObject *parent = getRenderObject(parentRef);
    if (parent == nullptr) {
      return;
    }

    if (child == nullptr) {
      return;
    }

    pushRenderToMap(child);

    // add child to Render Tree
    child->setParentRender(parent);
    parent->addRenderObject(insertPosition, child);
    sendAddElementAction(child, parent, insertPosition);
    calculateLayout();
  }

  void RenderPage::removeRenderObject(std::string ref) {
    RenderObject *child = getRenderObject(ref);
    if (child == nullptr)
      return;

    RenderObject *parent = child->getParentRender();
    if (parent == nullptr)
      return;

    parent->removeRenderObject(child);
    mRenderObjectMap.erase(child->getRef());
    delete child;
  }

  void RenderPage::moveRenderObject(std::string ref, std::string parentRef, std::string index) {
    RenderObject *child = getRenderObject(ref);
    if (child == nullptr)
      return;

    RenderObject *oldParent = child->getParentRender();
    if (oldParent == nullptr)
      return;

    RenderObject *newParent = getRenderObject(parentRef);
    if (newParent == nullptr)
      return;

    oldParent->removeRenderObject(child);
    newParent->addRenderObject(stringToNum<int>(index), child);
  }

  void RenderPage::updateStyle(std::string ref, std::string key, std::string value) {
    RenderObject *render = getRenderObject(ref);
    if (render == nullptr)
      return;

    render->addStyle(key, value);
  }

  void RenderPage::updateAttr(std::string ref, std::string key, std::string value) {
    RenderObject *render = getRenderObject(ref);
    if (render == nullptr)
      return;

    render->addAttr(key, value);
  }

  void RenderPage::addEvent(std::string ref, std::string event) {
    RenderObject *render = getRenderObject(ref);
    if (render == nullptr)
      return;

    render->addEvent(event);
  }

  void RenderPage::removeEvent(std::string ref, std::string event) {
    RenderObject *render = getRenderObject(ref);
    if (render == nullptr)
      return;

    render->removeEvent(event);
  }

  void RenderPage::createFinish() {
    if (pRoot != nullptr)
      traverseTree(pRoot);
    sendCreateFinishAction();
  }

  void RenderPage::addRenderAction(RenderAction *action) {
    if (action != nullptr)
      action->ExecuteAction();
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

  void RenderPage::sendUpdateStyleAction(std::string key, std::string value, std::string ref) {
    UpdateStyleAction *action = new UpdateStyleAction();
    action->GenerateAction(getPageId(), ref, key, value);
    addRenderAction(action);
  }

  void RenderPage::sendUpdateAttrAction(std::string key, std::string value, std::string ref) {
    UpdateStyleAction *action = new UpdateStyleAction();
    action->GenerateAction(getPageId(), ref, key, value);
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

  void RenderPage::bindInstance_Impl_Android(jobject instance) {
    if (instance != nullptr)
      this->mInstance_Impl_Android = getJNIEnv()->NewGlobalRef(instance);
  }

  void RenderPage::bindInstance_Impl_iOS(void *instance) {
    if (instance != nullptr)
      this->mInstance_Impl_iOS = instance;
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
