#include <rapidjson/weexjsontools.h>
#include <WeexCore/render/manager/RenderManager.h>
#include <WeexCore/render/page/RenderPage.h>
#include <WeexCore/render/node/RenderObject.h>
#include <base/TimeUtils.h>

namespace WeexCore {

  RenderManager *RenderManager::m_pInstance = nullptr;

  bool RenderManager::createPage(std::string pageId, std::string data) {
    RenderPage *page = new RenderPage(pageId);
    mPages.insert(std::pair<std::string, RenderPage *>(pageId, page));

    long long startTime = getCurrentTime();
    char *c_data = (char *) data.data();
    RenderObject *root = json2RenderObject(c_data, page);
    page->parseJsonTime(getCurrentTime() - startTime);
    page->buildRenderObjectTime(getCurrentTime() - startTime);
    return page->createRootRender(root);
  }

  bool RenderManager::addRenderObject(std::string pageId, std::string parentRef, int index,
                                      std::string data) {
    RenderPage *page = getPage(pageId);
    if (page == nullptr)
      return false;

    long long startTime = getCurrentTime();
    char *c_data = (char *) data.data();
    RenderObject *child = json2RenderObject(c_data, page);
    page->parseJsonTime(getCurrentTime() - startTime);
    page->buildRenderObjectTime(getCurrentTime() - startTime);
    if (child == nullptr)
      return false;

    return page->addRenderObject(parentRef, index, child);
  }

  bool RenderManager::removeRenderObject(std::string pageId, std::string ref) {
    RenderPage *page = this->getPage(pageId);
    if (page == nullptr)
      return false;

    return page->removeRenderObject(ref);
  }

  bool RenderManager::moveRenderObject(std::string pageId, std::string ref, std::string parentRef, int index) {
    RenderPage *page = this->getPage(pageId);
    if (page == nullptr)
      return false;

    return page->moveRenderObject(ref, parentRef, index);
  }

  bool RenderManager::updateAttr(std::string pageId, std::string ref, std::string data) {
    RenderPage *page = this->getPage(pageId);
    if (page == nullptr)
      return false;

    long long startTime = getCurrentTime();
    char *c_data = (char *) data.data();
    std::vector<std::pair<std::string, std::string> *> *attrs = json2Pairs(c_data);
    page->parseJsonTime(getCurrentTime() - startTime);
    page->buildRenderObjectTime(getCurrentTime() - startTime);
    return page->updateAttr(ref, attrs);
  }

  bool RenderManager::updateStyle(std::string pageId, std::string ref, std::string data) {
    RenderPage *page = this->getPage(pageId);
    if (page == nullptr)
      return false;

    long long startTime = getCurrentTime();
    char *c_data = (char *) data.data();
    std::vector<std::pair<std::string, std::string> *> *styles = json2Pairs(c_data);
    page->parseJsonTime(getCurrentTime() - startTime);
    page->buildRenderObjectTime(getCurrentTime() - startTime);
    return page->updateStyle(ref, styles);
  }

  bool RenderManager::addEvent(std::string pageId, std::string ref, std::string event) {
    RenderPage *page = this->getPage(pageId);
    if (page == nullptr)
      return false;

    return page->addEvent(ref, event);
  }

  bool RenderManager::removeEvent(std::string pageId, std::string ref, std::string event) {
    RenderPage *page = this->getPage(pageId);
    if (page == nullptr)
      return false;

    return page->removeEvent(ref, event);
  }

  bool RenderManager::createFinish(std::string pageId) {
    RenderPage *page = getPage(pageId);
    if (page == nullptr)
      return false;

    return page->createFinish();
  }

  RenderPage *RenderManager::getPage(std::string id) {
    RenderPage *page = mPages.find(id)->second;
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
