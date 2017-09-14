#include "RenderPage.h"

namespace WeexCore {
  RenderPage::RenderPage(std::string pageID, std::string data) {
    mPageId = stringToNum<float>(pageID);
    mRenderObjectMap = new std::map<std::string, RenderObject *>();

    int alen = data.length();
    char *c_data = (char *) malloc(alen + 1);
    strcpy(c_data, data.c_str());
    c_data[alen] = 0;
    RenderObject *render = json2RenderObject(c_data, this);
    free(c_data);
    c_data = nullptr;
    mRenderObjectMap->insert(pair<std::string, RenderObject *>(render->getRef(), render));
    setRootRenderObject(render);

    // layout by YogaNode Tree
    YGNodeCalculateLayout(pRoot->getYGNode(), YGUndefined, YGUndefined, YGDirectionLTR);
  }

  RenderPage::~RenderPage() {
    delete pRoot;

    for (RENDERACTION_IT it = mRenderActions.begin();
         it != mRenderActions.end(); it++) {
      if (nullptr != *it) {
        delete *it;
        *it = nullptr;
      }
    }
    mRenderActions.clear();

    RENDEROBJECT_COLLECTION_IT begin = mRenderObjectMap->begin();
    RENDEROBJECT_COLLECTION_IT end = mRenderObjectMap->end();
    for (; begin != end; ++begin) {
      delete begin->second;
    }

    mRenderObjectMap->clear();
    delete mRenderObjectMap;
  }

  void RenderPage::addRenderObject(std::string parentRef, int insertPosiotn, std::string data) {
    RenderObject *parent = getRenderObject(parentRef);
    if (parent == nullptr) {
      return;
    }
    char *c_data = (char *) data.data();
    RenderObject *child = json2RenderObject(c_data, this);
    if (child == nullptr) {
      return;
    }

    mRenderObjectMap->insert(pair<std::string, RenderObject *>(child->getRef(), child));

    // add child to YogaNode Tree
    YGNodeInsertChild(parent->getYGNode(), child->getYGNode(), parent->getChildCount());

    // add child to Render Tree
    child->setParentRender(parent);
    parent->addRenderObject(insertPosiotn, child);

    // layout by YogaNode Tree
    YGNodeCalculateLayout(pRoot->getYGNode(), YGUndefined, YGUndefined, YGDirectionLTR);
  }

  void RenderPage::removeRenderObject(std::string ref) {
    RenderObject *child = getRenderObject(ref);
    RenderObject *parent = child->getParentRender();

    parent->removeRenderObject(child);
    YGNodeRemoveChild(parent->getYGNode(), child->getYGNode());
    mRenderObjectMap->erase(child->getRef());
    delete child;

    // layout by YogaNode Tree
    YGNodeCalculateLayout(pRoot->getYGNode(), YGUndefined, YGUndefined, YGDirectionLTR);
  }

  void RenderPage::moveRenderObject(std::string ref, std::string parentRef, std::string index) {
    RenderObject *child = getRenderObject(ref);
    RenderObject *oldParent = child->getParentRender();
    RenderObject *newParent = getRenderObject(parentRef);

    oldParent->removeRenderObject(child);
    newParent->addRenderObject(stringToNum<int>(index), child);

    YGNodeCalculateLayout(pRoot->getYGNode(), YGUndefined, YGUndefined, YGDirectionLTR);
  }

  void RenderPage::updateStyle(std::string ref, std::string key, std::string value) {
    RenderObject *render = getRenderObject(ref);
    render->updateStyle(key, value);

    // layout by YogaNode Tree
    YGNodeCalculateLayout(pRoot->getYGNode(), YGUndefined, YGUndefined, YGDirectionLTR);
  }

  void RenderPage::updateAttr(std::string ref, std::string key, std::string value) {
    RenderObject *render = getRenderObject(ref);
    render->updateAttr(key, value);
  }

  void RenderPage::addEvent(std::string ref, std::string event) {
    RenderObject *render = getRenderObject(ref);
    render->addEvent(event);
  }

  void RenderPage::removeEvent(std::string ref, std::string event) {
    RenderObject *render = getRenderObject(ref);
    render->removeEvent(event);
  }

  void RenderPage::addRenderAction(RenderAction *action) {
    action->ExecuteAction();
  }

  RenderObject *RenderPage::getRenderObject(std::string ref) {
    return mRenderObjectMap->find(ref)->second;
  }

  void RenderPage::setRootRenderObject(RenderObject *root) {
    if (root != nullptr) {
      pRoot = root;
    }
  }

  RenderObject *RenderPage::getRootRenderObject() {
    return pRoot;
  }

} //namespace WeexCore
