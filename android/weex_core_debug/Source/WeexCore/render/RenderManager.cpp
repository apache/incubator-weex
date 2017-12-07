#include <rapidjson/weexjsontools.h>
#include <WeexCore/render/RenderManager.h>
#include <WeexCore/render/RenderPage.h>
#include <WeexCore/render/RenderObject.h>

namespace WeexCore {

  // save all pages info with RenderPage;
  RenderManager *RenderManager::m_pInstance = nullptr;

  void RenderManager::createPage(std::string pageId, std::string data) {
    RenderPage *page = new RenderPage(pageId);
    mPages.insert(std::pair<std::string, RenderPage *>(pageId, page));
    page->createRootRender(data);
  }

  void RenderManager::addRenderObject(std::string pageId, std::string parentRef, int index,
                                      std::string data) {
    RenderPage *page = getPage(pageId);
    if (page != nullptr) {
      char *c_data = (char *) data.data();
      RenderObject *child = json2RenderObject(c_data, page);
      page->addRenderObject(parentRef, index, child);
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

  void RenderManager::updateStyle(std::string pageId, std::string ref, std::string data) {
    RenderPage *page = this->getPage(pageId);
    if (page != nullptr) {
      char *c_data = (char *) data.data();
      std::vector<std::pair<std::string, std::string>> styles;
      json2Pair(c_data, styles);
      for (int i = 0; i < styles.size(); ++i) {
        page->updateStyle(ref, styles[i].first, styles[i].second);
      }
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
    RenderPage *page = mPages.find(id)->second;
    return page;
  }

  void RenderManager::createFinish(std::string pageId) {
    RenderPage *page = getPage(pageId);
    if (page != nullptr)
      page->createFinish();
  }

  void RenderManager::printRenderAndLayoutTree(std::string pageId) {
    RenderPage *page = this->getPage(pageId);
    if (page == nullptr)
      return;
    page->getRootRenderObject()->printRenderMsg();
    page->getRootRenderObject()->printYGNodeMsg();
  }

} //namespace WeexCore
