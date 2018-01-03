#include <WeexCore/parser/DomParser.h>
#include <WeexCore/render/manager/RenderManager.h>
#include <WeexCore/render/page/RenderPage.h>
#include <WeexCore/render/node/RenderObject.h>
#include <base/TimeUtils.h>

namespace WeexCore {

  RenderManager *RenderManager::m_pInstance = nullptr;

  bool RenderManager::createPage(const std::string &pageId, const std::string &data) {
    RenderPage *page = new RenderPage(pageId);
    mPages.insert(std::pair<std::string, RenderPage *>(pageId, page));

    long long startTime = getCurrentTime();
    char *c_data = (char *) data.data();
    RenderObject *root = json2RenderObject(c_data);
    page->parseJsonTime(getCurrentTime() - startTime);
    page->buildRenderObjectTime(getCurrentTime() - startTime);
    return page->createRootRender(root);
  }

  bool RenderManager::addRenderObject(const std::string &pageId, const std::string &parentRef,
                                      const int &index,
                                      const std::string &data) {
    RenderPage *page = getPage(pageId);
    if (page == nullptr)
      return false;

    long long startTime = getCurrentTime();
    char *c_data = (char *) data.data();
    RenderObject *child = json2RenderObject(c_data);
    page->parseJsonTime(getCurrentTime() - startTime);
    page->buildRenderObjectTime(getCurrentTime() - startTime);
    if (child == nullptr)
      return false;

    return page->addRenderObject(parentRef, index, child);
  }

  bool RenderManager::removeRenderObject(const std::string &pageId, const std::string &ref) {
    RenderPage *page = this->getPage(pageId);
    if (page == nullptr)
      return false;

    return page->removeRenderObject(ref);
  }

  bool RenderManager::moveRenderObject(const std::string &pageId, const std::string &ref,
                                       const std::string &parentRef,
                                       int index) {
    RenderPage *page = this->getPage(pageId);
    if (page == nullptr)
      return false;

    return page->moveRenderObject(ref, parentRef, index);
  }

  bool RenderManager::updateAttr(const std::string &pageId, const std::string &ref,
                                 const std::string &data) {
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

  bool RenderManager::updateStyle(const std::string &pageId, const std::string &ref,
                                  const std::string &data) {
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

  bool RenderManager::addEvent(const std::string &pageId, const std::string &ref,
                               const std::string &event) {
    RenderPage *page = this->getPage(pageId);
    if (page == nullptr)
      return false;

    return page->addEvent(ref, event);
  }

  bool RenderManager::removeEvent(const std::string &pageId, const std::string &ref,
                                  const std::string &event) {
    RenderPage *page = this->getPage(pageId);
    if (page == nullptr)
      return false;

    return page->removeEvent(ref, event);
  }

  bool RenderManager::createFinish(const std::string &pageId) {
    RenderPage *page = getPage(pageId);
    if (page == nullptr)
      return false;

    return page->createFinish();
  }

  RenderPage *RenderManager::getPage(const std::string &id) {
    RenderPage *page = mPages.find(id)->second;
    return page;
  }

  void RenderManager::printRenderAndLayoutTree(const std::string &pageId) {
    RenderPage *page = this->getPage(pageId);
    if (page == nullptr)
      return;
    page->getRootRenderObject()->printRenderMsg();
    page->getRootRenderObject()->printYGNodeMsg();
  }

  void RenderManager::batch(const std::string &pageId) {
    RenderPage *page = this->getPage(pageId);
    if (page == nullptr)
      return;

    page->batch();

  }

} //namespace WeexCore
