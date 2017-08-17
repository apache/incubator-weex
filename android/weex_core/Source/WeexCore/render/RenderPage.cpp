#include "RenderPage.h"

namespace WeexCore {
RenderPage::RenderPage(String pageID, String data) {
  mPageId = pageID.toInt();
  mRenderObjectMap = new HashMap<String, RenderObject *>();

  RenderObject *render = json2RenderObject(wtfString2cstr(data), this);
  mRenderObjectMap->set(render->getRef(), render);
  setRootRenderObject(render);
  // layout by YogaNode Tree
  YGNodeCalculateLayout(pRoot->getYGNode(), YGUndefined, YGUndefined, YGDirectionLTR);
}

RenderPage::~RenderPage() {
  delete pRoot;

  for (Vector<RenderAction *>::iterator it = mRenderActions.begin(); it != mRenderActions.end(); it++) {
    if (NULL != *it) {
      delete *it;
      *it = NULL;
    }
  }
  mRenderActions.clear();

  typedef typename HashMap<String, RenderObject *>::const_iterator::Keys iterator_keys;

  iterator_keys attr_it = mRenderObjectMap->begin().keys();
  iterator_keys attr_end = mRenderObjectMap->end().keys();
  for (; attr_it != attr_end; ++attr_it) {
    delete mRenderObjectMap->get(*attr_it);
  }

  mRenderObjectMap->clear();
  delete mRenderObjectMap;
}

void RenderPage::addRenderObject(String parentRef, int insertPosiotn, String data) {
  RenderObject *parent = getRenderObject(parentRef);
  if (parent == NULL) {
    return;
  }

  RenderObject *child = json2RenderObject(wtfString2cstr(data), this);
  if (child == NULL) {
    return;
  }

  mRenderObjectMap->set(child->getRef(), child);

  // add child to YogaNode Tree
  YGNodeInsertChild(parent->getYGNode(), child->getYGNode(), parent->getChildCount());

  // add child to Render Tree
  child->setParentRender(parent);
  LOGD("[RenderPage::addRenderObject] parentRef : %s, childRef: %s", parent->getRef().utf8().data(),
       child->getRef().utf8().data());
  parent->addRenderObject(insertPosiotn, child);

  // layout by YogaNode Tree
  YGNodeCalculateLayout(pRoot->getYGNode(), YGUndefined, YGUndefined, YGDirectionLTR);
}

void RenderPage::removeRenderObject(String ref) {
  RenderObject *child = getRenderObject(ref);
  RenderObject *parent = child->getParentRender();

  parent->removeRenderObject(child);
  YGNodeRemoveChild(parent->getYGNode(), child->getYGNode());
  mRenderObjectMap->remove(child->getRef());
  delete child;

  // layout by YogaNode Tree
  YGNodeCalculateLayout(pRoot->getYGNode(), YGUndefined, YGUndefined, YGDirectionLTR);
}

void RenderPage::moveRenderObject(String ref, String parentRef, String index) {
  RenderObject *child = getRenderObject(ref);
  RenderObject *oldParent = child->getParentRender();
  RenderObject *newParent = getRenderObject(parentRef);

  oldParent->removeRenderObject(child);
  newParent->addRenderObject(index.toInt(), child);

  YGNodeCalculateLayout(pRoot->getYGNode(), YGUndefined, YGUndefined, YGDirectionLTR);
}

void RenderPage::updateStyle(String ref, String key, String value) {
  RenderObject *render = getRenderObject(ref);
  render->updateStyle(key, value);

  // layout by YogaNode Tree
  YGNodeCalculateLayout(pRoot->getYGNode(), YGUndefined, YGUndefined, YGDirectionLTR);
}

void RenderPage::updateAttr(String ref, String key, String value) {
  RenderObject *render = getRenderObject(ref);
  render->updateAttr(key, value);
}

void RenderPage::addEvent(String ref, String event) {
  RenderObject *render = getRenderObject(ref);
  render->addEvent(event);
}

void RenderPage::removeEvent(String ref, String event) {
  RenderObject *render = getRenderObject(ref);
  render->removeEvent(event);
}

void RenderPage::addRenderAction(RenderAction *action) {

}

RenderObject *RenderPage::getRenderObject(String ref) {
  return mRenderObjectMap->get(ref);
}

void RenderPage::setRootRenderObject(RenderObject *root) {
  if (root != NULL) {
    pRoot = root;
  }
}

RenderObject *RenderPage::getRootRenderObject() {
  return pRoot;
}

} //namespace WeexCore
