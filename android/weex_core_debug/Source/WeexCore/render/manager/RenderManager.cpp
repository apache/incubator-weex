#include <WeexCore/render/node/factory/parser/DomParser.h>
#include <WeexCore/render/manager/RenderManager.h>
#include <WeexCore/render/page/RenderPage.h>
#include <WeexCore/render/node/RenderObject.h>
#include <base/TimeUtils.h>

namespace WeexCore {

  RenderManager *RenderManager::m_pInstance = nullptr;

  bool RenderManager::CreatePage(std::string pageId, const std::string &data) {
    RenderPage *page = new RenderPage(pageId);
    mPages.insert(std::pair<std::string, RenderPage *>(pageId, page));

    long long startTime = getCurrentTime();
    char *c_data = (char *) data.data();
    RenderObject *root = Json2RenderObject(c_data, pageId);
    page->ParseJsonTime(getCurrentTime() - startTime);
    page->BuildRenderTreeTime(getCurrentTime() - startTime);
    return page->CreateRootRender(root);
  }

  bool RenderManager::AddRenderObject(const std::string &pageId, std::string parentRef,
                                      int index,
                                      const std::string &data) {

    RenderPage *page = GetPage(pageId);
    if (page == nullptr)
      return false;

    long long startTime = getCurrentTime();
    char *c_data = (char *) data.data();
    RenderObject *child = Json2RenderObject(c_data, pageId);
    page->ParseJsonTime(getCurrentTime() - startTime);
    page->BuildRenderTreeTime(getCurrentTime() - startTime);
    if (child == nullptr)
      return false;

    return page->AddRenderObject(parentRef, index, child);
  }

  bool RenderManager::RemoveRenderObject(const std::string &pageId, const std::string &ref) {
    RenderPage *page = this->GetPage(pageId);
    if (page == nullptr)
      return false;

    return page->RemoveRenderObject(ref);
  }

  bool RenderManager::MoveRenderObject(const std::string &pageId, const std::string &ref,
                                       const std::string &parentRef,
                                       int index) {
    RenderPage *page = this->GetPage(pageId);
    if (page == nullptr)
      return false;

    return page->MoveRenderObject(ref, parentRef, index);
  }

  bool RenderManager::UpdateAttr(const std::string &pageId, const std::string &ref,
                                 const std::string &data) {
    RenderPage *page = this->GetPage(pageId);
    if (page == nullptr)
      return false;

    long long startTime = getCurrentTime();
    char *c_data = (char *) data.data();
    std::vector<std::pair<std::string, std::string> *> *attrs = Json2Pairs(c_data);
    page->ParseJsonTime(getCurrentTime() - startTime);
    page->BuildRenderTreeTime(getCurrentTime() - startTime);
    return page->UpdateAttr(ref, attrs);
  }

  bool RenderManager::UpdateStyle(const std::string &pageId, const std::string &ref,
                                  const std::string &data) {
    RenderPage *page = this->GetPage(pageId);
    if (page == nullptr)
      return false;

    long long startTime = getCurrentTime();
    char *c_data = (char *) data.data();
    std::vector<std::pair<std::string, std::string> *> *styles = Json2Pairs(c_data);
    page->ParseJsonTime(getCurrentTime() - startTime);
    page->BuildRenderTreeTime(getCurrentTime() - startTime);
    return page->UpdateStyle(ref, styles);
  }

  bool RenderManager::AddEvent(const std::string &pageId, const std::string &ref,
                               std::string event) {
    RenderPage *page = this->GetPage(pageId);
    if (page == nullptr)
      return false;

    return page->AddEvent(ref, event);
  }

  bool RenderManager::RemoveEvent(const std::string &pageId, const std::string &ref,
                                  const std::string &event) {
    RenderPage *page = this->GetPage(pageId);
    if (page == nullptr)
      return false;

    return page->RemoveEvent(ref, event);
  }

  bool RenderManager::CreateFinish(const std::string &pageId) {
    RenderPage *page = GetPage(pageId);
    if (page == nullptr)
      return false;

    return page->CreateFinish();
  }

  RenderPage *RenderManager::GetPage(const std::string &id) {
    RenderPage *page = mPages.find(id)->second;
    return page;
  }

  bool RenderManager::ClosePage(std::string pageId) {
    RenderPage *page = GetPage(pageId);
    if (page == nullptr)
      return false;

    mPages.erase(pageId);
    delete page;
  }

  void RenderManager::PrintRenderAndLayoutTree(const std::string &pageId) {
    RenderPage *page = this->GetPage(pageId);
    if (page == nullptr)
      return;
    page->GetRootRenderObject()->PrintRenderMsg();
    page->GetRootRenderObject()->PrintLayoutMsg();
  }

  void RenderManager::Batch(const std::string &pageId) {
    RenderPage *page = this->GetPage(pageId);
    if (page == nullptr)
      return;

    page->Batch();

  }

} //namespace WeexCore
