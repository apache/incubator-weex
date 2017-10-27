#include "RenderPage.h"

namespace WeexCore {

  void RenderPage::calculateLayout() {
    if (pRoot == nullptr)
      return;
    pRoot->getLayoutNode()->calculateLayout();
    traverseTree(pRoot);
  }

  void RenderPage::traverseTree(RenderObject *render) {
    sendReLayoutAction(render);
    for (int i = 0; i < render->getChildCount(); i++) {
      RenderObject *child = render->getChild(i);
      traverseTree(child);
    }
  }

  RenderPage::RenderPage(std::string pageID, std::string data) {
    mPageId = pageID;

    int alen = data.length();
    char *c_data = (char *) malloc(alen + 1);
    strcpy(c_data, data.c_str());
    c_data[alen] = 0;
    RenderObject *render = json2RenderObject(c_data, this);
    free(c_data);
    c_data = nullptr;
    mRenderObjectMap.insert(pair<std::string, RenderObject *>(render->getRef(), render));
    setRootRenderObject(render);

    calculateLayout();

    sendCreateBodyAction(render);
    sendUpdateStyleAction(render);
    sendUpdateAttrAction(render);
    sendAddEventAction(render);
  }

  RenderPage::~RenderPage() {

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
  }

  void RenderPage::addRenderObject(std::string parentRef, int insertPosition, RenderObject *child) {
    RenderObject *parent = getRenderObject(parentRef);
    if (parent == nullptr) {
      return;
    }

    if (child == nullptr) {
      return;
    }

    mRenderObjectMap.insert(pair<std::string, RenderObject *>(child->getRef(), child));

    // add child to dom Tree
    parent->getLayoutNode()->addChildAt(child->getLayoutNode(), parent->getChildCount());

    // add child to Render Tree
    child->setParentRender(parent);
    parent->addRenderObject(insertPosition, child);

    // layout by dom Tree
    calculateLayout();

    sendAddElementAction(child, parent, insertPosition);
    sendUpdateStyleAction(child);
    sendUpdateAttrAction(child);
    sendAddEventAction(child);
  }

  void RenderPage::removeRenderObject(std::string ref) {
    RenderObject *child = getRenderObject(ref);
    RenderObject *parent = child->getParentRender();

    parent->removeRenderObject(child);
//    YGNodeRemoveChild(parent->getLayoutNode(), child->getLayoutNode());
    mRenderObjectMap.erase(child->getRef());
    delete child;

    // TODO calculateLayout
  }

  void RenderPage::moveRenderObject(std::string ref, std::string parentRef, std::string index) {
    RenderObject *child = getRenderObject(ref);
    RenderObject *oldParent = child->getParentRender();
    RenderObject *newParent = getRenderObject(parentRef);

    oldParent->removeRenderObject(child);
    newParent->addRenderObject(stringToNum<int>(index), child);

    // TODO calculateLayout
  }

  void RenderPage::updateStyle(std::string ref, std::string key, std::string value) {
    RenderObject *render = getRenderObject(ref);
    render->updateStyle(key, value);

    // TODO calculateLayout
  }

  void RenderPage::updateAttr(std::string ref, std::string key, std::string value) {
    RenderObject *render = getRenderObject(ref);
    render->updateAttr(key, value);

    // TODO calculateLayout
  }

  void RenderPage::addEvent(std::string ref, std::string event) {
    RenderObject *render = getRenderObject(ref);
    render->addEvent(event);

    // TODO calculateLayout
  }

  void RenderPage::removeEvent(std::string ref, std::string event) {
    RenderObject *render = getRenderObject(ref);
    render->removeEvent(event);

    // TODO calculateLayout
  }

  void RenderPage::addRenderAction(RenderAction *action) {
    action->ExecuteAction();
  }

  RenderObject *RenderPage::getRenderObject(std::string ref) {
    return mRenderObjectMap.find(ref)->second;
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
