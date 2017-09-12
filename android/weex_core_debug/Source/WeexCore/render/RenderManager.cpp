#include "RenderManager.h"

namespace WeexCore {

  // save all pages info with RenderPage;
  static std::map<std::string, RenderPage *> *mPages = nullptr;

  RenderManager *RenderManager::m_pInstance = nullptr;

  RenderManager::RenderManager() {
  }

  void RenderManager::createPage(std::string pageId, std::string data) {
    RenderPage *page = new RenderPage(pageId, data);
    if (page != nullptr) {
      if (mPages == nullptr) {
        mPages = new std::map<std::string, RenderPage *>();
      }
      (*mPages)[pageId] = page;
    }
  }

  void RenderManager::addRenderObject(std::string pageId, std::string parentRef, int index,
                                      std::string data) {
    RenderPage *page = getPage(pageId);
    if (page != nullptr) {
      page->addRenderObject(parentRef, index, data);
    }
  }

  void RenderManager::removeRenderObject(std::string pageId, std::string ref) {
    RenderPage *page = this->getPage(pageId);
    if (page != nullptr) {
      page->removeRenderObject(ref);
    }
  }

  void RenderManager::moveRenderObject(std::string pageId, std::string ref, std::string parentRef,
                                       std::string index) {
    RenderPage *page = this->getPage(pageId);
    if (page != nullptr) {
      page->moveRenderObject(ref, parentRef, index);
    }
  }

  void RenderManager::updateAttr(std::string pageId, std::string ref, std::string key,
                                 std::string value) {
    RenderPage *page = this->getPage(pageId);
    if (page != nullptr) {
      page->updateAttr(ref, key, value);
    }
  }

  void RenderManager::updateStyle(std::string pageId, std::string ref, std::string key,
                                  std::string value) {
    RenderPage *page = this->getPage(pageId);
    if (page != nullptr) {
      page->updateStyle(ref, key, value);
    }
  }

  void RenderManager::addEvent(std::string pageId, std::string ref, std::string event) {
    RenderPage *page = this->getPage(pageId);
    if (page != nullptr) {
      page->addEvent(ref, event);
    }
  }

  void RenderManager::removeEvent(std::string pageId, std::string ref, std::string event) {
    RenderPage *page = this->getPage(pageId);
    if (page != nullptr) {
      page->removeEvent(ref, event);
    }
  }

  RenderPage *RenderManager::getPage(std::string id) {
    if (mPages == nullptr) {
      return nullptr;
    }

    mPages->find(id);
    RenderPage *page = mPages->find(id)->second;
    return page;
  }

  void RenderManager::printRenderAndLayoutTree(std::string pageId) {
    RenderPage *page = this->getPage(pageId);
    if (page == nullptr)
      return;
    page->getRootRenderObject()->printRenderMsg();
    page->getRootRenderObject()->printYGNodeMsg();
  }

} //namespace WeexCore
