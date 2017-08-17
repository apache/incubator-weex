#include "RenderManager.h"

namespace WeexCore {

RenderManager *RenderManager::m_pInstance = nullptr;

RenderManager::RenderManager() {
}

void RenderManager::createPage(String pageId, String data) {
  RenderPage *page = new RenderPage(pageId, data);
  if (page != NULL) {
    if (!mPages) {
      mPages = new HashMap<String, RenderPage *>();
    }
    mPages->add(pageId, page);
  }
}

void RenderManager::addRenderObject(String pageId, String parentRef, int index, String data) {
  LOGD("[RenderManager::addRenderObject] call addRenderObject, pageID: %d", pageId.toInt());
  RenderPage *page = this->getPage(pageId);
  if (page != NULL) {
    page->addRenderObject(parentRef, index, data);
  }
}

void RenderManager::removeRenderObject(String pageId, String ref) {
  RenderPage *page = this->getPage(pageId);
  if (page != NULL) {
    page->removeRenderObject(ref);
  }
}

void RenderManager::moveRenderObject(String pageId, String ref, String parentRef, String index) {
  RenderPage *page = this->getPage(pageId);
  if (page != NULL) {
    page->moveRenderObject(ref, parentRef, index);
  }
}

void RenderManager::updateAttr(String pageId, String ref, String key, String value) {
  RenderPage *page = this->getPage(pageId);
  if (page != NULL) {
    page->updateAttr(ref, key, value);
  }
}

void RenderManager::updateStyle(String pageId, String ref, String key, String value) {
  RenderPage *page = this->getPage(pageId);
  if (page != NULL) {
    page->updateStyle(ref, key, value);
  }
}

void RenderManager::addEvent(String pageId, String ref, String event) {
  RenderPage *page = this->getPage(pageId);
  if (page != NULL) {
    page->addEvent(ref, event);
  }
}

void RenderManager::removeEvent(String pageId, String ref, String event) {
  RenderPage *page = this->getPage(pageId);
  if (page != NULL) {
    page->removeEvent(ref, event);
  }
}

RenderPage *RenderManager::getPage(String id) {
  if (mPages == NULL) {
    return NULL;
  }
  return mPages->get(id);
}

void RenderManager::printRenderAndLayoutTree(String pageId) {
  LOGD("[RenderManager::Render tree] pageId: %s", pageId.utf8().data());
  RenderPage *page = this->getPage(pageId);
  page->getRootRenderObject()->printRenderMsg();
  page->getRootRenderObject()->printYGNodeMsg();
}

} //namespace WeexCore
