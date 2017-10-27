#include "RenderPage.h"

namespace WeexCore {

  inline void getLayoutInfo(RenderAction *action, WXCoreLayoutNode *node) {
    action->mTop = node->getLayoutPositionTop();
    action->mBottom = node->getLayoutPositionBottom();
    action->mRight = node->getLayoutPositionRight();
    action->mLeft = node->getLayoutPositionLeft();
    action->mHeight = node->getLayoutHeight();
    action->mWidth = node->getLayoutWidth();
  }

  void RenderPage::calculateLayout() {
    if (pRoot == nullptr)
      return;
    pRoot->getLayoutNode()->calculateLayout();
    traverseTree(pRoot);
  }

  void RenderPage::traverseTree(RenderObject *render) {
    RenderAction *action = new RelayoutRenderAction();
    action->mPageId = getPageId();
    action->mComponentType = render->getType();
    action->mRef = render->getRef();
    getLayoutInfo(action, render->getLayoutNode());
    addRenderAction(action);

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

    // layout by dom Tree
    calculateLayout();

    /**
     * Generate RenderAction: ACTION_CREATE_BODY
     */
    RenderAction *createBodyAction = new CreateBodyAction();
    createBodyAction->mPageId = mPageId;
    createBodyAction->mComponentType = render->getType();
    createBodyAction->mRef = render->getRef();
    getLayoutInfo(createBodyAction, render->getLayoutNode());
    addRenderAction(createBodyAction);

    /**
     * Generate RenderAction: ACTION_UPDATE_STYLE
     */
    STYLE_IT style_it_star = render->getStyleItBegin();
    STYLE_IT style_it_end = render->getStyleItEnd();

    for (; style_it_star != style_it_end; ++style_it_star) {
      RenderAction *updateStyleAction = new UpdateStyleAction();
      updateStyleAction->mPageId = mPageId;
      updateStyleAction->mRef = render->getRef();
      updateStyleAction->mKey = style_it_star->first;
      updateStyleAction->mValue = style_it_star->second;
      addRenderAction(updateStyleAction);
    }

    /**
     * Generate RenderAction: ACTION_UPDATE_ATTR
     */
    ATTR_IT attr_it_star = render->getAttrItBegin();
    ATTR_IT attr_it_end = render->getAttrItEnd();

    for (; attr_it_star != attr_it_end; ++attr_it_star) {
      RenderAction *updateAttrAction = new UpdateStyleAction();
      updateAttrAction->mPageId = mPageId;
      updateAttrAction->mRef = render->getRef();
      updateAttrAction->mKey = attr_it_star->first;
      updateAttrAction->mValue = attr_it_star->second;
      addRenderAction(updateAttrAction);
    }

    /**
     * Generate RenderAction: ACTION_ADD_EVENT
     */
    EVENT_IT event_it_start = render->getEventItBegin();
    EVENT_IT event_it_end = render->getEventItEnd();

    for (; event_it_start != event_it_end; ++event_it_start) {
      RenderAction *addEventAction = new AddElementAction();
      addEventAction->mPageId = mPageId;
      addEventAction->mRef = render->getRef();
      addEventAction->mValue = *event_it_start;
      addRenderAction(addEventAction);
    }
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

    /**
     * Generate RenderAction: ACTION_ADD_ELEMENT
     */
    RenderAction *addElementAction = new AddElementAction();
    addElementAction->mPageId = mPageId;
    addElementAction->mComponentType = child->getType();
    addElementAction->mRef = child->getRef();
    addElementAction->mParentRef = parent->getRef();
    addElementAction->mIndex = insertPosition;
    getLayoutInfo(addElementAction, child->getLayoutNode());
    addRenderAction(addElementAction);

    /**
     * Generate RenderAction: ACTION_UPDATE_STYLE
     */
    STYLE_IT style_it_star = child->getStyleItBegin();
    STYLE_IT style_it_end = child->getStyleItEnd();

    for (; style_it_star != style_it_end; ++style_it_star) {
      RenderAction *updateStyleAction = new UpdateStyleAction();
      updateStyleAction->mPageId = mPageId;
      updateStyleAction->mRef = child->getRef();
      updateStyleAction->mKey = style_it_star->first;
      updateStyleAction->mValue = style_it_star->second;
      addRenderAction(updateStyleAction);
    }

    /**
     * Generate RenderAction: ACTION_UPDATE_ATTR
     */
    ATTR_IT attr_it_star = child->getAttrItBegin();
    ATTR_IT attr_it_end = child->getAttrItEnd();

    for (; attr_it_star != attr_it_end; ++attr_it_star) {
      RenderAction *updateAttrAction = new UpdateStyleAction();
      updateAttrAction->mPageId = mPageId;
      updateAttrAction->mRef = child->getRef();
      updateAttrAction->mKey = attr_it_star->first;
      updateAttrAction->mValue = attr_it_star->second;
      addRenderAction(updateAttrAction);
    }

    /**
     * Generate RenderAction: ACTION_ADD_EVENT
     */
    EVENT_IT event_it_start = child->getEventItBegin();
    EVENT_IT event_it_end = child->getEventItEnd();

    for (; event_it_start != event_it_end; ++event_it_start) {
//      RenderAction *addEventAction = new RenderAction();
//      addEventAction->mPageId = mPageId;
//      addEventAction->mRef = child->getRef();
//      addEventAction->mValue = *event_it_start;
//      addRenderAction(addEventAction);

      // TODO AddEventAction
    }

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
